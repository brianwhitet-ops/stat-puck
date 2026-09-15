#include "png_write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

static int write_chunk(FILE *f, const char *type, const unsigned char *data, int len)
{
    unsigned char hdr[4];
    hdr[0] = (unsigned char)((len >> 24) & 0xFF);
    hdr[1] = (unsigned char)((len >> 16) & 0xFF);
    hdr[2] = (unsigned char)((len >> 8) & 0xFF);
    hdr[3] = (unsigned char)(len & 0xFF);
    if (fwrite(hdr, 1, 4, f) != 4) {
        return -1;
    }
    if (fwrite(type, 1, 4, f) != 4) {
        return -1;
    }
    if (len > 0 && data && fwrite(data, 1, (size_t)len, f) != (size_t)len) {
        return -1;
    }

    unsigned char *crc_in = (unsigned char *)malloc((size_t)len + 4);
    if (!crc_in) {
        return -1;
    }
    memcpy(crc_in, type, 4);
    if (len > 0 && data) {
        memcpy(crc_in + 4, data, (size_t)len);
    }
    unsigned long c = crc32(0L, crc_in, (uInt)(len + 4));
    free(crc_in);
    hdr[0] = (unsigned char)((c >> 24) & 0xFF);
    hdr[1] = (unsigned char)((c >> 16) & 0xFF);
    hdr[2] = (unsigned char)((c >> 8) & 0xFF);
    hdr[3] = (unsigned char)(c & 0xFF);
    return fwrite(hdr, 1, 4, f) == 4 ? 0 : -1;
}

int slab_write_png(const char *path, const slab_fb_t *fb)
{
    const int w = SLAB_FB_W;
    const int h = SLAB_FB_H;
    const int row_bytes = w * 4;
    unsigned long raw_len = (unsigned long)(h * (1 + row_bytes));
    unsigned char *raw = (unsigned char *)malloc(raw_len);
    if (!raw) {
        return -1;
    }

    for (int y = 0; y < h; y++) {
        raw[y * (1 + row_bytes)] = 0;
        memcpy(raw + y * (1 + row_bytes) + 1, fb->rgba + y * row_bytes, (size_t)row_bytes);
    }

    uLongf csz = compressBound(raw_len);
    unsigned char *comp = (unsigned char *)malloc(csz);
    if (!comp) {
        free(raw);
        return -1;
    }
    if (compress2(comp, &csz, raw, raw_len, Z_BEST_COMPRESSION) != Z_OK) {
        free(raw);
        free(comp);
        return -1;
    }
    free(raw);

    FILE *f = fopen(path, "wb");
    if (!f) {
        free(comp);
        return -1;
    }

    const unsigned char sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    fwrite(sig, 1, 8, f);

    unsigned char ihdr[13];
    ihdr[0] = (unsigned char)((w >> 24) & 0xFF);
    ihdr[1] = (unsigned char)((w >> 16) & 0xFF);
    ihdr[2] = (unsigned char)((w >> 8) & 0xFF);
    ihdr[3] = (unsigned char)(w & 0xFF);
    ihdr[4] = (unsigned char)((h >> 24) & 0xFF);
    ihdr[5] = (unsigned char)((h >> 16) & 0xFF);
    ihdr[6] = (unsigned char)((h >> 8) & 0xFF);
    ihdr[7] = (unsigned char)(h & 0xFF);
    ihdr[8] = 8;
    ihdr[9] = 6; /* RGBA */
    ihdr[10] = 0;
    ihdr[11] = 0;
    ihdr[12] = 0;

    int rc = 0;
    rc |= write_chunk(f, "IHDR", ihdr, 13);
    rc |= write_chunk(f, "IDAT", comp, (int)csz);
    rc |= write_chunk(f, "IEND", NULL, 0);

    free(comp);
    fclose(f);
    return rc;
}
