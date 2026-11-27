# VentuSample — 离线程序化音频工作站

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

[![CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&label=CI)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml) [![Release workflow](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/release.yml?branch=main&label=Release)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/release.yml) [![Apache-2.0](https://img.shields.io/github/license/Ivan-VHS/VentuSample)](LICENSE)

> 基于 C++20 的确定性采样生成与离线 DSP，支持有界 WAV I/O、可复现路由和版本化采样包 manifest。

**项目记录：** 开发中 · 版本 0.2.0 · C++20 核心与 C11 DSP 兼容层 · 可选 Qt 6 Widgets 桌面程序 · 离线渲染 · Apache-2.0 · **文档审核：** 2026-11-27 UTC

<p align="center"><img src="assets/screenshots/ventusample-studio.png" alt="VentuSample 离线工作站" width="100%"></p>

## 项目用途与架构

VentuSample 用于生成可复现的采样包，并在本地处理音频文件。C++20 核心负责参数验证、带 seed 的 pattern 生成、立体声处理、离线 DSP 路由、WAV 导入/导出和 manifest 生成。C11 兼容层提供底层 DSP 与 MIDI/文件操作原语。可选的 Qt 6 Widgets 应用提供桌面界面，但它不是实时音频主机。

~~~mermaid
flowchart TB
  U["Profile / seed / parameters"] --> V["Validation and resource bounds"]
  V --> G["Pattern scheduler and renderer"]
  G --> F["Stereo float processing"]
  F --> D["Offline DSP and stochastic routing"]
  D --> M["Master / RMS and peak guard"]
  M --> W["Bounded PCM16 WAV assets"]
  V --> J["Versioned pack manifest v1/v2"]
  W --> P["Local sample pack"]
  J --> P
  Q["Optional Qt 6 desktop"] --> U
~~~

## 功能

- **程序化生成：** 基于 seed 的确定性立体声 pattern，覆盖 14 种已记录的 hip-hop 及相关风格配置，其中包括可编辑的 Billionaire Golden Era 配置。
- **Golden-era crate 工作流：** 按 0–2–1–3 顺序确定性重组四个切片，加权 kick/snare 音色选择、克制的 hats、共享 swing 和可复现的过门。
- **WAV 导入与切片：** 有界 PCM16 mono/stereo 输入（8–192 kHz、最大 128 MiB、最多 2000 万帧）、四切片放置和 ±12 半音重采样。改变音高也会改变播放速度；尚未实现保持节奏的 time-stretch。
- **离线 DSP：** 滤波、饱和、立体声联动总线处理、RMS 匹配、sample peak 保护和有界效果路由。
- **随机路由器：** 八种加权效果，可组成 serial、parallel 和 hybrid 图；支持 file、bar 和 generator-onset 触发；根据 seed/variation 确定性路由；wet ≤65%、feedback ≤72%、淡入淡出 5–20 ms，并限制 RMS 补偿和 −0.3 dBFS sample-peak。已集成到离线 WAV 导出，并作为第 19 个内置插件提供。
- **分析：** 每个文件的 peak、RMS 与 crest factor；离线 FFT peak、centroid 和 85% rolloff。不需要访问音频设备。
- **QMR-20：** 20 项质量、cohesion 与节奏控制及回归门禁；它们是确定性的工程检查，不是通用或主观的音质评分。
- **调性元数据：** 仅为有音高的素材添加 key/mode 标签；不会为无调性鼓组虚构调性。
- **采样包交换：** 使用 `ventusample.pack.v1` 和 `ventusample.pack.v2` manifest 及 WAV 资产。当前 Reaper 消费端验证并比较 v1 manifest 元数据，不审计或修改波形数据。

这些配置是原创程序化 pattern，并非录音，也不声称复刻特定艺术家。导入音频的版权与许可责任由用户承担；软件不会替用户确认版权或许可状态。

## 构建与验证

需要 CMake 3.21+、C++20 编译器，以及用于 C11 DSP 层的 C 编译器。Qt 6 Widgets 是可选依赖；无 Qt 时也可构建可移植核心和测试。

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
~~~

不依赖 Qt 的可移植核心：

~~~sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
~~~

只有检测到 Qt 6 Widgets 时才会生成桌面目标。没有 SDK 时，仍可构建可移植核心和可用测试。

## 音频与文件边界

| 接口 | 契约 | 边界 |
|---|---|---|
| Presets | `ventusample.preset.v1` | 已验证的本地设置 |
| Pack exchange | `ventusample.pack.v1 / v2` | JSON manifest 与 WAV 资产 |
| Audio output | RIFF PCM16 stereo WAV | 带 peak/RMS 元数据的有界本地渲染 |
| DSP graph | Native offline DSP | 不提供实时 VST/AU 托管 |
| Analysis | Offline FFT 与文件指标 | 本地数据；不采集音频设备输入 |

渲染仅离线执行。当前项目不提供实时播放、音频设备引擎、VST/AU 主机、保持节奏的 time-stretch 或 GPU dispatch。本文不声明固定实时延迟、RAM 占用或适用于所有采样率的性能指标。

## 测试与性能证据

CTest 覆盖确定性生成、WAV 边界、重采样、MIDI 文件、离线频谱分析、插件路由和运行时事务。只有检测到 Qt 6 Widgets 时才会注册 UI 测试。没有公开报告时，不声明覆盖率或模糊测试百分比。

测试证明的是确定性与有界行为，而不是实时播放延迟或跨硬件通用性能排名。

## 互操作与文档

与 Reaper 的素材包交换采用文件和版本化格式。当前 Reaper 消费端只读取 v1 manifest 元数据，不会修改 WAV 资产。只有在向 CMake 显式提供本地 Reaper 源码树时，才启用可选的 Reaper 原生表达式集成。直接的 Pytyliki 原生指标/DSP 适配器仍属计划功能。

- [功能矩阵](docs/FEATURES.md) · [架构](docs/ARCHITECTURE.md)
- [CRATE 制作套件](docs/CRATE_PRODUCTION_SUITE.md) · [Hip-hop 预设包](examples/hip-hop-presets/README.md)
- [Quality Monolith Rhythm](docs/QUALITY_MONOLITH_RHYTHM.md) · [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md)
- [插件与生命周期](docs/PLUGINS.md) · [MIDI 与重采样](docs/MIDI_AND_RESAMPLING.md)
- [源码集成与许可](docs/SOURCE_INTEGRATION.md) · [文档索引](docs/README.md)
- [文档控制](docs/DOCUMENTATION_CONTROL.md)

## 安全、贡献与许可证

将导入的 WAV 和 preset packs 视为不可信输入。处理前验证文件大小、RIFF chunks、帧数和声明格式。不会隐式加载外部原生库，也不会控制驱动、固件或系统音频。

修改 DSP 行为、文件格式或 plugin 契约前，请阅读 [CONTRIBUTING.md](CONTRIBUTING.md)。本项目采用 [Apache-2.0](LICENSE) 许可证；请保留第三方声明。
