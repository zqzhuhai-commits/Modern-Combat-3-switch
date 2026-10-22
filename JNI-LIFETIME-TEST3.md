# Test3: JNI UTF character-buffer lifetime

The user's test2 log contains the correct `postload-test2` marker, a completed first frame, and `getDeviceLanguage -> en`. It then repeatedly opens the main OBB with its path prefix replaced by allocator-looking bytes. The newest crash entry remains a null resource-stream virtual dispatch at `libModernCombat3.so+0x8373c`. This rules out treating the missing language callback as a complete fix.

## Reproduced defect

The real JNIEnv table's GetStringUTFChars (slot 169) returned the String object's internal UTF buffer and reported `isCopy = false`. DeleteLocalRef (slot 23) destroyed that object and freed the buffer. A native consumer still holding the characters then read freed memory. ReleaseStringUTFChars (slot 170) was a no-op.

A new test invokes those actual JNIEnv slots, not a mocked string implementation. The test2 implementation fails with AddressSanitizer's `heap-use-after-free`; the allocation is `jni_str`/strdup and the freeing stack is DeleteLocalRef -> jni_release -> destroy. Its OBB sample path is 70 bytes and the invalid read starts at its first byte, consistent with the observed damaged prefixes. This reproduces a concrete runtime defect; full in-game causality still requires a hardware retest.

## Change

GetStringUTFChars now makes an independent UTF buffer, reports `isCopy = true`, and ReleaseStringUTFChars frees only that buffer without inspecting the String reference. This matches the reference implementation in EapRules/mc3-native-arm. Allocation failure records an OutOfMemoryError. Multiple acquisitions own separate buffers and can be released independently. The Java String storage continues to be owned by the Java object.

Also cap traced fopen messages at 64: opening an OBB per asset must not produce thousands of boot-log writes. OBB path translation and early-frame checkpoints remain enabled. Boot and first-frame logs identify `postload-test3`.

## Verification

- Actual-JNIEnv lifecycle test passes under ASan/UBSan for main/patch OBB paths, language and an empty string; two simultaneous acquisitions survive local-reference deletion and either buffer's release.
- The old implementation fails the same test with ASan; its report was retained locally during diagnosis.
- Existing JNI/audio/video, input focus, ATC and all 15 native export binding tests pass.
- ARM32 compilation and NSO three-segment/hash checks pass.
- Complete data-inclusive NSP packaging and independent readback checks are recorded with the test3 deliverable.

Version: 0.1.2-test3. No Switch hardware or full emulator run was performed here. The package is a test build, not a claim that all resource-loading or gameplay problems are resolved.
