# 🎛️ VentuSample — 程序化音频工作站

<p align="center">[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)</p>

> **Psychoporter 666** — 架构的精确 × 探索的能量 × 可追溯的证据。[共享文档规范](https://github.com/Ivan-VHS/DAK/blob/main/docs/PSYCHOPORTER_DOCUMENTATION_STYLE_V1.md)。  
> **编辑修订 · 2026-11-28：** 更新了文档呈现与导航；保留原有作者身份、许可证条款及第三方归属信息。


[![CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&label=CI)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml) [![Release](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/release.yml?branch=main&label=Release)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/release.yml) [![Apache-2.0](https://img.shields.io/github/license/Ivan-VHS/VentuSample)](LICENSE)

> **确定性采样生成与离线 DSP。不虚称实时音频引擎。**

**项目状态:** 开发中 · 离线音频核心已实现 · Qt 桌面端部分完成 · **版本:** 0.2.0 · **License:** Apache-2.0 · **README 标准:** Psychoporter 666 v1.1 · **审阅日期:** 2026-11-25 UTC · **Source:** [Ivan-VHS/VentuSample](https://github.com/Ivan-VHS/VentuSample)

<p align="center"><img src="assets/readme/dashboard.svg" alt="VentuSample project overview" width="100%"></p>
<p align="center"><a href="assets/screenshots/ventusample-studio.png"><img src="assets/screenshots/ventusample-studio.png" alt="VentuSample desktop application screenshot" width="100%"></a></p>

## 快速导航

[🚀 Demo: Studio screenshot](assets/screenshots/ventusample-studio.png) · [Documentation](docs/README.md) · [Source](https://github.com/Ivan-VHS/VentuSample) · [Issues](https://github.com/Ivan-VHS/VentuSample/issues)

## 目标与范围

原生程序化采样工作站，采用 C++20 核心、C11 DSP 兼容层和可选 Qt 6 Widgets 桌面应用。渲染为离线处理。它不是实时 DAW、VST/AU 主机、云服务或音频驱动。

## 功能

- 基于固定随机种子的程序化立体声生成，覆盖 14 种已记录的嘻哈及相邻风格配置。
- 黄金时代采样素材重组、加权鼓组音色选择和确定性变化。
- 有界 PCM16 WAV 导入/导出、四切片编排和半音重采样。
- 离线 DSP 路由图，限制 wet/feedback，支持 RMS 匹配和采样峰值保护。
- 使用版本化 ventusample.pack.v1 和 v2 清单进行显式采样包交换。
- 可选 Qt 6 Widgets 工作站；桌面构建受 Qt SDK 可用性单独控制。

## 架构与数据流

~~~mermaid
flowchart LR
  P["Seed / profile / parameters"] --> G["Deterministic generator"]
  G --> D["Offline C11 DSP layer"]
  D --> X["C++20 audio core"]
  X --> W["PCM16 WAV assets"]
  X --> M["Versioned pack manifest"]
  Q["Optional Qt 6 UI"] --> P
~~~

生成器将带有固定 seed 的参数传入离线 DSP 核心，输出受限的 WAV 文件和版本化素材包清单。可选 Qt 界面用于配置输入，不会使引擎变成实时音频主机。

## 环境要求

需要 CMake 3.21+、C++20 编译器和用于 C11 DSP 层的 C 编译器。桌面应用可选 Qt 6 Widgets；可移植核心及其测试可在无 Qt 环境下构建。Linux 推荐使用 Ninja。

## 构建

~~~sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
~~~

### 不依赖 Qt 的可移植核心

~~~sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
~~~

## 运行

只有检测到 Qt 6 Widgets 时才会生成桌面目标；否则请使用 portable core 构建。

## 接口

| Surface | Contract | Boundary |
|---|---|---|
| Presets | `ventusample.preset.v1` | Schema-validated settings |
| Pack exchange | `ventusample.pack.v1 / v2` | JSON manifest plus WAV assets |
| Audio output | `RIFF PCM16 stereo WAV` | Bounded local file output with peak/RMS metadata |
| Plugin layer | Native offline DSP graph | No real-time VST/AU hosting |

## 安全与信任边界

默认离线渲染；音频管线不包含后台录音或网络遥测。导入的 WAV 被视为不可信输入，并受格式、数据块、帧数和文件大小检查限制。不会隐式加载外部原生库。不执行驱动、固件或系统音频控制。

导入的素材必须视为不可信数据。处理前请验证文件大小、数据块边界和声明的格式。

## 质量验证

~~~sh
ctest --test-dir build --output-on-failure
~~~

测试目标覆盖确定性生成、插件路由、WAV 边界、重采样、MIDI 文件、离线频谱分析和运行时事务。只有检测到 Qt Widgets 时才会注册桌面界面测试。没有公开报告时，不声明覆盖率或模糊测试百分比。

## 性能证据

生成器和 DSP 测试验证确定性与有界处理，而非实时播放延迟。此处不声明固定微秒延迟、RAM 占用或采样率兼容矩阵。

## 兼容性

CMake 提供可移植核心和可选 Qt 6 Widgets 桌面目标。文档记录了 MSVC 与 GCC/Clang 构建路径；桌面目标是否可用取决于兼容的 Qt 6 SDK。

## 文档与生态

- [Features](docs/FEATURES.md)
- [CRATE Hip-Hop Production Suite](docs/CRATE_PRODUCTION_SUITE.md)
- [Hip-Hop Preset Crate](examples/hip-hop-presets/README.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md)
- [MIDI and resampling](docs/MIDI_AND_RESAMPLING.md)
- [Plugins](docs/PLUGINS.md)
- [Source integration and licensing](docs/SOURCE_INTEGRATION.md)
- [Documentation index](docs/README.md)

音频引擎、MIDI/重采样、插件和源码许可文档定义了格式边界与扩展点。

与 Reaper 的素材包交换采用文件和版本化格式。只有在向 CMake 显式提供本地 Reaper 源码树时，才启用可选的 Reaper 原生表达式集成；Pytyliki 原生指标仍属于计划中的可选功能。

## 参与贡献

修改代码或格式契约前请阅读 [CONTRIBUTING.md](CONTRIBUTING.md)。保持改动可审查，添加确定性测试，并按贡献指南使用 Conventional Commits。

## 许可证与法律说明

本项目依据 Apache-2.0 许可，软件按许可条款以“按现状”方式提供。第三方名称和商标归其各自权利人所有。

云服务条款、Cookie、CORS 和 OAuth 不适用于本文档所述的本地运行环境。

---

<p align="center"><sub>Psychoporter 666 README Standard v1.0 · Ivan-VHS · 2026-11-25 UTC</sub></p>

## 相关数学研究

[维尤年科 K3 理论 v0.4.0 提案](https://github.com/Ivan-VHS/Kaskad/pull/36)记录了基数 1000 完备化的 CRT 分解及唯一的无限位置系数序列。这只是外部数学参考；不会改变 VentuSample 的音频表示、PCM 格式、DSP 算法、离线处理模型、资源包清单或插件契约。详见[本地研究说明](docs/VYUNENKO_K3_RESEARCH_REFERENCE.md)。
