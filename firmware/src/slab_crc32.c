#include "slab_crc32.h"
uint32_t slab_crc32(const void *bytes, size_t length)
{
    const uint8_t *p = (const uint8_t *)bytes;
    uint32_t crc = 0xffffffffu;
    for (size_t i = 0; i < length; i++) {
        crc ^= p[i];
        for (int bit = 0; bit < 8; bit++)
            crc = (crc >> 1) ^ ((crc & 1u) ? 0xedb88320u : 0);
    }
    return crc ^ 0xffffffffu;
}
