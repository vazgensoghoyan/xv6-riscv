#include "ext2.h"
#include "endian.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

struct dir_ctx {
    int fd;
    const struct ext2_superblock *sb;
};

#pragma pack(push, 1)
struct ext2_dir_entry_2 {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char name[];
};
#pragma pack(pop)

static int dir_block(uint32_t logical, uint32_t physical, void *ctx_void) {
    (void)logical;

    struct dir_ctx *ctx = (struct dir_ctx *)ctx_void;

    uint32_t block_size = ext2_block_size(ctx->sb);

    uint8_t *buf = malloc(block_size);
    if (!buf)
        return -1;

    if (ext2_read_block(ctx->fd, ctx->sb, physical, buf) < 0) {
        free(buf);
        return -1;
    }

    uint32_t offset = 0;

    while (offset < block_size) {

        struct ext2_dir_entry_2 *e = (struct ext2_dir_entry_2 *)(buf + offset);

        uint16_t rec_len = ext2_le16(e->rec_len);

        if (rec_len < 8)
            break;

        if (offset + rec_len > block_size)
            break;

        if (e->inode != 0) {
            printf("%u\t%.*s\n", ext2_le32(e->inode), e->name_len, e->name);
        }

        offset += rec_len;
    }

    free(buf);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <image> <inode>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    uint32_t inode_num = (uint32_t)strtoul(argv[2], NULL, 10);

    struct ext2_superblock sb;
    if (ext2_read_superblock(fd, &sb) < 0) {
        perror("superblock");
        close(fd);
        return 1;
    }

    struct ext2_inode inode;
    if (ext2_read_inode(fd, &sb, inode_num, &inode) < 0) {
        perror("inode");
        close(fd);
        return 1;
    }

    uint16_t mode = ext2_le16(inode.i_mode);

    if ((mode & 0xF000) != 0x4000) {
        fprintf(stderr, "error: inode %u is not a directory\n", inode_num);
        close(fd);
        return 1;
    }

    struct dir_ctx ctx = {
        .fd = fd,
        .sb = &sb
    };

    ext2_inode_foreach_block(fd, &sb, &inode, dir_block, &ctx);

    close(fd);
    return 0;
}
