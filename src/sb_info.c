#include "ext2.h"
#include "endian.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <image>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);

    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct ext2_superblock sb;

    if (ext2_read_superblock(fd, &sb) < 0) {
        perror("read superblock");
        close(fd);
        return 1;
    }

    if (ext2_le16(sb.s_magic) != EXT2_SUPER_MAGIC) {
        fprintf(stderr, "not an ext2 filesystem\n");
        close(fd);
        return 1;
    }

    printf("magic: 0x%04x\n", ext2_le16(sb.s_magic));
    printf("block size: %u\n", ext2_block_size(&sb));
    printf("inode size: %u\n", ext2_le16(sb.s_inode_size));
    printf("blocks per group: %u\n", ext2_le32(sb.s_blocks_per_group));
    printf("inodes per group: %u\n", ext2_le32(sb.s_inodes_per_group));

    close(fd);

    return 0;
}
