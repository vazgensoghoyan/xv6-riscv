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

# CLEANUP

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

    rmdir ext2_mnt
    rm ext2.img

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

# CREATE FS

echo "=== [1] Create image ==="

rm -f "$IMG"
truncate --size "$IMG_SIZE" "$IMG"
mkfs.ext2 -b "$BLOCK_SIZE" -N 4096 "$IMG"

echo "=== [2] mount ==="

mkdir -p "$MNT"
LOOPDEV=$(sudo losetup --find --show "$IMG")

require_root_for mount -t ext2 "$LOOPDEV" "$MNT"

# CREATE FILES

require_root_for mkdir -p "$MNT/dira/subdirb"
require_root_for bash -c "echo 'Hello' > $MNT/small.txt"
require_root_for bash -c "dd if=/dev/urandom bs=4096 count=64 2>/dev/null > $MNT/big.bin"
require_root_for bash -c "truncate --size 5G $MNT/sparse_huge.bin"

# INODES

get_inode() { stat --format='%i' "$MNT/$1"; }

INODE_ROOT=$(get_inode ".")
INODE_DIRA=$(get_inode "dira")
INODE_SUB=$(get_inode "dira/subdirb")
INODE_SMALL=$(get_inode "small.txt")
INODE_BIG=$(get_inode "big.bin")
INODE_SPARSE=$(get_inode "sparse_huge.bin")

SHA_SMALL=$(sha512sum "$MNT/small.txt" | awk '{print $1}')
SHA_BIG=$(sha512sum "$MNT/big.bin" | awk '{print $1}')

SHA_SPARSE=$(dd if="$MNT/sparse_huge.bin" bs=4096 count=16 2>/dev/null | sha512sum | awk '{print $1}')

# UMOUNT BEFORE TESTS

require_root_for umount "$MNT"

# TESTS

echo "=== [5] sb_info ==="
SB=$("$BINDIR/sb_info" "$IMG")

echo "$SB" | grep -q "magic: 0xef53" && ok "sb_info magic" || fail "sb_info magic"
echo "$SB" | grep -q "block size: $BLOCK_SIZE" && ok "sb_info block size" || fail "sb_info block size"

echo "=== [6] inode_info ==="
for ino in $INODE_ROOT $INODE_DIRA $INODE_SMALL $INODE_BIG $INODE_SPARSE; do
    OUT=$("$BINDIR/inode_info" "$IMG" "$ino")
    echo "$OUT" | grep -q "inode $ino" && ok "inode_info $ino" || fail "inode_info $ino"
done

SPARSE_INFO=$("$BINDIR/inode_info" "$IMG" "$INODE_SPARSE")
SPARSE_SIZE=$(echo "$SPARSE_INFO" | grep '^size:' | awk '{print $2}')

[ "$SPARSE_SIZE" = "5368709120" ] && ok "sparse size" || fail "sparse size"

echo "=== [7] inode_cat ==="

GOT_SMALL=$("$BINDIR/inode_cat" "$IMG" "$INODE_SMALL" | sha512sum | awk '{print $1}')
[ "$GOT_SMALL" = "$SHA_SMALL" ] && ok "small" || fail "small"

GOT_BIG=$("$BINDIR/inode_cat" "$IMG" "$INODE_BIG" | sha512sum | awk '{print $1}')
[ "$GOT_BIG" = "$SHA_BIG" ] && ok "big" || fail "big"

echo "=== [8] inode_dir ==="

ROOT_DIR=$("$BINDIR/inode_dir" "$IMG" "$INODE_ROOT")

echo "$ROOT_DIR"

echo "=== [9] loop test ==="

OUT_LOOP=$(require_root_for "$BINDIR/sb_info" "$LOOPDEV")
echo "$OUT_LOOP" | grep -q "magic: 0xef53" && ok "loop sb_info" || fail "loop sb_info"

GOT_SMALL_LOOP=$(require_root_for "$BINDIR/inode_cat" "$LOOPDEV" "$INODE_SMALL" | sha512sum | awk '{print $1}')
[ "$GOT_SMALL_LOOP" = "$SHA_SMALL" ] && ok "loop cat" || fail "loop cat"

# RESULT

echo "==============================="
echo "PASS=$PASS FAIL=$FAIL"
echo "==============================="

[ "$FAIL" -eq 0 ]
