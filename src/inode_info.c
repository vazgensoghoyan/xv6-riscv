#include "ext2.h"
#include "endian.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void print_blocks(const struct ext2_inode *inode) {
    puts("direct blocks:");

    for (int i = 0; i < 12; i++)
        printf("  %u\n", ext2_le32(inode->i_block[i]));

    printf("single indirect: %u\n", ext2_le32(inode->i_block[12]));
    printf("double indirect: %u\n", ext2_le32(inode->i_block[13]));
    printf("triple indirect: %u\n", ext2_le32(inode->i_block[14]));
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

    printf("inode: %u\n", inode_num);

    uint16_t mode = ext2_le16(inode.i_mode);

    printf("mode: 0%o\n", mode & 07777);

    printf("type: ");
    switch (mode & 0xF000) {
        case 0x4000: printf("directory\n"); break;
        case 0x8000: printf("file\n"); break;
        case 0xA000: printf("symlink\n"); break;
        default: printf("other\n"); break;
    }

    printf("uid: %u\n", ext2_le16(inode.i_uid));
    printf("gid: %u\n", ext2_le16(inode.i_gid));
    printf("size: %u\n", ext2_le32(inode.i_size));
    printf("links: %u\n", ext2_le16(inode.i_links_count));
    printf("blocks: %u\n", ext2_le32(inode.i_blocks));

    print_blocks(&inode);

    close(fd);

    return 0;
}
