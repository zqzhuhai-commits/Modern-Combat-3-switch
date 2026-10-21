# 路线 A：Switchroot Android（原生 APK）

在 Switchroot Android（LineageOS 17.1/18.1）上以原生 APK 方式运行，Tegra 原生
GPU 加速，性能最稳的路线。

## 前置条件

- Switchroot Android 已安装（[官方教程](https://wiki.switchroot.org/en/Android/Setup-Guide)）
- TF 卡剩余空间 ≥ 3GB
- Pro 手柄 / Joy-Con

## 安装

1. 安装 `Modern Combat 3.apk`（v1.1.7g，允许"安装未知应用"）。
2. 把两个 OBB 放到内置存储：
   ```
   sdcard/Android/obb/com.gameloft.android.ANMP.GloftM3HM/
   ├── main.1120.com.gameloft.android.ANMP.GloftM3HM.obb    (约 1.34GB)
   └── patch.11428.com.gameloft.android.ANMP.GloftM3HM.obb   (约 81MB)
   ```
3. 数据校验参数见仓库根 `loader/mc3.eapx.json`（SHA-256 + 关键区），
   与正版 v1.1.7g 一致方可运行。

## 手柄映射

MC3 是纯触摸游戏，Android 下用手柄映射工具把触摸区域映射到按键：

- **Panda Gamepad Pro**（免费、无需 root，无障碍服务实现）
  1. 安装并授权无障碍
  2. 映射方案：左摇杆→左下虚拟摇杆（移动）；右摇杆→右侧滑动区（视角）；
     R1→开火、L1→机瞄、R2→换弹、L2→手雷；上→冲刺、下→下蹲
  3. 从 Panda 启动游戏

## 说明

- 游戏 APK/OBB 为 Gameloft 版权文件，本仓库不提供，请自备。
- 若启动闪退，先重启游戏一次；部分 UI 直接用触屏。
