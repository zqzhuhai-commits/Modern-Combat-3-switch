/* SPDX-License-Identifier: MIT
 * ATC palette/alpha rules verified against AMD Compressonator:
 * cmp_compressonatorlib/ati/compressonatori_tc.c and atc/codec_atc.cpp.
 * Attribution and MIT terms: licenses/AMD-Compressonator-MIT.txt.
 */
#include "mc3_atc.h"
#include <stdlib.h>
#include <string.h>

static unsigned le16(const uint8_t *p) { return p[0] | (unsigned)p[1] << 8; }
static unsigned expand5(unsigned v) { return (v << 3) | (v >> 2); }
static unsigned expand6(unsigned v) { return (v << 2) | (v >> 4); }

static void color_block(const uint8_t *src, uint8_t rgba[64]) {
    unsigned lo = le16(src), hi = le16(src + 2);
    unsigned palette[4][3];
    palette[0][0] = expand5((lo >> 10) & 31);
    palette[0][1] = expand5((lo >> 5) & 31);
    palette[0][2] = expand5(lo & 31);
    palette[3][0] = expand5(hi >> 11);
    palette[3][1] = expand6((hi >> 5) & 63);
    palette[3][2] = expand5(hi & 31);
    for (unsigned c = 0; c < 3; ++c) {
        if (lo & 0x8000) {
            palette[2][c] = palette[0][c];
            unsigned quarter = palette[3][c] >> 2;
            palette[1][c] = palette[0][c] > quarter ? palette[0][c] - quarter : 0;
            palette[0][c] = 0;
        } else {
            palette[1][c] = (5 * palette[0][c] + 3 * palette[3][c]) >> 3;
            palette[2][c] = (3 * palette[0][c] + 5 * palette[3][c]) >> 3;
        }
    }
    for (unsigned i = 0; i < 16; ++i) {
        unsigned index = (src[4 + i / 4] >> (2 * (i % 4))) & 3;
        for (unsigned c = 0; c < 3; ++c) rgba[4 * i + c] = palette[index][c];
        rgba[4 * i + 3] = 255;
    }
}

static void alpha_block(unsigned format, const uint8_t *src, uint8_t rgba[64]) {
    if (format == MC3_ATC_EXPLICIT_ALPHA) {
        for (unsigned i = 0; i < 16; ++i)
            rgba[4 * i + 3] = ((src[i / 2] >> (4 * (i % 2))) & 15) * 17;
    } else {
        unsigned a[8] = {src[0], src[1], 0, 0, 0, 0, 0, 255};
        unsigned steps = a[0] > a[1] ? 7 : 5;
        for (unsigned k = 1; k < steps; ++k)
            a[k + 1] = ((steps - k) * a[0] + k * a[1] + steps / 2) / steps;
        /* Six little-endian bytes hold all sixteen 3-bit selectors. */
        uint64_t indices = 0;
        for (unsigned k = 0; k < 6; ++k) indices |= (uint64_t)src[k + 2] << (8 * k);
        for (unsigned i = 0; i < 16; ++i) rgba[4 * i + 3] = a[(indices >> (3 * i)) & 7];
    }
}

int mc3_atc_decode(unsigned format, int width, int height, int bytes,
                  const void *data, uint8_t **out) {
    if (!out) return -1;
    *out = NULL;
    if (format != MC3_ATC_RGB && format != MC3_ATC_EXPLICIT_ALPHA &&
        format != MC3_ATC_INTERPOLATED_ALPHA) return -1;
    if (width <= 0 || height <= 0 || bytes < 0 || !data) return -1;
    size_t w = (size_t)width, h = (size_t)height;
    size_t bw = (w + 3) / 4, bh = (h + 3) / 4;
    size_t block_size = format == MC3_ATC_RGB ? 8 : 16;
    if (bw > SIZE_MAX / bh || bw * bh > SIZE_MAX / block_size ||
        bw * bh * block_size != (size_t)bytes ||
        w > SIZE_MAX / h || w * h > SIZE_MAX / 4) return -1;
    uint8_t *dest = malloc(w * h * 4);
    if (!dest) return -2;
    const uint8_t *src = data;
    for (size_t by = 0; by < bh; ++by) {
        for (size_t bx = 0; bx < bw; ++bx, src += block_size) {
            uint8_t rgba[64];
            color_block(src + (block_size == 16 ? 8 : 0), rgba);
            if (block_size == 16) alpha_block(format, src, rgba);
            for (size_t y = 0; y < 4 && by * 4 + y < h; ++y) {
                size_t pixels = w - bx * 4;
                if (pixels > 4) pixels = 4;
                memcpy(dest + ((by * 4 + y) * w + bx * 4) * 4,
                       rgba + y * 16, pixels * 4);
            }
        }
    }
    *out = dest;
    return 0;
}
