#include "ext2.h"
#include "endian.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

struct cat_ctx {
    int fd;
    const struct ext2_superblock *sb;

    uint64_t remaining;
};

static int cat_block(uint32_t logical_block, uint32_t physical_block, void *opaque) {
    (void)logical_block;

    struct cat_ctx *ctx = opaque;

    uint32_t block_size =
        ext2_block_size(ctx->sb);

    uint8_t *buf = malloc(block_size);

    if (!buf)
        return -1;

    if (ext2_read_block(
            ctx->fd,
            ctx->sb,
            physical_block,
            buf) < 0) {

        free(buf);
        return -1;
    }

    uint64_t to_write =
        ctx->remaining;

    if (to_write > block_size)
        to_write = block_size;

    size_t done = 0;

    while (done < to_write) {

        ssize_t rc =
            write(STDOUT_FILENO,
                  buf + done,
                  to_write - done);

        if (rc <= 0) {
            free(buf);
            return -1;
        }

        done += (size_t)rc;
    }

    ctx->remaining -= to_write;

    free(buf);

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <image> <inode>\n", argv[0]);
        return 1;
    }

    uint32_t inode_num = (uint32_t)strtoul(argv[2], NULL, 10);

    int fd = open(argv[1], O_RDONLY);

    if (fd < 0) {
        perror("open");
        return 1;
    }

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

    struct cat_ctx ctx = {
        .fd = fd,
        .sb = &sb,
        .remaining = ext2_le32(inode.i_size),
    };

    if (ext2_inode_foreach_block(&inode, cat_block, &ctx) < 0) {
        perror("cat");
        close(fd);
        return 1;
    }

    close(fd);

    return 0;
}
