/* SPDX-License-Identifier: MIT.
 * GDRM unlock for Modern Combat 3: Fallen Nation (1.1.7g).
 * Every offset belongs to the 1.1.7g build whose libModernCombat3.so has
 * sha1 be0d5e8779899e081a538b3930ec711d2df8aeb4. Reference:
 * EapRules mc3-native-arm game/patch.cpp (verified on hardware).
 */
#include <stdint.h>
#include <stdlib.h>

#include "so_util.h"

static const struct {
    uint32_t    offset;
    const char *what;
} kDrmSlots[] = {
    {0x0098a278, "GDRM lock word 1"},
    {0x0098a26c, "GDRM lock word 2"},
    {0x0098a27c, "GDRM lock word 3"},
};

/*
 * Gameloft's licence check reads three pointers out of .data and dereferences
 * each one; a non-zero word behind every pointer is what it accepts as
 * "licensed". Set them immediately AFTER JNI_OnLoad returns -- the engine's
 * own init writes these words first, so patching before that would be
 * overwritten. Four bytes each, never freed.
 */
void mc3_unlock_drm(so_module *mod)
{
    if (!mod || !mod->load_virtbase) return;
    uint8_t *base = (uint8_t *)mod->load_virtbase;
    for (unsigned i = 0; i < sizeof(kDrmSlots) / sizeof(kDrmSlots[0]); i++) {
        int **slot = (int **)(base + kDrmSlots[i].offset);
        int *word = (int *)malloc(sizeof(int));
        if (!word) return;
        *word = 1;
        *slot = word;
    }
}
