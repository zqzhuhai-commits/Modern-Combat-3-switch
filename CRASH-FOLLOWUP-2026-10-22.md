# 首次实机崩溃后的修复

实机日志显示原版资源校验、导出绑定、EGL 初始化、原生初始化均返回成功，进入帧循环后发生 data abort；PC 为原版库 +0x8373c，FAR=0、r3=0。反汇编对应 `ldr r3,[r3]`，处于原生流对象调用链。当前证据不能证明最终根因已完全消除。

本次处理已由日志证实的三个兼容层缺口：

- `android/content/Context.getSystemService(String)`：现有实现位于类名过滤之后，Context 类被提前拒绝，实机因此收到默认空返回值。把处理移到过滤之前，限定 Context、Activity 与游戏类；保留具体服务类型返回。
- `GL2JNILib.getDeviceLanguage()String`：新增非空 `en` 返回，与当前固定英语设置一致。此前处理器仅有 getLanguage，未覆盖实际请求名。
- `android/os/Process.setThreadPriority(int)`：显式处理为无操作；Android 的 nice 值不能直接当作 Switch 线程优先级。运行时自身已经配置调度，不在此伪造数值转换。

同时打开 `.obb` 路径跟踪，让下一次实机日志记录包路径转换与打开结果。没有对原版崩溃指令打补丁，也没有跳过失败的资源流读取。

验证：新增 Context/Activity/游戏类的服务路由、服务对象类型、语言返回、线程优先级调用和无关类拒绝测试；旧处理器在新增 Context 路由断言处失败，新处理器通过。宿主 JNI/音频/视频、输入、ATC 测试均通过 ASan/UBSan，原版库 15 项导出绑定零缺失。Switch ARM32 重新编译成功，NSO 三段回读哈希通过。

这些检查不等于完整模拟器或实机运行通过。下一次实机测试应使用新安装包；若再次退出，需要该次 debug.log 和 crash.log，重点检查服务/语言警告是否消失、OBB 打开结果和崩溃偏移是否变化。
