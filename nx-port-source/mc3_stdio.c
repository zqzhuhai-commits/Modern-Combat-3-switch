/* SPDX-License-Identifier: MIT.
 * Android 2.3's stdio.h defines fpos_t as off_t (32-bit long on ARM).
 * Do not hand a guest fpos_t* to newlib: its off_t is 64 bits.
 * Reuse the runtime's FILE/errno conversion rather than dereference FILE.
 */
#include <stdint.h>
#include <stdio.h>

extern int32_t b_ftello(void *fp);
extern int b_fseeko(void *fp, int32_t off, int whence);
extern void b_set_errno(int linux_errno);

/* Unique address for this permanently loaded game module's C++ destructors. */
void *b___dso_handle = &b___dso_handle;

int b_fgetpos(void *fp, int32_t *pos) {
    if (!pos) { b_set_errno(22); return -1; }
    int32_t value = b_ftello(fp);
    if (value < 0) return -1;
    *pos = value;
    return 0;
}

int b_fsetpos(void *fp, const int32_t *pos) {
    if (!pos) { b_set_errno(22); return -1; }
    return b_fseeko(fp, *pos, SEEK_SET);
}

void b_rewind(void *fp) {
    b_fseeko(fp, 0, SEEK_SET);
}
