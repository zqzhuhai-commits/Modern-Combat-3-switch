# 路线 B：Switchroot Ubuntu (L4T) + 加载器

保留原版 PortMaster 加载器体验（手柄直连、指针模式）的 Switch 适配。
加载器为 armhf 二进制，直接加载游戏原版 `libModernCombat3.so`，因此 Switch 的
Ubuntu 需要 **armhf 多架构** 支持；GPU 走 Mesa **软件渲染**（llvmpipe/kms_swrast），
因为 NVIDIA L4T 闭源驱动没有 32 位 GL 用户层。

## 前置条件

- Switchroot Ubuntu 20.04+（L4T，64 位）已安装（[官方教程](https://wiki.switchroot.org/en/Linux/Ubuntu-Install)）
- TF 卡剩余空间 ≥ 3GB
- Pro 手柄（推荐 USB）或 Joy-Con 组合体

## 安装（在 Switch 的 Ubuntu 里，一次即可）

```bash
sudo bash setup_mc3_switch.sh
```

脚本内容：添加 armhf 架构 → `apt install` armhf 运行库（libc6、libsdl2-2.0-0、
libegl1、libgles2、libgl1-mesa-dri、libgbm1、libdrm2、libudev1、libasound2、
libpulse0 等）→ 加入 video/input 组。Ubuntu 18.04 会明确报错（glibc 2.27 不满足）。

## 使用

1. 把本仓库 `loader/` 全部内容拷贝到 Ubuntu（如 `~/mc3/`），加上 `run_mc3.sh`。
2. 放入你自己的 v1.1.7g APK + 两个 OBB（文件名随意），首次运行自动导入校验：
   ```bash
   bash ~/mc3/run_mc3.sh
   ```
3. 画面不出来时强制 kmsdrm：`SDL_VIDEODRIVER=kmsdrm bash ~/mc3/run_mc3.sh`
   （kmsdrm 需在 VT/控制台会话运行；桌面会话自动走 x11/wayland）
4. 日志在 `~/mc3/log.txt`。

## 说明与已知问题

- 加载器自身处理手柄输入（SDL_GameController）与退出（游戏内暂停菜单），
  不需要 gptokeyb / PortMaster 运行时。
- 软件渲染：2011 年引擎在 Switch 4×A57 上预计 30fps 上下；`MC3_SCALE=stretch`
  可铺满 720p 屏幕（默认 fit 保持 4:3）。
- 上游待修问题：暂停菜单里重启任务崩溃（原版移植已知问题）。
- 本目录脚本与 `loader/` 均为 GPL-3.0（见仓库根 LICENSE）。
