#!/usr/bin/env bash
set -euo pipefail

IMG=ext2.img
MNT=ext2_mnt
BLOCK_SIZE=2048
IMG_SIZE=512M

BINDIR=.

PASS=0
FAIL=0

LOOPDEV=""

ok()   { echo "[PASS] $*"; PASS=$((PASS+1)); }
fail() { echo "[FAIL] $*"; FAIL=$((FAIL+1)); }

cleanup() {
    echo "=== [cleanup] ==="

    set +e

    if mountpoint -q "$MNT" 2>/dev/null; then
        sudo umount "$MNT"
    fi

    if [ -n "${LOOPDEV:-}" ]; then
        sudo losetup -d "$LOOPDEV" 2>/dev/null
    fi

    sudo losetup -a | grep "$IMG" | cut -d: -f1 | while read -r l; do
        sudo losetup -d "$l" 2>/dev/null || true
    done

    rmdir "$MNT" 2>/dev/null || true

    echo "cleanup done"
}

trap cleanup EXIT INT TERM

require_root_for() {
    if [ "$(id -u)" -eq 0 ]; then
        "$@"
    else
        sudo "$@"
    fi
}

echo "=== [1] Create image ==="

rm -f "$IMG"
truncate --size "$IMG_SIZE" "$IMG"
mkfs.ext2 -b "$BLOCK_SIZE" -N 4096 "$IMG"

echo "=== [2] Setup loop device ==="

mkdir -p "$MNT"
LOOPDEV=$(sudo losetup --find --show "$IMG")
sudo chown "$(id -u):$(id -g)" "$LOOPDEV"

echo "=== [3] Mount filesystem ==="

require_root_for mount -t ext2 "$LOOPDEV" "$MNT"

sudo chown "$(id -u):$(id -g)" "$MNT"
echo "=== [4] Create test data ==="

mkdir -p "$MNT/dira/subdirb"
mkdir -p "$MNT/dirc"

echo 'Hello ext2' > "$MNT/dira/small.txt"

dd if=/dev/urandom \
   of="$MNT/dira/subdirb/big.bin" \
   bs=4096 \
   count=256 \
   status=none

truncate --size 5G "$MNT/dirc/sparse_huge.bin"

sync

echo "=== [5] Collect metadata ==="

get_inode() {
    stat --format='%i' "$1"
}

INODE_ROOT=$(get_inode "$MNT")
INODE_DIRA=$(get_inode "$MNT/dira")
INODE_SUB=$(get_inode "$MNT/dira/subdirb")

INODE_SMALL=$(get_inode "$MNT/dira/small.txt")
INODE_BIG=$(get_inode "$MNT/dira/subdirb/big.bin")
INODE_SPARSE=$(get_inode "$MNT/dirc/sparse_huge.bin")

SHA_SMALL=$(sha512sum "$MNT/dira/small.txt" | awk '{print $1}')
SHA_BIG=$(sha512sum "$MNT/dira/subdirb/big.bin" | awk '{print $1}')

SHA_SPARSE=$(dd if="$MNT/dirc/sparse_huge.bin" \
                bs=4096 \
                count=16 \
                status=none | sha512sum | awk '{print $1}')

echo "=== [6] Unmount ==="

require_root_for umount "$MNT"

echo "=== [7] sb_info ==="

SB=$("$BINDIR/sb_info" "$IMG")

echo "$SB" | grep -q "magic: 0xef53" \
    && ok "sb_info magic" \
    || fail "sb_info magic"

echo "$SB" | grep -q "block size: $BLOCK_SIZE" \
    && ok "sb_info block size" \
    || fail "sb_info block size"

echo "=== [8] inode_info ==="

for ino in \
    "$INODE_ROOT" \
    "$INODE_DIRA" \
    "$INODE_SUB" \
    "$INODE_SMALL" \
    "$INODE_BIG" \
    "$INODE_SPARSE"
do
    OUT=$("$BINDIR/inode_info" "$IMG" "$ino")

    echo "$OUT" | grep -q "inode $ino" \
        && ok "inode_info $ino" \
        || fail "inode_info $ino"
done

SPARSE_INFO=$("$BINDIR/inode_info" "$IMG" "$INODE_SPARSE")
SPARSE_SIZE=$(echo "$SPARSE_INFO" | grep '^size:' | awk '{print $2}')

[ "$SPARSE_SIZE" = "5368709120" ] && ok "sparse size" || fail "sparse size"

echo "=== [9] inode_cat ==="

GOT_SMALL=$("$BINDIR/inode_cat" "$IMG" "$INODE_SMALL" | sha512sum | awk '{print $1}')
[ "$GOT_SMALL" = "$SHA_SMALL" ] && ok "small file" || fail "small file"

GOT_BIG=$("$BINDIR/inode_cat" "$IMG" "$INODE_BIG" | sha512sum | awk '{print $1}')
[ "$GOT_BIG" = "$SHA_BIG" ] && ok "big file" || fail "big file"

TMP_SPARSE=$(mktemp)

"$BINDIR/inode_cat" "$IMG" "$INODE_SPARSE" > "$TMP_SPARSE"

GOT_SPARSE=$(dd if="$TMP_SPARSE" bs=4096 count=16 2>/dev/null \
    | sha512sum | awk '{print $1}')

rm -f "$TMP_SPARSE"

[ "$GOT_SPARSE" = "$SHA_SPARSE" ] \
    && ok "sparse file" \
    || fail "sparse file"

echo "=== [10] inode_dir ==="

ROOT_DIR=$("$BINDIR/inode_dir" "$IMG" "$INODE_ROOT")

echo "$ROOT_DIR"

echo "$ROOT_DIR" | grep -q "dira" \
    && ok "root contains dira" \
    || fail "root contains dira"

echo "$ROOT_DIR" | grep -q "dirc" \
    && ok "root contains dirc" \
    || fail "root contains dirc"

echo "=== [11] loop device ==="

OUT_LOOP=$("$BINDIR/sb_info" "$LOOPDEV")

echo "$OUT_LOOP" | grep -q "magic: 0xef53" \
    && ok "loop sb_info" \
    || fail "loop sb_info"

GOT_SMALL_LOOP=$("$BINDIR/inode_cat" "$LOOPDEV" "$INODE_SMALL" \
    | sha512sum | awk '{print $1}')

[ "$GOT_SMALL_LOOP" = "$SHA_SMALL" ] \
    && ok "loop inode_cat" \
    || fail "loop inode_cat"

echo "=== [12] valgrind ==="

valgrind --quiet --error-exitcode=1 \
    "$BINDIR/inode_info" "$IMG" "$INODE_BIG" >/dev/null \
    && ok "valgrind inode_info" \
    || fail "valgrind inode_info"

valgrind --quiet --error-exitcode=1 \
    "$BINDIR/inode_cat" "$IMG" "$INODE_BIG" >/dev/null \
    && ok "valgrind inode_cat" \
    || fail "valgrind inode_cat"

valgrind --quiet --error-exitcode=1 \
    "$BINDIR/inode_dir" "$IMG" "$INODE_ROOT" >/dev/null \
    && ok "valgrind inode_dir" \
    || fail "valgrind inode_dir"

echo "==============================="
echo "PASS=$PASS FAIL=$FAIL"
echo "==============================="

[ "$FAIL" -eq 0 ]
