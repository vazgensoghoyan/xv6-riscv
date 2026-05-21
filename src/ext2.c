#include "ext2.h"
#include "util.h"
#include "endian.h"

#include <stdlib.h>
#include <string.h>

int ext2_read_superblock(int fd, struct ext2_superblock *sb) {
    return read_bytes(fd, sb, sizeof(*sb), (off_t)EXT2_SUPER_OFFSET);
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

    return read_bytes(fd, gd, sizeof(*gd), (off_t)offset);
}

// INODE

int ext2_read_inode(
    int fd,
    const struct ext2_superblock *sb,
    uint32_t inode_num,
    struct ext2_inode *inode
) {
    uint32_t total_inodes = ext2_le32(sb->s_inodes_count);
    if (inode_num == 0 || inode_num > total_inodes)
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

    return read_bytes(fd, inode, sizeof(*inode), (off_t)inode_offset);
}

// BLOCK

int ext2_read_block(int fd, const struct ext2_superblock *sb, uint32_t block_num, void *buf) {
    uint32_t block_size = ext2_block_size(sb);
    uint64_t offset = (uint64_t)block_num * block_size;
    return read_bytes(fd, buf, block_size, (off_t)offset);
}

// INDIRECT HELPERS

static int walk_indirect(
    int fd,
    const struct ext2_superblock *sb,
    uint32_t block_num,
    int depth,
    uint32_t *logical_counter,
    ext2_block_cb cb,
    void *ctx
) {
    uint32_t block_size = ext2_block_size(sb);
    uint32_t per_block  = block_size / 4;

    uint32_t *buf = malloc(block_size);
    if (!buf)
        return -1;

    if (ext2_read_block(fd, sb, block_num, buf) < 0) {
        free(buf);
        return -1;
    }

    for (uint32_t i = 0; i < per_block; i++) {
        uint32_t phys = ext2_le32(buf[i]);
        if (depth == 1) {
            if (phys != 0) {
                if (cb(*logical_counter, phys, ctx) < 0) {
                    free(buf);
                    return -1;
                }
            }
            (*logical_counter)++;
        } else {
            if (phys != 0) {
                if (walk_indirect(fd, sb, phys, depth - 1,
                                  logical_counter, cb, ctx) < 0) {
                    free(buf);
                    return -1;
                }
            } else {
                uint32_t subtree = 1;
                for (int d = 1; d < depth; d++)
                    subtree *= per_block;
                *logical_counter += subtree;
            }
        }
    }

    free(buf);
    return 0;
}

int ext2_inode_foreach_block(
    int fd,
    const struct ext2_superblock *sb,
    const struct ext2_inode *inode,
    ext2_block_cb cb,
    void *ctx
) {
    uint32_t logical = 0;

    for (uint32_t i = 0; i < 12; i++) {
        uint32_t phys = ext2_le32(inode->i_block[i]);
        if (phys != 0) {
            if (cb(logical, phys, ctx) < 0)
                return -1;
        }
        logical++;
    }

    uint32_t s1 = ext2_le32(inode->i_block[12]);
    if (s1) {
        if (walk_indirect(fd, sb, s1, 1, &logical, cb, ctx) < 0)
            return -1;
    } else {
        uint32_t block_size = ext2_block_size(sb);
        logical += block_size / 4;
    }

    uint32_t s2 = ext2_le32(inode->i_block[13]);
    if (s2) {
        if (walk_indirect(fd, sb, s2, 2, &logical, cb, ctx) < 0)
            return -1;
    } else {
        uint32_t block_size = ext2_block_size(sb);
        uint32_t per = block_size / 4;
        logical += per * per;
    }

    uint32_t s3 = ext2_le32(inode->i_block[14]);
    if (s3) {
        if (walk_indirect(fd, sb, s3, 3, &logical, cb, ctx) < 0)
            return -1;
    }

    return 0;
}
