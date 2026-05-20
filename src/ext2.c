#include "ext2.h"
#include "util.h"
#include "endian.h"

#include <stdlib.h>
#include <string.h>

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

// INODE

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

// BLOCK

int ext2_read_block(int fd, const struct ext2_superblock *sb, uint32_t block_num, void *buf) {
    uint32_t block_size = ext2_block_size(sb);

    uint64_t offset = (uint64_t)block_num * block_size;

    return read_bytes(fd, buf, block_size, offset);
}

// INDIRECT HELPERS

static int process_indirect(
    int fd,
    const struct ext2_superblock *sb,
    uint32_t block_num,
    uint32_t logical_base,
    ext2_block_cb cb,
    void *ctx
) {
    uint32_t block_size = ext2_block_size(sb);

    uint32_t *buf = malloc(block_size);
    if (!buf)
        return -1;

    if (ext2_read_block(fd, sb, block_num, buf) < 0) {
        free(buf);
        return -1;
    }

    uint32_t count = block_size / 4;

    for (uint32_t i = 0; i < count; i++) {
        uint32_t phys = ext2_le32(buf[i]);

        if (phys == 0)
            continue;

        if (cb(logical_base + i, phys, ctx) < 0) {
            free(buf);
            return -1;
        }
    }

    free(buf);
    return 0;
}

// FOREACH BLOCK

int ext2_inode_foreach_block(
    int fd,
    const struct ext2_superblock *sb,
    const struct ext2_inode *inode,
    ext2_block_cb cb,
    void *ctx
) {
    for (uint32_t i = 0; i < 12; i++) {

        uint32_t block = ext2_le32(inode->i_block[i]);

        if (block == 0)
            continue;

        if (cb(i, block, ctx) < 0)
            return -1;
    }

    uint32_t single = ext2_le32(inode->i_block[12]);

    if (single) {
        if (process_indirect(fd, sb, single, 12, cb, ctx) < 0)
            return -1;
    }

    uint32_t double_ind = ext2_le32(inode->i_block[13]);

    if (double_ind) {

        uint32_t block_size = ext2_block_size(sb);

        uint32_t *buf = malloc(block_size);
        if (!buf)
            return -1;

        if (ext2_read_block(fd, sb, double_ind, buf) < 0) {
            free(buf);
            return -1;
        }

        uint32_t per_indirect = block_size / 4;

        for (uint32_t i = 0; i < per_indirect; i++) {
            uint32_t indirect = ext2_le32(buf[i]);

            if (indirect == 0)
                continue;

            uint32_t base = 12 + i * per_indirect;

            if (process_indirect(fd, sb, indirect, base, cb, ctx) < 0) {
                free(buf);
                return -1;
            }
        }

        free(buf);
    }

    return 0;
}
