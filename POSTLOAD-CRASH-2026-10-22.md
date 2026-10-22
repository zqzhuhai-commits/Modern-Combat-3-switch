# Post-load JNI crash investigation (2026-10-22)

Baseline: `mc3-test-20261022` (`6a3c18df7448605367b4877eadaef0ee6ee74e9d`).

The supplied debug log completes all 267 constructors, EGL setup, native bootstrap and audio initialization. Its final unhandled call is `com/gameloft/glf/GL2JNILib.getDeviceLanguage()Ljava/lang/String;`. The supplied crash log records a null data read at `libModernCombat3.so+0x8373c` (FAR=0, instruction `e5933000`, r3=0). The runtime returns NULL for an unhandled object call, and GetStringUTFChars(NULL) also returns NULL. The missing language string is therefore a likely trigger, but exact native symbolization and a hardware retest are still required to confirm causality.

Changes:

- Return a real Java string (`en`, matching the existing resource language) for the exact GL2JNILib language method and signature.
- Move getSystemService handling before the game-class filter and accept Context/Activity declarations as well as game classes. The supplied log shows the method declared on Context; previously that path returned unhandled despite having a service implementation later in the function.
- Explicitly acknowledge Android Process.setThreadPriority(I)V without mapping Android priorities to Horizon priorities. This was another unhandled startup call in the supplied log.
- Keep both port source copies synchronized.

Validation:

- `bash tests/run_host_tests.sh`: JNI/audio/video, input focus and ATC upload tests pass under AddressSanitizer and UndefinedBehaviorSanitizer.
- Added coverage for the language object and its contents, wrong class/signature rejection, Context/Activity/game-class service queries and thread-priority handling.
- The release-baseline handler fails the new language test, confirming the regression test detects the missing implementation.
- `git diff --check` passes.
- `bash build.sh` passes with restored devkitARM/libnx32/Mesa32. The NSO converter verifies all three segments and their hashes. The generated mc3_nx.nsp is an ExeFS build container, not a complete installable NSP.

No complete game run or hardware validation has been performed. No replacement data-inclusive NSP is included in this change. The existing release remains unchanged.
