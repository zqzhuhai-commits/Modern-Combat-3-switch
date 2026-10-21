# Modern Combat 3: Fallen Nation — Switch

让 Gameloft《现代战争3：失落国度》(v1.1.7g, 2011) 在 **Nintendo Switch** 上运行的工程。

核心加载器来自 [EapRules/mc3-native-arm](https://github.com/EapRules/mc3-native-arm)
（GPL-3.0）：一个 bionic/JNI 兼容加载器，在 Linux 上直接加载游戏原版
`libModernCombat3.so`（armeabi-v7a），无模拟、无重编译、完整手柄映射。
本仓库将其适配到 Switch 的两种可行运行环境，并提供一键配置脚本。

---

## ⚠️ 先说清楚：为什么没有 NSP

游戏的引擎核心 `libModernCombat3.so` 是 **32 位 ARM（armeabi-v7a）闭源二进制**，
只有这一种架构。Switch 的 Horizon 系统（大气层所在系统）内核 **只支持 64 位程序**，
没有 32 位兼容层，也没有 Android 运行时。因此**不存在能在大气层里直接安装运行的
NSP**——这与移植技巧无关，是 CPU 指令集与系统 ABI 层面的硬限制。

要在 Switch 上玩到它，现实路线只有两条，都需要在 TF 卡上装一个第二系统
（**大气层原样保留**，Hekate 启动时二选一）：

| 路线 | 位置 | 体验 | 代价 |
|---|---|---|---|
| **A. Android** | [`android/`](android/README.md) | 原生 APK + Tegra 原生 GPU，最稳 | 触摸操作用映射工具（Panda Gamepad Pro） |
| **B. Ubuntu (L4T) + 加载器** | [`l4t/`](l4t/README.md) | 手柄直连 + 触摸指针模式，原版移植体验 | 32 位软件渲染（llvmpipe），性能一般 |

两条路线共用一个数据源（玩家自带的 v1.1.7g APK + 两个 OBB），eapx 会在首次
启动时自动导入并做 SHA-256 校验。

---

## 安装（二选一）

### 路线 A：Switchroot Android

1. 按 [switchroot.org](https://wiki.switchroot.org/en/Android/Setup-Guide) 在
   TF 卡上安装 Android 双系统（Hekate 引导，大气层不变）。
2. 安装 `Modern Combat 3.apk`（v1.1.7g），把两个 OBB 放到
   `sdcard/Android/obb/com.gameloft.android.ANMP.GloftM3HM/`。
3. 用 Panda Gamepad Pro 把触摸区域映射到手柄按键。
   详见 [`android/README.md`](android/README.md)。

### 路线 B：Switchroot Ubuntu + 加载器

1. 按 [switchroot.org](https://wiki.switchroot.org/en/Linux/Ubuntu-Install) 在
   TF 卡上安装 Ubuntu 20.04+（L4T）双系统。
2. 拷贝 `loader/` 全部内容 + `l4t/` 脚本到 Switch 的 Ubuntu（如 `~/mc3/`），
   执行一次 `sudo bash setup_mc3_switch.sh`（自动配 armhf 多架构 + 依赖）。
3. 放入游戏数据（APK + OBB 丢进目录，首启自动导入），`bash run_mc3.sh` 开玩。
   详见 [`l4t/README.md`](l4t/README.md)。

---

## 操作说明（加载器版）

| 控制 | 动作 |
|---|---|
| 左摇杆 | 移动 |
| 右摇杆 | 视角 / 瞄准 |
| R1 | 开火 |
| L1 | 机瞄 |
| R2 / X | 换弹 |
| L2 / B | 手雷 |
| D-pad 上 | 冲刺 |
| D-pad 下 | 下蹲 / 掩体 |
| D-pad 左右 | 切枪 |
| Start | 暂停菜单 |
| A | 确认 |

**指针模式（按住 SELECT）**：部分提示（近战图标、暂停菜单按钮）只响应触摸，
按住 SELECT 呼出指针：右摇杆移动，A/R1 点击，方向键滑动手势
（对角线 = 快速按两个方向键）。

可调环境变量：`MC3_RPAD_SCALE`（视角速度%）、`MC3_RPAD_INVERT_Y`、
`MC3_SCALE`（fit/stretch）、`MC3_NO_AUDIO`。

---

## 游戏数据（Bring Your Own Game）

Gameloft 文件不随仓库分发。你需要自己提供：

- **APK**：Modern Combat 3 v1.1.7g（native lib 的 SHA1 必须是
  `be0d5e8779899e081a538b3930ec711d2df8aeb4`，启动时强校验）
- **main.1120.com.gameloft.android.ANMP.GloftM3HM.obb**（约 1.34GB）
- **patch.11428.com.gameloft.android.ANMP.GloftM3HM.obb**（约 81MB）

文件名无所谓，eapx 按内容识别并校验（SHA-256 + 关键区比对）。
校验参数见 [`loader/mc3.eapx.json`](loader/mc3.eapx.json)。

## 目录结构

```
├── README.md          本文件
├── LICENSE            GPL-3.0（加载器及脚本）
├── NOTICE.md          第三方代码与许可证
├── l4t/               Ubuntu 路线：启动脚本 + 一键环境配置 + 说明
│   ├── run_mc3.sh
│   ├── setup_mc3_switch.sh
│   └── README.md
├── android/           Android 路线：安装与手柄映射说明
│   └── README.md
└── loader/            PortMaster 加载器（可再分发部分，不含游戏数据）
    ├── mc3            armhf 加载器二进制
    ├── libs.armhf/    依赖闭包（libz/libzip/libbsd/libstdc++ 等）
    ├── eapx.py        数据导入校验工具
    ├── mc3.eapx.json  eapx 配方（SHA-256 / 关键区）
    ├── mc3.gptk       手柄配置（本移植中按键由加载器直连，未用 gptokeyb）
    └── licenses/      各依赖许可证
```

## 致谢与许可

- 加载器/移植：EapRules 的 [mc3-native-arm](https://github.com/EapRules/mc3-native-arm)（GPL-3.0）
- bionic ELF 加载器：gmloader-next (JohnnyonFlame) / Vita so-loader (Andy Nguyen)
- 本仓库为上述项目的 Switch 适配；游戏本体版权归 Gameloft 所有，请自备数据。

## 大气层 NSP 路线

按 shadowguardian-switch 的 AArch32 运行层路线把 mc3 加载器迁到大气层 NSP：
- `nx-port-source/`  MC3 游戏粘合层（main/mc3_api/mc3_jni/mc3_input/mc3_audio/mc3_swp/mc3_atc/mc3_patch/mc3_video/mc3_stdio/dcr_config），全部 362 个符号导入表已生成（0 缺失，99 个 GL 入口走 Mesa32）
- `nx-port-tools/`  导入表生成器/缓存、ELF32→NSO 转换脚本
- `build.sh`        工具链装配 + 构建入口（devkitARM + libnx32 + mesa32 + switch-tools，无 Docker）
- 数据要求：sd:/switch/mc3_nx/ 下 libModernCombat3.so（SHA256 99cd2fe3...）+ main.1120…obb（1342717349B）+ patch.11428…obb（81507652B），由 eapx 校验的原始数据放入即可

Title ID 0x0100F5344D433300（ExeFS NSP，AArch32）。**mc3_nx.nsp 已构建**（6.5MB，ExeFS，AArch32，与 shadowguardian-switch 同一运行时管线）。安装：把 mc3_nx.nsp 用安装器（DBI/Tinfoil/awOO）安装到大气层，游戏数据放到 sd:/switch/mc3_nx/（libModernCombat3.so + main.1120…obb + patch.11428…obb，即 eapx 校验后的原始文件）。
