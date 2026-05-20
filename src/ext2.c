#include "ext2.h"
#include "util.h"

int ext2_read_superblock(int fd, struct ext2_superblock *sb) {
    return read_bytes(fd, sb, sizeof(*sb), EXT2_SUPER_OFFSET);
}
