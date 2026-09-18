#ifndef SLAB_CRC32_H
#define SLAB_CRC32_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
uint32_t slab_crc32(const void *bytes, size_t length);
#ifdef __cplusplus
}
#endif
#endif
