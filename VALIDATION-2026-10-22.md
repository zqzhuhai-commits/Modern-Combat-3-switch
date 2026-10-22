# 2026-10-22 本地修复与验证

基线：`c2758191c4dab061296ac09d7ed63c61a2ead52a`。

## 本次修复

- 原版 ELF 实际导出 `Java_video_MyVideoView_Init` / `SetFinished`；此前多加 `com_gameloft_` 会使 15 项必需导出绑定中有两项失败，启动停在加载阶段。
- JNI AudioTrack：单声道掩码 4 不再判为立体声；short[] 按 offset 读取；write 返回短整数数量，保留错误码。
- Activity.startActivity/startActivityForResult 在类名过滤前接入视频跳过；补充 MyVideoView.playMovie，90 帧后仅通知完成一次。
- JNI 原生入口收到真正的 Class 对象（JClass.obj），resize 和视频初始化不再收到空 class；视频完成回调不再传伪造地址。
- 失焦释放全部 Android 按键和左右摇杆，复焦重新检测按住状态；合并共用一个 Android 键码的实体按键，避免 X/ZR 等组合松开其中一个时误释放另一个。
- 同步 nx-port-source/source 镜像，消除其与主源码的 RomFS 挂载等差异。

## 已执行且通过

`bash tests/run_host_tests.sh /absolute/path/libModernCombat3.so`

1. 实际 JNI 处理器的音频/视频测试：单/双声道、非零 offset、负值与越界拒绝、返回单位、错误码、三类视频调用、89/90 帧边界及重复完成抑制。
2. 实际输入代码的状态测试：确认/射击/方向按键、左右摇杆、模拟点击与滑动在失焦后释放，复焦重建状态，X/ZR 组合释放互不干扰。
3. 已有 ATC 上传测试：RGB/alpha、边缘裁剪、错误长度、非 ATC 转发及子纹理上传。
4. 上述 C 测试均启用 AddressSanitizer 与 UndefinedBehaviorSanitizer；LeakSanitizer 未启用。Switch 平台服务由宿主桩替代。
5. 真正调用 mc3_bind 并查询提供的原版 ELF 动态符号，15 项全部命中。
6. 使用修复前代码运行相同测试，音频单声道断言、失焦释放断言及两个视频导出绑定均失败，确认测试能够识别这些回归。
7. devkitARM / libnx32 / Mesa32 交叉编译并链接成功；ELF32→NSO 的三个加载段与哈希回读检查通过。
8. Git diff 空白检查通过。

## 未执行，不能宣称通过

没有 Switch 模拟器、固件环境或实机，因此没有运行完整原版游戏，也没有验证进入菜单、任务加载、绘制正确性、声音听感、存档、HOME/休眠。宿主接口测试与交叉编译不能替代模拟器或实机测试。

本次构建生成的 mc3_nx.nsp 是裸 ExeFS 容器，不是可直接安装的完整 NSP。本次未提供打包密钥，未重新生成含 Program/Control/Meta NCA 的完整安装包。完整封装继续使用 nx-port-tools/package_installable.py；必须自备打包密钥和校验通过的资源。

游戏文字仍固定英语，未在本次改为中文；原先未实现的 JNI / APK resource 查询仍有待运行时日志确认。过场仍为跳过实现。
