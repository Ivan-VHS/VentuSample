# VentuSample — 离线程序化音频工作站

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

> 基于 C++20 的确定性采样生成与离线 DSP 工作站，支持受限 PCM16 WAV 导入/导出、版本化 pack manifest 和可选 Qt 6 桌面外壳。

**规格**

| 字段 | 当前记录 |
|---|---|
| 仓库 | [Ivan-VHS/VentuSample](https://github.com/Ivan-VHS/VentuSample) |
| 项目版本 | 0.2.0（项目契约与 CMake） |
| 项目状态 | 开发中；离线音频核心 |
| 核心 | C++20；C11 DSP 兼容层 |
| 桌面 | 可选 Qt 6 Widgets 外壳 |
| 构建 | CMake 3.21+ |
| 音频输出 | PCM16 WAV，确定性本地渲染 |
| Pack 格式 | Manifest v1 与 v2 |
| 许可证 | [Apache-2.0](LICENSE) |
| 文档审核日期 | 2026-11-26 |

<p align="center"><img src="assets/screenshots/ventusample-studio.png" alt="VentuSample 离线工作站" width="100%"></p>

## 目录
- [用途与架构](#用途与架构)
- [构建与验证](#构建与验证)
- [音频与文件契约](#音频与文件契约)
- [DSP 与插件系统](#dsp-与插件系统)
- [互操作](#互操作)
- [限制与权利](#限制与权利)
- [文档与版本](#文档与版本)

## 用途与架构

VentuSample 用于生成可复现的采样包并在本地处理音频文件。C++20 核心负责参数验证、确定性 pattern 生成、立体声浮点处理、离线 DSP 路由、WAV I/O 和 manifest 生成。Qt 6 Widgets 桌面目标是可选项，只有找到 SDK 时才会构建。

~~~mermaid
flowchart TB
  U["参数 / seed / variation"] --> V["验证与资源边界"]
  V --> G["Pattern 调度器与渲染器"]
  G --> F["Stereo float buffer"]
  F --> D["离线 DSP / stochastic router"]
  D --> M["Master 与 peak guard"]
  M --> W["PCM16 WAV assets"]
  V --> J["版本化 pack manifest v1/v2"]
  W --> P["Pack 目录"]
  J --> P
  Q["可选 Qt 6 桌面端"] --> U
~~~

渲染在工作线程中离线执行，不提供实时音频回调或音频设备访问。

## 构建与验证

需要 CMake 3.21+ 和 C++20 编译器。构建桌面程序需要安装 Qt 6 Widgets。

### 核心与可用测试

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
~~~

没有 Qt 6 时，显式构建核心和测试：

~~~sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
~~~

只有配置 Qt Widgets target 后才有 Qt UI 测试。桌面目标被跳过不代表桌面程序构建成功。

## 音频与文件契约

| 接口 | 契约 |
|---|---|
| 生成的资产 | RIFF/WAVE PCM16 |
| 输入音源 | PCM16 单声道或立体声 WAV |
| 输入采样率 | 8–192 kHz |
| 输入限制 | ≤128 MiB 且 ≤20,000,000 帧 |
| Chop | 四段切片导入与节拍放置 |
| 移调 | ±12 半音重采样；播放时长/速度会变化 |
| 输出采样率 | 22.05、44.1、48 或 96 kHz |
| Pack manifest | v1 和 v2；v2 增加可选调性与路由元数据 |
| Reaper audit | 读取 manifest 元数据，不修改 WAV 文件 |

所有导入音频都视为不可信输入，必须验证格式、chunk、帧数和文件大小。调性标签描述生成器设置，并非对导入音频自动检测的结果。不会给鼓组资产伪造调性标签。

## DSP 与插件系统

当前项目包含基于 seed 的程序化风格配置、共享 swing/timing 调度、Golden Era 乐句编排、受限立体声母带处理、八效果 stochastic 离线路由，以及由 21 个可复用 DSP kernel 构成的 Quantum Audio Engine 目录，其中包含 133 个命名模块标识。

Stochastic router 在 WAV 编码前处理已渲染的音频缓冲区。Onset trigger 使用渲染器生成的事件帧；它不会对任意导入音频执行瞬态检测。Quantum Audio Engine 是离线 DSP 目录和处理链，不是物理量子计算机，也不是实时 DAW 引擎。

参阅 [Features](docs/FEATURES.md)、[Architecture](docs/ARCHITECTURE.md)、[Plugins](docs/PLUGINS.md)、[Quality Monolith Rhythm](docs/QUALITY_MONOLITH_RHYTHM.md) 和 [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md)。

## 互操作

VentuSample 导出 manifest v1/v2 与 WAV 资产。当前 Reaper consumer 验证并比较 pack 元数据；不会分析、改写或渲染 WAV 波形。直接调用 Pytyliki 原生指标的适配器仍处于计划阶段，必须 opt-in 并通过 ABI differential tests 后才能宣称可用。

CMake 中的 Reaper bridge 默认关闭，只有显式指定本地 Reaper 源码目录才会启用。不会隐式加载外部原生库。

## 限制与权利

> VentuSample 是离线工作站，不是实时音频引擎或 DAW 插件宿主。

实时播放、音频设备采集、DAW 插件托管、保时长 time-stretch 和 GPU dispatch 尚未实现。输入移调使用重采样，因此音高变化也会改变播放时长。渲染器使用可移植 scalar 路径，不承诺通用延迟或吞吐指标。

用户有责任确认导入采样、生成输出和分发 pack 的权利。仓库不声明拥有第三方音频素材，也不声称其授权状态已获确认。不要将私人采样路径或未经授权的录音加入公共测试 fixture。

## 文档与版本

从[文档索引](docs/README.md)和[信息架构图](docs/INFORMATION_ARCHITECTURE.md)开始。

- [功能矩阵](docs/FEATURES.md)
- [架构与音频契约](docs/ARCHITECTURE.md)
- [用户指南](docs/USER_GUIDE_RU.md)
- [Manifest 与 Reaper pack audit](docs/REAPER_PACK_AUDIT.md)
- [插件 API 与生命周期](docs/PLUGINS.md)
- [MIDI 与重采样](docs/MIDI_AND_RESAMPLING.md)
- [Quality Monolith Rhythm](docs/QUALITY_MONOLITH_RHYTHM.md)
- [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md)
- [文档控制](docs/DOCUMENTATION_CONTROL.md)

项目契约和 CMake 声明版本 0.2.0，但这本身不证明存在稳定发行版。公开 pack 格式变更需要同步更新 schema、兼容性测试和迁移说明。修改 DSP 输入处理、插件或 pack 契约前，请阅读 [CONTRIBUTING.md](CONTRIBUTING.md) 和 [SECURITY.md](SECURITY.md)。项目采用 [Apache-2.0](LICENSE)，请保留第三方声明。