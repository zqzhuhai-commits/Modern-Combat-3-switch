#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
MC3_TOOLCHAIN_ROOT="${MC3_TOOLCHAIN_ROOT:-$HERE/../toolchain}"
export DEVKITPRO="${DEVKITPRO:-$MC3_TOOLCHAIN_ROOT/v2h-root/opt/devkitpro}"
export DEVKITARM="${DEVKITARM:-$DEVKITPRO/devkitARM}"
export PORTLIBS="${PORTLIBS:-$MC3_TOOLCHAIN_ROOT/mesa32/mesa32}"
missing=0
for required in "$HERE/runtime/runtime.mk" "$DEVKITARM/bin/arm-none-eabi-gcc" \
    "$DEVKITPRO/libnx32/lib/libnx.a" "$DEVKITPRO/libnx32/lib/libminiz.a" \
    "$PORTLIBS/lib/libEGL.a" "$PORTLIBS/lib/libGLESv2.a" \
    "$DEVKITPRO/tools/bin/npdmtool" "$DEVKITPRO/tools/bin/build_pfs0"; do
    if [ ! -e "$required" ]; then
        echo "Missing build prerequisite: $required" >&2
        missing=1
    fi
done
[ "$missing" -eq 0 ] || exit 1
export PATH="$DEVKITARM/bin:$DEVKITPRO/tools/bin:$PATH"
cd "$HERE"
exec make -j"${MC3_BUILD_JOBS:-4}" "$@"
