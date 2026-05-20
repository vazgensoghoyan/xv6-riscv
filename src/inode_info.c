#include "ext2.h"
#include "endian.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>

static void print_time(const char *label, uint32_t t) {
    time_t ts = (time_t)t;
    char buf[64];
    struct tm *tm = gmtime(&ts);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S UTC", tm);
    printf("%-20s %s\n", label, buf);
}

static const char *type_str(uint16_t mode) {
    switch (mode & 0xF000) {
        case 0x1000: return "FIFO";
        case 0x2000: return "character device";
        case 0x4000: return "directory";
        case 0x6000: return "block device";
        case 0x8000: return "regular file";
        case 0xA000: return "symbolic link";
        case 0xC000: return "socket";
        default:     return "unknown";
    }
}

static void print_perm(uint16_t mode) {
    char p[11];
    p[0] = (mode & 0xF000) == 0x4000 ? 'd' : (mode & 0xF000) == 0xA000 ? 'l' : '-';
    p[1] = (mode & 0100) ? 'r' : '-';
    p[2] = (mode & 0200) ? 'w' : '-';
    p[3] = (mode & 04000) ? 's' : (mode & 0400) ? 'x' : '-';
    p[4] = (mode & 010)  ? 'r' : '-';
    p[5] = (mode & 020)  ? 'w' : '-';
    p[6] = (mode & 02000) ? 's' : (mode & 040) ? 'x' : '-';
    p[7] = (mode & 01)   ? 'r' : '-';
    p[8] = (mode & 02)   ? 'w' : '-';
    p[9] = (mode & 01000) ? 't' : (mode & 04) ? 'x' : '-';
    p[10] = '\0';
    printf("%-20s %s  (0%o)\n", "permissions:", p, mode & 07777);
}

static void print_blocks(const struct ext2_inode *inode) {
    printf("\n--- block map ---\n");
    printf("direct:\n");
    for (int i = 0; i < 12; i++) {
        uint32_t b = ext2_le32(inode->i_block[i]);
        if (b) printf("  [%2d] %u\n", i, b);
    }
    uint32_t s1 = ext2_le32(inode->i_block[12]);
    uint32_t s2 = ext2_le32(inode->i_block[13]);
    uint32_t s3 = ext2_le32(inode->i_block[14]);
    if (s1) printf("single indirect:  block %u\n", s1);
    if (s2) printf("double indirect:  block %u\n", s2);
    if (s3) printf("triple indirect:  block %u\n", s3);
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

    if (ext2_le16(sb.s_magic) != EXT2_SUPER_MAGIC) {
        fprintf(stderr, "not an ext2 filesystem\n");
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

    uint64_t size = ext2_le32(inode.i_size);
    if ((mode & 0xF000) == 0x8000) {
        uint32_t upper = ext2_le32(inode.i_dir_acl);
        size |= (uint64_t)upper << 32;
    }

    printf("=== inode %u ===\n", inode_num);
    printf("%-20s %s\n",        "type:",    type_str(mode));
    print_perm(mode);
    printf("%-20s %u\n",        "uid:",     ext2_le16(inode.i_uid));
    printf("%-20s %u\n",        "gid:",     ext2_le16(inode.i_gid));
    printf("%-20s %llu\n",      "size:",    (unsigned long long)size);
    printf("%-20s %u\n",        "links:",   ext2_le16(inode.i_links_count));
    printf("%-20s %u  (512-byte sectors)\n", "i_blocks:", ext2_le32(inode.i_blocks));
    printf("%-20s 0x%08x\n",    "flags:",   ext2_le32(inode.i_flags));

    print_time("atime:", ext2_le32(inode.i_atime));
    print_time("ctime:", ext2_le32(inode.i_ctime));
    print_time("mtime:", ext2_le32(inode.i_mtime));
    if (ext2_le32(inode.i_dtime))
        print_time("dtime:", ext2_le32(inode.i_dtime));

    print_blocks(&inode);

    close(fd);

    return 0;
}
