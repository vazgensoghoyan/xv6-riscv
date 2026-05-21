#pragma once

#include <stdint.h>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define ext2_le16(x) (x)
#define ext2_le32(x) (x)

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

static inline uint16_t ext2_bswap16(uint16_t x)
{
    return (x >> 8) | (x << 8);
}

static inline uint32_t ext2_bswap32(uint32_t x)
{
    return ((x & 0x000000FFU) << 24) |
           ((x & 0x0000FF00U) << 8)  |
           ((x & 0x00FF0000U) >> 8)  |
           ((x & 0xFF000000U) >> 24);
}

#define ext2_le16(x) ext2_bswap16(x)
#define ext2_le32(x) ext2_bswap32(x)

#else
#error Unsupported endian
#endif
