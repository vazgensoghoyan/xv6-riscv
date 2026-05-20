#include "ext2.h"
#include "util.h"
#include "endian.h"

int ext2_read_superblock(int fd, struct ext2_superblock *sb) {
    return read_bytes(fd, sb, sizeof(*sb), EXT2_SUPER_OFFSET);
}

uint32_t ext2_block_size(const struct ext2_superblock *sb) {
    return 1024U << ext2_le32(sb->s_log_block_size);
}

int ext2_read_group_desc(
    int fd,
    const struct ext2_superblock *sb,
    uint32_t group,
    struct ext2_group_desc *gd
) {
    uint32_t block_size = ext2_block_size(sb);

    uint64_t bgdt_offset = (block_size == 1024) ? 2 * block_size : block_size;

    uint64_t offset = bgdt_offset + (uint64_t)group * sizeof(*gd);

    return read_bytes(fd, gd, sizeof(*gd), offset);
}

int ext2_read_inode(
    int fd,
    const struct ext2_superblock *sb,
    uint32_t inode_num,
    struct ext2_inode *inode
) {
    if (inode_num == 0)
        return -1;

    uint32_t inodes_per_group = ext2_le32(sb->s_inodes_per_group);
    uint32_t inode_size = ext2_le16(sb->s_inode_size);
    uint32_t block_size = ext2_block_size(sb);
    uint32_t group = (inode_num - 1) / inodes_per_group;
    uint32_t index = (inode_num - 1) % inodes_per_group;

    struct ext2_group_desc gd;
    if (ext2_read_group_desc(fd, sb, group, &gd) < 0)
        return -1;

    uint32_t inode_table = ext2_le32(gd.bg_inode_table);

    uint64_t inode_offset = (uint64_t)inode_table * block_size + (uint64_t)index * inode_size;

    return read_bytes(fd, inode, sizeof(*inode), inode_offset);
}
