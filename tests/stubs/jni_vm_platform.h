#pragma once
/* The host test does not exercise the ARM bionic syscall layouts. */
#define DCR_BIONIC_H
#include <stddef.h>
#include <stdarg.h>
int b_vsnprintf(char *,size_t,const char *,va_list);
