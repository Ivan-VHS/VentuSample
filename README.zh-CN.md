# VentuSample — 程序化采样工作站

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

[![CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&label=CI)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml) [![Apache-2.0](https://img.shields.io/github/license/Ivan-VHS/VentuSample)](LICENSE)

<p align="center"><img src="assets/readme/dashboard.svg" alt="VentuSample 项目概览" width="100%"></p>
<p align="center"><a href="assets/screenshots/ventusample-studio.png"><img src="assets/screenshots/ventusample-studio.png" alt="VentuSample 桌面应用" width="100%"></a></p>

VentuSample 是一款原生程序化采样工作站，采用 C++20/Qt 6 桌面应用和离线音频处理核心。

**范围：**本地声音生成、参数调整、离线 DSP 和文件导出。它不是实时 DAW、VST/AU 主机、云服务或音频驱动程序。

## 功能

- 程序化采样生成、预设以及会话内 A/B 参数槽。
- 离线 DSP、WAV 导入/导出、MIDI 和采样率转换工具。
- 插件 SDK 与版本化 pack 清单；发现和交换遵循明确的元数据及信任边界。
- 可复现的生成设置和有界文件处理。

详细资料：[功能](docs/FEATURES.md) · [架构](docs/ARCHITECTURE.md) · [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md) · [MIDI 与重采样](docs/MIDI_AND_RESAMPLING.md) · [插件](docs/PLUGINS.md) · [素材来源规则](docs/SOURCE_INTEGRATION.md)。

## 构建与测试

桌面应用需要 CMake、C++20 编译器和 Qt 6 Widgets。可移植核心可在不安装 Qt 的情况下构建。

Linux：

```sh
sudo apt-get update
sudo apt-get install -y cmake ninja-build g++ qt6-base-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

不使用 Qt 的可移植核心：

```sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
```

Windows 使用 MSVC 和 Qt 6：

```sh
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

## 规格与集成

- 预设格式：[ventusample.preset.v1 Schema](schemas/ventusample.preset.v1.schema.json)。
- [文档索引](docs/README.md) · [Reaper pack 审计](docs/REAPER_PACK_AUDIT.md) · [发布流程](docs/RELEASE.md)。
- [生态共享标准](https://github.com/Ivan-VHS/DuckOS/blob/main/docs/SHARED_STANDARDS.md)。

[贡献指南](CONTRIBUTING.md) · [安全策略](SECURITY.md) · [Apache-2.0 许可证](LICENSE)
