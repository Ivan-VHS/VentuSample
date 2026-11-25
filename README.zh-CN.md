<p align="center">[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)</p>

<h1 align="center"><strong>🎛️ VentuSample — 程序化音频工作站</strong></h1>

[![CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&label=CI)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml) [![Release](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/release.yml?branch=main&label=Release)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/release.yml) [![Apache-2.0](https://img.shields.io/github/license/Ivan-VHS/VentuSample)](LICENSE)

> **确定性采样生成与离线 DSP。不虚称实时音频引擎。**

**项目状态:** 开发中 · 离线音频核心已实现 · Qt 桌面端部分完成 · **版本:** 0.2.0 · **License:** Apache-2.0 · **README 标准:** Psychoporter 666 v1.0 · **审阅日期:** 2026-11-25 UTC · **Source:** [Ivan-VHS/VentuSample](https://github.com/Ivan-VHS/VentuSample)

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

The generator sends seeded settings through the offline DSP core into bounded WAV assets and a versioned pack manifest. The optional Qt interface configures inputs; it does not turn the engine into a real-time audio host.

## 环境要求

需要 CMake 3.21+、C++20 编译器和用于 C11 DSP 层的 C 编译器。桌面应用可选 Qt 6 Widgets；可移植核心及其测试可在无 Qt 环境下构建。Linux 推荐使用 Ninja。

## 构建

~~~sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
~~~

### Portable core without Qt

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

Imported assets must be treated as untrusted data. Validate file sizes, chunk boundaries and declared formats before consuming them.

## 质量验证

~~~sh
ctest --test-dir build --output-on-failure
~~~

The test targets exercise deterministic generation, plugin routing, WAV boundaries, resampling, MIDI files, offline spectrum analysis and runtime transactions. Desktop UI tests are registered only when Qt Widgets is available. Coverage and fuzzing percentages are not claimed without published reports.

## 性能证据

生成器和 DSP 测试验证确定性与有界处理，而非实时播放延迟。此处不声明固定微秒延迟、RAM 占用或采样率兼容矩阵。

## 兼容性

CMake 提供可移植核心和可选 Qt 6 Widgets 桌面目标。文档记录了 MSVC 与 GCC/Clang 构建路径；桌面目标是否可用取决于兼容的 Qt 6 SDK。

## 文档与生态

- [Features](docs/FEATURES.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md)
- [MIDI and resampling](docs/MIDI_AND_RESAMPLING.md)
- [Plugins](docs/PLUGINS.md)
- [Source integration and licensing](docs/SOURCE_INTEGRATION.md)
- [Documentation index](docs/README.md)

音频引擎、MIDI/重采样、插件和源码许可文档定义了格式边界与扩展点。

Reaper pack exchange is file-based and versioned. Optional Reaper native expression integration is enabled only when a local Reaper source tree is explicitly supplied to CMake; Pytyliki native metrics remain a planned opt-in path.

## 参与贡献

修改代码或格式契约前请阅读 CONTRIBUTING.md。保持改动可审查，添加确定性测试，并按贡献指南使用 Conventional Commits。

## 许可证与法律说明

本项目依据 Apache-2.0 许可，软件按许可条款以“按现状”方式提供。第三方名称和商标归其各自权利人所有。

Hosted-service terms, cookies, CORS and OAuth are not applicable to the documented local runtime.

---

<p align="center"><sub>Psychoporter 666 README Standard v1.0 · Ivan-VHS · 2026-11-25 UTC</sub></p>