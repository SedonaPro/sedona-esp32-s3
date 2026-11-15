#ifndef _CRC32_H_
#define _CRC32_H_ 1

#include <stdlib.h>
#include <stdint.h>
#include "bclimateLibNativeCrc32_conf.h" // for CRC32_POLY

#ifdef CRC32_POLY
uint32_t crc32_update      (uint32_t    crc, uint8_t byte);
uint32_t crc32_block       (const void* src, size_t  len);
uint32_t crc32_block_append(const void* src, size_t  len, uint32_t crc);
#endif

uint32_t crc32_poly_update      (uint32_t crc,    uint8_t byte, uint32_t poly);
uint32_t crc32_poly_block       (const void* src, size_t len,   uint32_t poly);
uint32_t crc32_poly_block_append(const void* src, size_t len,   uint32_t crc, uint32_t poly);

#endif

