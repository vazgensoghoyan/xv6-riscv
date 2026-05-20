#pragma once

#include <stdint.h>

#define EXT2_SUPER_OFFSET 1024
#define EXT2_SUPER_SIZE   1024
#define EXT2_SUPER_MAGIC  0xEF53

// SUPERBLOCK

struct ext2_superblock {
    uint32_t s_inodes_count;
    uint32_t s_blocks_count;
    uint32_t s_r_blocks_count;
    uint32_t s_free_blocks_count;
    uint32_t s_free_inodes_count;
    uint32_t s_first_data_block;
    uint32_t s_log_block_size;
    uint32_t s_log_frag_size;
    uint32_t s_blocks_per_group;
    uint32_t s_frags_per_group;
    uint32_t s_inodes_per_group;
    uint32_t s_mtime;
    uint32_t s_wtime;
    uint16_t s_mnt_count;
    uint16_t s_max_mnt_count;
    uint16_t s_magic;
    uint16_t s_state;
    uint16_t s_errors;
    uint16_t s_minor_rev_level;
    uint32_t s_lastcheck;
    uint32_t s_checkinterval;
    uint32_t s_creator_os;
    uint32_t s_rev_level;
    uint16_t s_def_resuid;
    uint16_t s_def_resgid;

    uint32_t s_first_ino;
    uint16_t s_inode_size;
};

// GROUP DESC

struct ext2_group_desc {
    uint32_t bg_block_bitmap;
    uint32_t bg_inode_bitmap;
    uint32_t bg_inode_table;
    uint16_t bg_free_blocks_count;
    uint16_t bg_free_inodes_count;
    uint16_t bg_used_dirs_count;
    uint16_t bg_pad;
    uint8_t  bg_reserved[12];
};

// INODE

struct ext2_inode {
    uint16_t i_mode;
    uint16_t i_uid;
    uint32_t i_size;
    uint32_t i_atime;
    uint32_t i_ctime;
    uint32_t i_mtime;
    uint32_t i_dtime;
    uint16_t i_gid;
    uint16_t i_links_count;
    uint32_t i_blocks;
    uint32_t i_flags;
    uint32_t i_osd1;
    uint32_t i_block[15];
    uint32_t i_generation;
    uint32_t i_file_acl;
    uint32_t i_dir_acl;
    uint32_t i_faddr;
    uint8_t  i_osd2[12];
};

// API

int ext2_read_superblock(int fd, struct ext2_superblock *sb);

uint32_t ext2_block_size(const struct ext2_superblock *sb);

int ext2_read_group_desc(int fd, const struct ext2_superblock *sb,
                         uint32_t group, struct ext2_group_desc *gd);

int ext2_read_inode(int fd, const struct ext2_superblock *sb,
                    uint32_t inode_num, struct ext2_inode *inode);

int ext2_read_block(int fd, const struct ext2_superblock *sb,
                    uint32_t block_num, void *buf);

// callback on each block
typedef int (*ext2_block_cb)(uint32_t logical_block, uint32_t physical_block, void *ctx);

int ext2_inode_foreach_block(int fd, const struct ext2_superblock *sb,
                             const struct ext2_inode *inode,
                             ext2_block_cb cb, void *ctx);
