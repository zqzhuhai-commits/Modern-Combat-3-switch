# 现代战争3:堕落国度 — Switch 移植开发

当前是 AArch32 运行时开发版，已重新编译；尚未通过 Switch 实机验证。

`build.sh` 生成的 `mc3_nx.nsp` 只有裸 ExeFS 内容，供后续封装使用，不能当成 DBI 可安装的完整 NSP。历史上提交的同名文件已从本开发分支移除。完整安装包需要 Program、Control、Meta NCA 与 CNMT，并包含游戏数据的 RomFS。

## 已恢复的修复

- 补齐公共运行时；纠正源码目录、构建工具路径和 JNI 初始化调用。
- 根据原版库的导出表修正 `Java_video_MyVideoView_Init` / `SetFinished`。
- 修正 AudioTrack 短数组写入的偏移、返回单位和单声道配置；失焦时释放输入。
- 接通 ATC 纹理解码与 GLES2 上传；接通过场跳过的两个 Java 调用入口。
- 优先读取包内 `game:/` RomFS 的原版库和 OBB；存档、配置、日志继续放在 SD 卡目录。
- 中文标题为“现代战争3:堕落国度”，图标位于 `assets/mc3-icon-256.jpg`。菜单中文名称不代表游戏正文已汉化。

## 构建

需要 Python 3.12+、make 和宿主编译工具。构建工具不随源码分发，可执行：

```bash
python3 nx-port-tools/bootstrap_toolchain.py
bash build.sh
```

工具链默认放在仓库的兄弟目录 `toolchain/`；也可通过 `MC3_TOOLCHAIN_ROOT`、`DEVKITPRO`、`DEVKITARM` 和 `PORTLIBS` 指定已安装环境。恢复脚本使用固定 SHA-256 的 vita2hos 容器层、libnx32 4.12.0 和 mesa32 release。下载需要网络访问 GHCR/GitHub；宿主平台为 Linux x86_64。

公共运行时来自 `zqzhuhai-commits/shadowguardian-switch` 的 `runtime/`，固定于提交 `8877f2d991ba624fb54cd0f715f642e850b78ad3`，保留其 MIT 许可。原项目加载器与游戏粘合代码的许可证见 LICENSE、NOTICE.md 与文件头。

## 封装完整测试 NSP

请自备 Android v1.1.7g 的以下三个文件；打包脚本按 `loader/mc3.eapx.json` 严格检查大小和 SHA-256：

- `libModernCombat3.so`，9,766,612 字节，来自 APK 的 armeabi-v7a 目录。
- `main.1120.com.gameloft.android.ANMP.GloftM3HM.obb`，1,342,717,349 字节。
- `patch.11428.com.gameloft.android.ANMP.GloftM3HM.obb`，81,507,652 字节。

编译完成后，使用本地 hacBrewPack 和自己的打包密钥：

```bash
python3 nx-port-tools/package_installable.py \
  --data-dir /path/to/validated-game-files \
  --keys /private/path/prod.keys \
  --packer /path/to/hacbrewpack \
  --nacptool ../toolchain/v2h-root/opt/devkitpro/tools/bin/nacptool \
  --output /path/to/test-nsp-output
```

脚本把原版数据放入 Program RomFS，生成中文 NACP 和菜单图标，使用 Title ID `0100F5344D433300`。输出是需要大气层环境和实机验证的测试包，不应宣称已验证安装或通关。密钥与原版游戏数据不会写入源码树。

## 验证范围与后续

交叉编译、ELF32→NSO 段哈希检查、缓存导入表扫描、ATC 上传宿主测试和脚本语法检查已通过。当前诊断启动界面需按 A 才进入游戏初始化，PLUS 退出；游戏内 PLUS+MINUS 退出。日志与存档目录为 `sd:/switch/mc3_nx/`。

尚待实机验证：安装、JNI 剩余调用、游戏主菜单与任务加载、纹理和着色器、声音、存档、HOME/休眠恢复、掌机/底座模式。过场当前按参考移植方式跳过，不提供视频播放实现。

2026-10-22 恢复时，原资源链接已返回 HTTP 403，当前没有重新生成完整 NSP。此前生成但未保存的完整封装产物已丢失；需要重新提供资源包后重打包。详见 `REVIEW-2026-10-22.md`。

Android 和 Switchroot Linux 的独立运行说明分别保留在 `android/` 与 `l4t/`。


## 最新本地验证（2026-10-22）

已继续修复原版视频导出绑定、JNI 音频单位与偏移、视频回调、JNI class 参数及失焦手柄状态，并完成带 ASan/UBSan 的宿主接口测试、原版 ELF 的 15 项导出绑定检查和 Switch ARM32 交叉编译。运行 `bash tests/run_host_tests.sh /absolute/path/libModernCombat3.so` 可复验；省略原版库参数可只运行宿主接口测试。完整结果及未验证范围见 [VALIDATION-2026-10-22.md](VALIDATION-2026-10-22.md)。尚未运行 Switch 模拟器或实机，不能据此认定完整游戏启动通过。
