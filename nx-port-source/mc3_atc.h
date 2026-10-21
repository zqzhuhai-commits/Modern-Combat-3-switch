/* SPDX-License-Identifier: MIT */
#ifndef MC3_ATC_H
#define MC3_ATC_H
#include <stddef.h>
#include <stdint.h>
#define MC3_ATC_RGB 0x8c92u
#define MC3_ATC_EXPLICIT_ALPHA 0x8c93u
#define MC3_ATC_INTERPOLATED_ALPHA 0x87eeu
/* Success returns a malloc-owned tightly packed RGBA8 image. On failure,
 * *out is NULL. Validate every dimension/size before reading input. */
int mc3_atc_decode(unsigned format, int width, int height, int bytes,
                  const void *data, uint8_t **out);
#endif
