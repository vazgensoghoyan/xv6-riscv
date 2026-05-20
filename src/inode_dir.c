#include "ext2.h"
#include "endian.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct dir_ctx {
    int fd;
    const struct ext2_superblock *sb;
};

struct ext2_dir_entry_2 {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char name[];
};

static int dir_block(uint32_t logical, uint32_t physical, void *ctx_void) {
    (void)logical;

    struct dir_ctx *ctx = ctx_void;

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

        struct ext2_dir_entry_2 *e =
            (struct ext2_dir_entry_2 *)(buf + offset);

        if (e->rec_len == 0)
            break;

        if (e->inode != 0) {
            printf("%u\t%.*s\n",
                   ext2_le32(e->inode),
                   e->name_len,
                   e->name);
        }

        offset += ext2_le16(e->rec_len);
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
        perror("sb");
        return 1;
    }

    struct ext2_inode inode;
    if (ext2_read_inode(fd, &sb, inode_num, &inode) < 0) {
        perror("inode");
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
