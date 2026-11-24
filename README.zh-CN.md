# VentuSample — 程序化采样工作站

## 版本化预设与 A/B 对比

桌面应用支持保存和加载本地 JSON 预设，格式为 `ventusample.preset.v1`。预设保留音乐配置、节奏、滤波器、采样器和 DSP 路由设置。默认不包含随机 seed，因此预设描述的是声音配置，而不是锁定每一次生成变化；保存对话框提供显式包含 seed 的选项。

使用 **Presets → Save preset** 或 **Presets → Load preset**。两个额外的 A、B 会话槽可在不写入磁盘的情况下捕获并切换参数集。预设在接受前会按照与生成器相同的规则进行验证。由于本地源采样路径与具体机器绑定，因此不会导出。

Schema 发布于 `schemas/ventusample.preset.v1.schema.json`。预设是本地文件，不提供云同步，也不主张拥有其所描述音频的著作权。

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

<p align="center"><img src="assets/readme/dashboard.svg" alt="VentuSample 项目概览" width="100%"></p>

[源代码仓库](https://github.com/Ivan-VHS/VentuSample) · [CI 工作流](.github/workflows/ci.yml) · [文档索引](docs/README.md) · [Apache-2.0](LICENSE)

## 桌面应用实际截屏

<p align="center"><a href="assets/screenshots/ventusample-studio.png"><img src="assets/screenshots/ventusample-studio.png" alt="VentuSample 在 Windows 上运行的实际截屏" width="100%"></a></p>
<p align="center"><sub>1920×1080 · 点击图片可打开原始 PNG 文件。</sub></p>

这是 VentuSample 在 Windows CI 桌面上运行时的真实截图，来自构建后的 Qt 应用。横幅和数据流图是另外制作的原创 SVG 矢量插图。

### 架构一览

<p align="center"><img src="assets/readme/workflow.svg" alt="VentuSample implemented data flow" width="100%"></p>

实际流程：参数与 seed → C++20 渲染器 → 立体声缓冲区 → 可选离线 DSP → PCM16 WAV 与 JSON manifest。


[![C++20 CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&label=C%2B%2B20%20CI)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml)  [![Qt desktop jobs](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&label=Qt%20desktop%20jobs)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml)  [![License](https://img.shields.io/github/license/Ivan-VHS/VentuSample)](LICENSE)


> **项目范围：** VentuSample 是一款基于 C++20 的原生工作站，用于确定性程序化生成采样包并离线处理 WAV。安装 Qt 时可使用 Qt 6 桌面界面，也可以单独构建可移植的 DSP/核心库。它不是托管 Web Demo、浏览器 Sandbox、DAW、音频设备主机或生产音频服务。文档已于 **2026-11-22** 对照 `main` 分支核对。

## 项目用途与边界

VentuSample 根据明确的参数和 seed 在本地创建嘻哈采样包。它可以渲染音频、通过离线 DSP 处理受支持的 WAV 文件，并生成描述设置和输出的版本化 JSON manifest。无需 DAW、音频接口、网络连接或托管后端。

目标用户包括 Beatmaker、声音设计师、音频开发者、QA 工程师和需要确定性、可检查采样生成及可重复离线处理的审查者。

### 已实现的功能

| 模块 | 已实现行为 | 明确边界 |
|---|---|---|
| 程序化生成器 | Full Loop、Melody、Drums、Bass 和 One-Shot Kit 输出 | 本地渲染；相同 seed/设置会生成相同结果 |
| 音频导出 | 立体声 PCM16 WAV 和版本化 JSON manifest | 本地文件；不会覆盖已有输出文件 |
| WAV 导入 | 受支持的 PCM16 mono/stereo 源文件 | 验证大小、帧数、采样率和格式 |
| Quantum Audio Engine | 133 个类型化模块 ID 和 21 个可复用 DSP 内核 | 创意离线 DSP，不是物理量子计算 |
| Migrated C11 processing | 有界 WAV I/O、滤波器、压缩器、饱和处理、限幅器和测量 | 独立 C11 构建目标，离线缓冲区 |
| Offline spectrum analysis | 对现有缓冲区计算 FFT 峰值、谱质心和 85% rolloff | 有界 256–65,536 点 FFT；不访问音频设备 |
| 采样率转换 | 对单声道/立体声缓冲区进行有界线性插值 | 预览级质量；不包含抗混叠滤波 |
| MIDI 文件检查 | 解析 Standard MIDI File 格式 0/1、音符、速度和拍号 | 有界只读解析；不访问 MIDI 设备或播放 |
| Stochastic Router | 可选的 seeded 路由，使用八个内置处理器 | 离线缓冲区处理；默认关闭 |
| Plugin Manager | 19 个内置工具和经过审查的 C++20 handler API | manifest 发现仅读取元数据；不任意加载 DLL/SO |
| Atomic-Quantum 133 | 独立的有界经典模拟子集 | 不访问量子硬件 |
| Reaper Bridge | 构建时可选的 C17 integer-expression ABI | 显式本地源码路径；不进入音频 DSP 路径 |

生成器包含 14 种配置：Boom Bap、Billionaire Golden Era、East Coast、Harlem Dip-Trap、G-Unit、Modern Trap、Atlanta Trap、UK Drill、UK Grime、Memphis、Jazz Rap、G-Funk、Boom Trap 和 Cripple Trill。

该应用不是实时音频引擎。渲染和 DSP 在工作线程中离线执行；没有音频设备回调、播放热切换、VST/AU 主机、任意第三方插件加载器或 DAW 自动化。

下方 Windows 图片是桌面应用截屏，不是浏览器预览。


## Reaper 集成

## 插件执行策略

原生插件主机现在会验证生成参数、应用输出包大小预算，并记录执行关联 ID 和运行状态指标。请参阅[插件执行策略](docs/PLUGIN_EXECUTION_POLICY.md)。Soft deadline 在处理器返回后观测，不会强制终止执行。

VentuSample 可使用版本化的 Reaper C17 ABI 执行确定性的控制表达式和 selfhood 查询。Reaper 不进入实时 DSP 路径；生成、缓冲区、路由及离线处理仍由原生 C++20 负责。

详见 [Reaper 集成说明](docs/REAPER_RUNTIME_INTEGRATION.md)。

## 快速开始

### 环境要求

- CMake **3.21 或更高版本**。
- C++20 编译器：GCC、Clang 或 MSVC。
- Git。
- 桌面应用需要 Qt **6 Widgets**。没有 Qt 时，仍可构建 DSP/核心库和非 Qt 测试。
- Ninja 是可选项，但 Linux CI 桌面任务使用它。

渲染音频不需要 Python 或 Node.js。CI 使用 Python 3.10–3.12 运行项目合同与工程验证脚本。

### Linux：构建桌面工作站

在 Ubuntu 24.04 上安装 CI 使用的工具链软件包：

```sh
sudo apt-get update
sudo apt-get install -y cmake ninja-build g++ qt6-base-dev
```

配置、构建并运行测试：

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

桌面可执行文件由配置好的 CMake 构建生成。Linux Ninja 构建可通过 `./build/VentuSample` 启动；Windows Release 构建可通过 `build\\Release\\VentuSample.exe` 启动。

### 不使用 Qt 构建可移植核心

没有安装 Qt 6 Widgets，或只需要 DSP 引擎和测试时：

```sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
```

若 `VENTU_BUILD_DESKTOP=ON` 但找不到 Qt 6 Widgets，CMake 会发出警告并跳过桌面目标；这不代表已经生成 GUI 可执行文件。

### Windows：使用 MSVC 和 Qt 6 构建

使用 Visual Studio 2022 x64 开发者环境并安装 Qt 6 Widgets（当前 CI 使用 Qt **6.8.3**、MSVC 2022 x64）：

```sh
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

Windows CI 会先安装 Qt 并配置对应的 MSVC 工具链，再执行这些命令。

### 首次冒烟检查

成功运行 `ctest` 是通用构建检查。测试涵盖参数验证、确定性重复渲染、变化版本、WAV round-trip、stochastic DSP、Schema、插件生命周期和 Quantum Audio Engine 行为。

项目没有默认 HTTP 端口、localhost URL、浏览器启动命令、数据库 seed 或远程 Mock Service。所有生成与处理均在本地执行。

## 可复现数据与测试场景

VentuSample 不使用远程数据库或实时 API。生成器使用本地参数和 seed；导入音频来自操作者选择的文件。Manifest 与输出文件一起在本地生成。

可在桌面应用中验证以下场景：

1. **可重复的 Loop：**选择生成器配置、BPM、Key/Scale 和固定 seed；使用相同设置渲染两次 Full Loop，并比较音频和 manifest。
2. **独立分轨：**分别渲染 Melody、Drums 和 Bass，并检查每种输出类型。
3. **One-Shot Kit：**生成八个采样，并确认只有有音高的声音才会添加音高标签。
4. **WAV 导入：**加载受支持的 PCM16 mono/stereo 文件并检查其属性。
5. **切片与音高：**选择最多四个切片并应用文档所述的半音重采样；它会同时改变音高和播放速度，不是 time-stretch。
6. **离线 DSP 链：**在 Quantum Audio Engine 中加载受支持的 WAV，排列处理链，调整模块控制并导出处理后的 WAV。
7. **随机路由：**启用可选 Router，选择拓扑/触发/强度后重新渲染。Seed、variation、trigger 和 effect mask 决定路由。
8. **插件发现：**检查本地 `plugin.json`；发现的元数据条目在显式附加经过审查的原生处理器前保持禁用。

### 生成器范围与文件限制

| 控制项 / 输入 | 支持范围 |
|---|---|
| BPM / bars / variations | 60–190 / 1–16 / 1–32 |
| Key / scale | C–B / 6 个音阶系列 |
| Swing | 50–68% |
| Density / grit / saturation / stereo | 0–100% |
| High-pass / low-pass | 20–300 Hz / 2–20 kHz |
| Snare offset | −30…+30 ms |
| Bus compression | 1:1–12:1 |
| 采样率 | 22.05 / 44.1 / 48 / 96 kHz |
| 导入源文件 | PCM16 mono/stereo，8–192 kHz |
| 源文件限制 | 128 MiB 和 20,000,000 帧 |
| Chop pitch / mix | −12…+12 半音 / 0–100% |

Low-pass 截止频率必须高于 high-pass 截止频率，并低于采样率的 0.49 倍。导入音频不会自动进行调性检测。

### Stochastic Router 合同

Router 为可选功能，**默认关闭**。它在 PCM16 WAV 编码前处理已渲染的立体声缓冲区。内置池包含 low-pass、high-pass、saturator、bitcrusher、delay、reverb、ring modulation 和 stereo width。

| 参数 | 合同 |
|---|---|
| Topology | hybrid、serial 或 parallel；parallel 使用 2–4 个有界分支 |
| Trigger | 每个文件、每个小节或渲染器安排的事件帧 |
| Intensity | 0–100%；零强度为透明路径 |
| Wet / feedback | 最大 65% / 72% |
| Segment crossfade | 5–20 ms |
| RMS compensation | 有界 0.5–2× |
| Output guard | sample peak ceiling 不高于 −0.3 dBFS |
| Repeatability | generator seed + variation + trigger index |

Event Mode 使用程序化渲染器提供的预定帧。它不会对任意导入的 WAV 文件执行瞬态检测。这是离线信号处理，不是实时回调或插件热切换。

### 状态生命周期与存储

工作音频缓冲区和当前 UI 状态存在于应用进程中。输出 WAV 和 JSON manifest 写入用户选择的本地目录；不会覆盖已有文件。关闭应用会结束内存会话，但不会删除已导出的文件。

桌面 UI 通过原生偏好设置层保存自身设置。没有浏览器 localStorage、sessionStorage、IndexedDB、Cookie 状态、共享服务器数据库或自动云同步。由于音频处理完全离线，没有内置延迟/HTTP 500 模拟器或外部服务回退机制。

## 配置、环境与信任边界

仓库根目录没有 `.env.example`、Docker Compose Stack、账号系统、演示凭据、API Token、CORS 配置、反向代理或本地 HTTPS 设置。音频生成和处理不需要网络。

`CMakeLists.txt` 中声明的构建选项：

| CMake 选项 | 默认值 | 用途 |
|---|---:|---|
| `VENTU_BUILD_DESKTOP` | `ON` | 查找 Qt 6 Widgets，并在可用时构建桌面目标 |
| `VENTU_BUILD_TESTS` | `ON` | 构建原生测试 |
| `VENTU_ENABLE_SANITIZERS` | `OFF` | 在受支持的 GCC/Clang 构建中启用 AddressSanitizer/UndefinedBehaviorSanitizer |
| `REAPER_SOURCE_DIR` | 空 | 显式指定本地 Reaper 源码目录，启用可选 C17 expression bridge |

除非显式提供本地源码目录，否则 Reaper Bridge 默认关闭。它使用 Reaper 原生表达式 ABI 进行确定性整数控制表达式计算；不会替代 C++20 引擎，也不会进入实时 DSP 路径。

### 插件安全边界

Plugin Manager 内置 19 个工具。可以递归发现本地 `plugin.json` 元数据，但发现的条目在显式附加经过审查的原生处理器之前会保持禁用。管理器不会加载 manifest entrypoint、任意 DLL/SO 或可执行载荷。公开的 manifest Schema：[ivan-vhs.plugin.v1](schemas/ivan-vhs.plugin.v1.schema.json)。

不要将 manifest 发现理解为代码执行或 Sandbox 保证。注册 handler 是由应用审查的显式原生 C++ 操作。

## 架构与仓库目录

数据流程：**参数 + seed → 程序化渲染器 → 立体声浮点缓冲区 → 可选离线路由/DSP → PCM16 WAV + 版本化 JSON manifest**。

| 路径 | 内容 |
|---|---|
| `src/` | C++20 生成器、渲染器、音频缓冲区、DSP 内核、插件管理器、Qt 桌面 UI 和可选 Reaper Bridge |
| `src/compat/pytyliki-dsp/` | 迁入的 C11 缓冲区处理库 |
| `integrations/cockos-reaper/` | 可选的 Cockos REAPER Lua 辅助操作 |
| `tests/` | 原生 DSP、生成器、manifest、UI、插件生命周期和回归测试 |
| `docs/` | 架构、功能、插件 SDK、UI 行为和音频合同 |
| `docs/PYTYLIKI_DSP_MIGRATION.md` | 迁入的 C11 模块所有权与构建约定 |
| `docs/integrations/COCKOS_REAPER.md` | 可选 Cockos REAPER 辅助脚本 |
| `schemas/` | 版本化采样包与插件 manifest Schema |
| `tools/` | project-contract、fixture、visual-canon 和工程验证脚本 |
| `federation/` | 显式互操作与适配器 manifest |
| `assets/screenshots/` | 桌面应用截图 |
| `.github/workflows/` | Python 合同、原生、Linux Qt 与 Windows Qt CI 任务 |
| `CMakeLists.txt` | C++20 构建图与可选功能开关 |
| `project-contract.json` | 机器可读的项目合同 |

桌面界面使用 Qt 6 Widgets。生成器和 DSP 引擎使用 C++20。CI 中的 Python 用来验证仓库合同，并非 Python Web Backend。项目没有 frontend/backend 部署、Web Server、Mock Server、Browser Bundle、端口绑定、CORS 或公开在线预览。浏览器支持和响应式 viewport 规则不适用。

## CI、构建产物与运维

[CI 工作流](.github/workflows/ci.yml) 包含：

- Python 3.10、3.11 和 3.12 的合同检查；
- 不启用桌面目标的原生 C++20 核心构建和 CTest；
- Ubuntu 24.04 上带 Qt 6 Widgets 的 Linux DSP + Qt 桌面构建；
- Windows 2022 Runner 上使用 Qt 6.8.3 和 MSVC 2022 的 Windows 11 原生 Qt 桌面构建；
- project-contract、generated-fixture、engineering-evolution 和 CLife 验证。

Windows 桌面任务会截取正在运行的应用并上传保留 30 天的截图 Artifact。该工作流**没有**声明公开 Web 部署、托管预览或带标签的 Windows Release 压缩包。CI 成功并不代表存在 Release 打包或自动部署。

回滚本地修改时，可返回上一个 Git Commit 或重新构建最后一个已知正常版本。没有在线服务或已部署数据库需要回滚。CI 日志和 Artifact 可在 GitHub Actions 中查看。

## 故障排查

| 现象 | 检查方法 |
|---|---|
| 缺少 Qt 桌面目标 | 安装 Qt 6 Widgets，或使用 `-DVENTU_BUILD_DESKTOP=OFF` 配置仅核心构建 |
| CMake 版本被拒绝 | 安装 CMake 3.21 或更高版本 |
| C++ 编译失败 | 确认 C++20 编译器，并检查第一条编译诊断 |
| WAV 导入被拒绝 | 检查 PCM16、mono/stereo、8–192 kHz、128 MiB 和 20M 帧限制 |
| 导出结果不同 | 确认 generator seed、variation 和 settings；要重复 stochastic route 必须使用相同选项 |
| Router 意外改变导入音频 | onset routing 使用 renderer-scheduled events；不会对任意导入 WAV 做 transient detection |
| 能看到插件 manifest 但无法运行 | discovery 只读取 metadata；需要显式附加经过审查的原生处理器 |
| Reaper Bridge 不可用 | 配置时将 `REAPER_SOURCE_DIR` 指向现有且可信的 Reaper 源码仓库 |

出现测试或构建问题时，请保留准确的 CMake 命令、编译器/Qt 版本、失败测试名称和完整 GitHub Actions 日志。

## 已知限制、贡献与许可证

VentuSample 是离线采样工作站，不是 DAW 或实时音频主机。采样生成器输出 PCM16 立体声 WAV 和 manifest；它不会调用外部 API、向浏览器流式传输音频或管理云端项目。Stochastic Router 使用八个内置处理器，不托管第三方插件。Quantum Audio Engine 是创意 DSP 目录；Atomic-Quantum 是独立的经典模拟层，不是物理量子计算。导入或导出音频必须拥有相应源素材使用权；哈希只能证明字节一致，不能证明所有权。

仓库的贡献流程、变更验证和 Pull Request 要求见 [CONTRIBUTING.md](CONTRIBUTING.md)。请通过 [GitHub Issues](https://github.com/Ivan-VHS/VentuSample/issues) 或 Pull Request 提出改进。请附上操作系统、编译器/CMake/Qt 版本、seed 和准确设置、安全的输入 WAV 描述，以及准确命令/结果。提交前运行 CTest 和 project-contract 检查。

许可证：[Apache-2.0](LICENSE)。

## README 与 66 条规则的映射

Web Stand 检查清单根据实际离线桌面/音频产品进行映射；纯 Web 部分明确标记为不适用：

| 规则范围 | README 覆盖内容 |
|---|---|
| 1–11 · 用途与边界 | 说明确定性采样工作站的目的、用户、工作流程、离线范围和功能边界 |
| 12–22 · 快速开始 | 给出 CMake/C++20/Qt 要求、Linux/Windows/仅核心命令、构建与 CTest |
| 23–33 · 配置与访问 | 列出真实 CMake 选项和可选 Reaper Bridge；不虚构 ENV、账号、秘密、CORS 或 HTTPS |
| 34–44 · 数据与行为 | 说明 seed 重复性、本地 WAV 输入/输出、manifest、文件限制、插件发现和 UI 状态生命周期 |
| 45–55 · 架构与构建 | 列出真实 C++20/Qt 架构、DSP 阶段、仓库目录、Schema 和 CI 矩阵 |
| 56–66 · 运维与支持 | 链接真实 CI 工作流，说明截图 Artifact 限制与回滚、故障排查、已知问题、贡献方式和许可证 |

**VentuSample 没有公开的 Web Stand URL。** 桌面截图和 CI 截图 Artifact 并不是交互式浏览器 Demo。

## 统一报告封装

可以将 JSON 结果包装为通用的 ivan-vhs.report.v1 元数据格式：

    python tools/report_envelope.py --tool ventusample --version VERSION < render-manifest.json

请参阅 docs/REPORT_ENVELOPE.md 和 schemas/ivan-vhs.report.v1.schema.json。工具自身的结果内容保持不变。


## 文档导航图

请从[文档索引](docs/README.md)进入，并使用[信息架构图](docs/INFORMATION_ARCHITECTURE.md)浏览音频/DSP、Studio 工作流、预设与插件、架构、UX 和集成主题。主题入口链接到详细契约；支持格式和实现状态仍以源代码、测试、Manifest 与 Schema 为准。


## Mora 桌面集成

构建 Qt 6 桌面目标后，VentuSample 即可从统一的 Mora 工作空间启动。Mora 提供应用目录和启动主题；采样生成与离线 DSP 仍由 VentuSample 原生实现。请参阅 [Mora 集成说明](docs/MORA_DESKTOP_INTEGRATION.md) 和 [DuckOS 工作空间契约](https://github.com/Ivan-VHS/DuckOS/blob/main/docs/MORA_DESKTOP.md)。
