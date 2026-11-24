# VentuSample

VentuSample 是一款原生 C++20 工作站，用于程序化 hip-hop 采样包生成和离线 WAV 处理，并提供可选的 Qt 6 桌面界面。

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

<picture><source media="(prefers-color-scheme: dark)" srcset="assets/readme/dashboard.svg"><img src="assets/readme/dashboard-light.svg" alt="VentuSample 概览：参数驱动的 C++20 渲染、离线 DSP 和本地采样包导出" width="100%"></picture>

[![C++20 CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&style=flat-square&label=C%2B%2B20%20CI)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml) [![最近提交](https://img.shields.io/github/last-commit/Ivan-VHS/VentuSample?style=flat-square)](https://github.com/Ivan-VHS/VentuSample/commits/main) [![Apache-2.0](https://img.shields.io/github/license/Ivan-VHS/VentuSample?style=flat-square)](LICENSE)

[快速开始](#快速开始) · [截图](#截图) · [架构](#架构) · [路线图](#路线图) · [文档](docs/README.md)

> **状态：** 活跃开发。仓库包含 C++20 生成核心、C11 DSP 支持库、确定性测试和可选 Qt 6 桌面应用。CI 构建 Linux core/Qt 与 Windows desktop。
>
> **要求：** CMake 3.21+、C++20/C11 编译器和 Git。桌面界面需要 Qt 6 Widgets；没有 Qt 时可关闭桌面目标。

## Reaper 集成

VentuSample 可使用版本化的 Reaper C17 ABI 执行确定性的控制表达式和 selfhood 查询。Reaper 不进入实时 DSP 路径；生成、缓冲区、路由及离线处理仍由原生 C++20 负责。

详见 [Reaper 集成说明](docs/REAPER_RUNTIME_INTEGRATION.md)。

## 快速开始

Linux + Qt 6：

~~~sh
sudo apt-get update
sudo apt-get install -y cmake ninja-build g++ qt6-base-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
~~~

不依赖 Qt 的可移植核心：

~~~sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
~~~

Windows + Visual Studio 2022 与已配置的 Qt 6：

~~~powershell
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
~~~

## 功能与边界

| 领域 | 已实现 | 边界 |
|---|---|---|
| 采样包生成 | Full Loop、Melody、Drums、Bass、One-Shot Kit | 使用显式设置和 seed 在本地生成 |
| 音频导出 | 立体声 PCM16 WAV 与版本化 JSON manifest | 本地文件；不覆盖已有输出 |
| WAV 导入 | 支持 PCM16 mono/stereo | 验证格式、帧数、采样率和大小 |
| Quantum Audio Engine | 133 个类型化模块标识和 21 个可复用内核 | 离线 DSP，不是物理量子计算 |
| C11 DSP | 滤波器、压缩器、饱和、限幅、重采样和频谱工具 | 独立的有界 C11 库 |
| 频谱分析 | FFT peak、spectral centroid、85% rolloff | 仅处理已有 buffer；FFT 256–65,536 点 |
| MIDI 检查 | 读取 Standard MIDI File 0/1、音符、速度和拍号 | 只读解析，无 MIDI 设备播放 |
| Stochastic Router | 可选的 seed 控制变化 | 固定参数和 seed 时可复现 |
| Presets | 本地 schema v1 和 A/B slots | 不导出机器专属源路径 |
| Plugin tooling | 19 个内置工具和 metadata-only discovery | 不加载任意 DLL/SO |
| Reaper bridge | 可选 native C17 integer-expression ABI | 需要显式本地 Reaper checkout |

VentuSample **不是** DAW、实时音频引擎、音频设备主机、浏览器沙箱或托管服务。渲染和 DSP 仅处理本地 buffer 与文件。Atomic-Quantum 是经典模拟功能，不是量子硬件。

## VentuSample 的特点

- 具备 seed 感知的确定性采样生成和可检查的本地 manifest。
- C++20 产品核心与单独测试的 C11 DSP 支持库。
- 离线处理不依赖音频设备或实时调度。
- 版本化 presets 支持 A/B，同时排除机器专属源路径。
- 可选集成需显式启用；不加载任意原生插件二进制文件。

## 截图

<picture><source media="(prefers-color-scheme: dark)" srcset="assets/readme/workflow-dark.svg"><img src="assets/readme/workflow.svg" alt="VentuSample 从参数到 C++20 渲染、离线处理和 WAV/manifest 导出的数据流" width="100%"></picture>

<p align="center"><a href="assets/screenshots/ventusample-studio.png"><img src="assets/screenshots/ventusample-studio.png" alt="VentuSample Qt 采样工作站的真实 Windows 截图" width="100%"></a></p>
<sub>由原生 CI 工作流捕获的真实 1920×1080 Windows 桌面截图。上方是矢量架构图。</sub>

## 架构

~~~mermaid
flowchart LR
 A[参数和 seed] --> B[C++20 generator]
 B --> C[立体声 float buffer]
 C --> D[Offline C11 DSP]
 D --> E[PCM16 WAV 和 JSON manifest]
 F[可选 Qt 6 desktop] --> A
 G[可选 Reaper C17 bridge] --> B
~~~

| 路径 | 职责 |
|---|---|
| src/ | C++20 生成器、DSP、presets 与 Qt desktop |
| src/compat/pytyliki-dsp/ | C11 DSP 支持库 |
| tests/ | generator、DSP、preset 和契约测试 |
| schemas/ | 版本化 JSON schemas |
| docs/ | 音频、架构、UI、质量与集成契约 |
| assets/screenshots/ | Qt 桌面截图 |

## Presets 和 A/B

Presets 使用本地版本化 JSON 格式，保存声音配置和生成/DSP 参数。默认不保存随机 seed，除非用户明确选择。两个会话内 A/B slots 可切换参数快照而不写入文件。源采样路径不会导出，因为它们与机器绑定。

Schema：[Preset v1](schemas/ventusample.preset.v1.schema.json)。没有云同步，也不声称拥有导入或生成音频的版权。

## 兼容性

| 目标 | 要求 | CI / 范围 |
|---|---|---|
| Linux core 与 desktop | CMake 3.21+、C++20/C11、UI 需要 Qt 6 Widgets | Ubuntu 24.04、Qt 6 |
| Windows desktop | Visual Studio 2022、CMake 3.21+、Qt 6 Widgets | Windows Server 2022、Qt 6.8.3 |
| Portable core | CMake 3.21+、C++20/C11 | VENTU_BUILD_DESKTOP=OFF |
| Python tooling | Python 3.10–3.12 | 仅工程检查 |
| Reaper bridge | 可选本地 Reaper source tree | 显式设置 REAPER_SOURCE_DIR |

如果没有 Qt 6 Widgets，请设置 VENTU_BUILD_DESKTOP=OFF，以构建可移植核心和测试。

## README Revival 更新

- 首屏说明项目用途、成熟度、工具链和可复制的构建/测试路径。
- 真实 Windows 截图与架构图分开。
- 本地浅色/深色 SVG 尺寸一致。
- 明确说明离线范围、Qt 依赖、preset 行为和插件限制。

| 旧版流程 | 更新后 |
|---|---|
| 先读概览再找构建命令 | 构建和测试命令前置 |
| 只有单一主题图像 | 自适应本地 SVG |
| 较晚才说明可选依赖 | 提前说明 Qt 与 Reaper bridge |
| 泛化功能描述 | 实现状态和边界表 |

## 路线图

- [x] C++20 生成核心和 C11 DSP 支持库。
- [x] 离线 WAV 导出、JSON manifest 和源 WAV 验证。
- [x] Qt 6 桌面工作流和确定性原生测试目标。
- [x] 版本化本地 presets 和 A/B slots。
- [ ] 通过确定性 fixture 和数值回归测试扩展渲染/DSP。
- [ ] 仅在配套版本化 schema、有界解析器和兼容测试后增加格式。

## 生态

- [DuckOS](https://github.com/Ivan-VHS/DuckOS) — 操作系统项目和 Mora 工作区。
- [Reaper](https://github.com/Ivan-VHS/Reaper) — 系统语言和可选原生表达式桥接。
- [Pytyliki](https://github.com/Ivan-VHS/Pytyliki) — 处理器检查和系统工具。
- [OPlague](https://github.com/Ivan-VHS/OPlague) — 设备与驱动证据工作台。

参阅[文档](docs/README.md)、[架构](docs/ARCHITECTURE.md)、[视觉发布标准](docs/README_VISUAL_STANDARD.md)、[贡献指南](CONTRIBUTING.md)和 [Issues](https://github.com/Ivan-VHS/VentuSample/issues)。许可证：Apache-2.0。
