#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p test-build
CC="${CC:-cc}"
flags=(-D_POSIX_C_SOURCE=200809L -std=c11 -g -fsanitize=address,undefined -fno-omit-frame-pointer
       -I tests/stubs -I nx-port-source -I runtime/source)
export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=0}"
"$CC" "${flags[@]}" tests/jni_bridge_test.c nx-port-source/mc3_jni.c nx-port-source/mc3_video.c -o test-build/jni_bridge_test
./test-build/jni_bridge_test
"$CC" "${flags[@]}" tests/input_release_test.c nx-port-source/mc3_input.c -o test-build/input_release_test
./test-build/input_release_test
"$CC" "${flags[@]}" tests/atc_upload_test.c nx-port-source/mc3_atc.c nx-port-source/mc3_gl.c -o test-build/atc_upload_test
./test-build/atc_upload_test
echo 'ATC upload tests passed'
"$CC" "${flags[@]}" '-DPORT_PAYLOAD_NAME="mc3_nx"' -include tests/stubs/jni_vm_platform.h \
    tests/jni_string_lifetime_test.c runtime/source/jni_core.c -o test-build/jni_string_lifetime_test
./test-build/jni_string_lifetime_test
if [ "$#" -gt 0 ]; then
    "$CC" -shared -fPIC nx-port-source/mc3_api.c -o test-build/api.so
    python3 tests/check_native_exports.py "$1" test-build/api.so
fi
