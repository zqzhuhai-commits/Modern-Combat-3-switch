# Post-load JNI crash investigation (2026-10-22)

Baseline: `mc3-test-20261022` (`6a3c18df7448605367b4877eadaef0ee6ee74e9d`).

The supplied debug log completes all 267 constructors, EGL setup, native bootstrap and audio initialization. Its final unhandled call is `com/gameloft/glf/GL2JNILib.getDeviceLanguage()Ljava/lang/String;`. The supplied crash log records a null data read at `libModernCombat3.so+0x8373c` (FAR=0, instruction `e5933000`, r3=0). The runtime returns NULL for an unhandled object call, and GetStringUTFChars(NULL) also returns NULL. Disassembly of the recovered original library shows that +0x83710 is a resource-stream method: it loads the inner pointer through [this+8], then dereferences its vtable at +0x8373c. This confirms a null inner resource stream, rather than a direct GetStringUTFChars fault. The missing language string remains a possible upstream cause of incorrect resource selection; hardware retesting is required to establish causality.

Changes:

- Return a real Java string (`en`, matching the existing resource language) for the exact language signature on GL2JNILib and game classes, matching the reference port.
- Move getSystemService handling before the game-class filter and accept Context/Activity declarations as well as game classes. The supplied log shows the method declared on Context; previously that path returned unhandled despite having a service implementation later in the function.
- Explicitly acknowledge Android Process.setThreadPriority(I)V without mapping Android priorities to Horizon priorities. This was another unhandled startup call in the supplied log.
- Keep both port source copies synchronized.

Validation:

- `bash tests/run_host_tests.sh`: JNI/audio/video, input focus and ATC upload tests pass under AddressSanitizer and UndefinedBehaviorSanitizer.
- Added coverage for the language object and its contents, wrong class/signature rejection, Context/Activity/game-class service queries and thread-priority handling.
- The release-baseline handler fails the new language test, confirming the regression test detects the missing implementation.
- `git diff --check` passes.
- `bash build.sh` passes with restored devkitARM/libnx32/Mesa32. The NSO converter verifies all three segments and their hashes. The generated mc3_nx.nsp is an ExeFS build container, not a complete installable NSP.

## Test2 package follow-up

- Recovered the original library and OBB files from the prior release, whose full NSP SHA-256 matches its published digest. All three recovered game files match the recipe hashes.
- Added OBB path/open tracing, a test2 boot marker, first-frame entry logging and frame counts at 1/10/60 before the regular 300-frame checkpoints.
- Cross-checked the English language response and registration on both GL/activity classes with EapRules/mc3-native-arm.
- Added a validated --version option to the complete NSP packaging script; test2 displays 0.1.1-test2.
- Generated a complete data-inclusive NSP; separate deliverable verification records the hashes and package checks. This is an unsigned homebrew package for the same Atmosphere test environment as the previous release.

No complete game run or hardware validation has been performed. The existing GitHub Release remains unchanged; the replacement test NSP is delivered separately.
