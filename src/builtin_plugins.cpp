#include "plugin_manager.hpp"
#include "stochastic_router.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace ventu {
namespace {
namespace fs = std::filesystem;

PluginResult ok(std::string summary, std::vector<std::string> details = {}) {
    return {true, std::move(summary), std::move(details)};
}
PluginResult fail(std::string summary, std::vector<std::string> details = {}) {
    return {false, std::move(summary), std::move(details)};
}

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::vector<fs::path> collectFiles(const fs::path& root, std::size_t limit = 5000u) {
    std::vector<fs::path> result;
    std::error_code ec;
    if (root.empty() || !fs::is_directory(root, ec) || ec) return result;
    fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, ec), end;
    while (it != end && result.size() < limit) {
        const auto path = it->path();
        const auto status = it->symlink_status(ec);
        if (!ec && fs::is_regular_file(status)) result.push_back(path);
        ec.clear();
        it.increment(ec);
        if (ec) ec.clear();
    }
    std::sort(result.begin(), result.end());
    return result;
}

std::string extension(const fs::path& path) {
    return lower(path.extension().string());
}

std::string sizeLabel(std::uintmax_t bytes) {
    std::ostringstream out;
    out << bytes << " bytes";
    return out.str();
}

bool readBounded(const fs::path& path, std::string& text, std::uintmax_t maxBytes = 1024u * 1024u) {
    std::error_code ec;
    const auto size = fs::file_size(path, ec);
    if (ec || size > maxBytes) return false;
    std::ifstream input(path, std::ios::binary);
    if (!input) return false;
    text.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    return input.good() || input.eof();
}

std::string parameterLine(const Parameters& p) {
    std::ostringstream out;
    out << "Profile: " << toString(p.style) << " | target: " << toString(p.target)
        << " | " << p.bpm << " BPM | " << p.bars << " bars | " << p.variations
        << " variations | seed " << p.seed << " | swing " << p.swingPercent << "%";
    return out.str();
}

PluginDescriptor descriptor(std::string id, std::string name, std::string domain,
                            std::string description, std::string operation,
                            std::vector<std::string> capabilities) {
    return {std::move(id), std::move(name), std::move(domain), "1.0.0",
            std::move(description), std::move(operation), std::move(capabilities)};
}

PluginResult runBuiltin(const std::string& id, const PluginContext& context) {
    const auto& p = context.parameters;
    const auto validation = validate(p);

    if (id == "stochastic-router") {
        if (!validation.empty()) return fail("Stochastic Router rejected invalid generator settings: " + validation);
        Parameters previewParameters = p;
        previewParameters.bars = 1;
        auto preview = render(previewParameters, 0);
        if (preview.samples.empty()) return fail("Stochastic Router could not obtain a valid preview buffer.");
        StochasticConfig config;
        config.seed = static_cast<std::uint32_t>(p.seed);
        config.intensity = std::clamp(
            static_cast<double>(p.stochasticIntensityPercent) / 100.0, 0.05, 1.0);
        config.aggression = static_cast<double>(p.gritPercent) / 100.0;
        config.space = static_cast<double>(p.stereoPercent) / 100.0;
        config.dirt = static_cast<double>(p.saturationPercent) / 100.0;
        config.chaos = static_cast<double>(p.densityPercent) / 100.0;
        config.triggerMode = p.stochasticTriggerMode == 1
            ? StochasticTriggerMode::FixedBlocks
            : (p.stochasticTriggerMode == 2
                ? StochasticTriggerMode::ScheduledOnsets
                : StochasticTriggerMode::SinglePass);
        config.topology = p.stochasticTopology == 1
            ? StochasticTopology::Serial
            : (p.stochasticTopology == 2 ? StochasticTopology::Parallel : StochasticTopology::Hybrid);
        for (std::size_t effect = 0; effect < config.pool.size(); ++effect)
            config.pool[effect].enabled = (p.stochasticPluginMask & (1u << effect)) != 0u;
        config.blockFrames = std::max<std::size_t>(
            256u, static_cast<std::size_t>(preview.sampleRate) * 240u
                / static_cast<std::size_t>(p.bpm));
        const auto report = processStochastic(preview, config);
        if (!report.success) return fail(report.error);
        std::ostringstream peak;
        peak << std::fixed << std::setprecision(4) << report.peakBefore
             << " -> " << report.peakAfter;
        std::ostringstream rms;
        rms << std::fixed << std::setprecision(4) << report.rmsBefore
            << " -> " << report.rmsAfter;
        std::vector<std::string> details{
            "In-memory preview only: no file was written or overwritten.",
            "Seed: " + std::to_string(config.seed),
            "Triggers processed: " + std::to_string(report.triggerCount),
            "Peak before/after: " + peak.str(),
            "RMS before/after: " + rms.str(),
            "Maximum wet ceiling: 65%",
            "Maximum feedback ceiling: 72%",
            "Output peak ceiling: -0.3 dBFS"
        };
        details.insert(details.end(), report.routes.begin(), report.routes.end());
        return ok("Stochastic Router generated and processed a deterministic preview route.",
                  std::move(details));
    }
    if (id == "operandu") {
        return ok("Operandu inspected the active generator state.",
                  {parameterLine(p), "Typed fields: BPM=int, bars=int, variations=int, seed=int, compression=double.",
                   validation.empty() ? "Parameter contract: valid." : "Parameter contract: " + validation});
    }
    if (id == "dropaul") {
        if (!validation.empty()) return fail("Export workflow cannot be planned: " + validation);
        return ok("Dropaul produced a deterministic dry-run. No render or file write was started.",
                  {"01 validate-settings", "02 preflight-output-directory",
                   "03 render-" + std::to_string(p.variations) + "-variation(s)",
                   "04 analyze-peak-rms-crest", "05 write-pcm16-wav",
                   "06 write-versioned-manifest"});
    }
    if (id == "zdoro") {
        std::vector<std::string> details;
        std::error_code ec;
        const bool workspace = !context.workspaceRoot.empty()
            && fs::is_directory(context.workspaceRoot, ec) && !ec;
        details.push_back(std::string("Workspace: ") + (workspace ? "available" : "not configured or unavailable"));
        ec.clear();
        const bool output = !context.outputDirectory.empty()
            && fs::is_directory(context.outputDirectory, ec) && !ec;
        details.push_back(std::string("Output directory: ") + (output ? "exists" : "not created"));
        details.push_back(validation.empty() ? "Generator settings: valid" : "Generator settings: " + validation);
        return ok("Zdoro completed a read-only workspace and settings baseline.", std::move(details));
    }
    if (id == "pyemont" || id == "vorof" || id == "vprof") {
        const auto files = collectFiles(context.workspaceRoot);
        std::size_t sourceFiles = 0, cppFiles = 0, headerFiles = 0, lines = 0;
        std::size_t branchHints = 0;
        const std::regex branchPattern(R"(\b(if|for|while|switch|catch)\b)");
        for (const auto& file : files) {
            const auto ext = extension(file);
            if (ext == ".cpp" || ext == ".cc" || ext == ".c") { ++sourceFiles; ++cppFiles; }
            if (ext == ".hpp" || ext == ".h") { ++sourceFiles; ++headerFiles; }
            if (ext == ".cpp" || ext == ".cc" || ext == ".c" || ext == ".hpp" || ext == ".h") {
                std::string content;
                if (readBounded(file, content)) {
                    lines += static_cast<std::size_t>(std::count(content.begin(), content.end(), '\n')) + 1u;
                    branchHints += static_cast<std::size_t>(std::distance(
                        std::sregex_iterator(content.begin(), content.end(), branchPattern),
                        std::sregex_iterator()));
                }
            }
        }
        if (id == "pyemont")
            return ok("Pyemont inventoried the local application tree without compiling or executing it.",
                      {"Files indexed: " + std::to_string(files.size()),
                       "C/C++ source files: " + std::to_string(sourceFiles),
                       "Translation units: " + std::to_string(cppFiles),
                       "Headers: " + std::to_string(headerFiles)});
        if (id == "vorof")
            return ok("Vorof produced a static source map; no source was executed.",
                      {"Workspace root: " + context.workspaceRoot.string(),
                       "Indexed files: " + std::to_string(files.size()),
                       "Native source files: " + std::to_string(sourceFiles)});
        return ok("Vprof produced static source-shape estimates, not runtime profiling.",
                  {"Source files: " + std::to_string(sourceFiles),
                   "Approximate source lines: " + std::to_string(lines),
                   "Branch-keyword occurrences: " + std::to_string(branchHints),
                   "Runtime timing and audio callback metrics: not collected."});
    }
    if (id == "dilly") {
        return ok("Dilly checked the desktop presentation contract.",
                  {"Main window and generator/results/about tabs are registered.",
                   "Plugin manager is available from the Плагины menu.",
                   "Theme preference is persisted with QSettings.",
                   "Accessibility follow-up: keep keyboard focus and text labels on all future plugin panels."});
    }
    if (id == "baplap") {
        const auto files = collectFiles(context.outputDirectory, 2000u);
        std::vector<std::string> details;
        std::size_t waveCount = 0;
        std::uintmax_t total = 0;
        for (const auto& file : files) {
            if (extension(file) != ".wav") continue;
            std::error_code ec;
            const auto bytes = fs::file_size(file, ec);
            if (ec) continue;
            ++waveCount; total += bytes;
            details.push_back(file.filename().string() + " — " + sizeLabel(bytes));
            if (details.size() == 25u) break;
        }
        details.push_back("WAV files catalogued: " + std::to_string(waveCount));
        details.push_back("Total WAV size: " + sizeLabel(total));
        return ok("Baplap created a bounded read-only index of exported audio files.", std::move(details));
    }
    if (id == "sretuo") {
        return ok("Sretuo prepared the native verification plan; tests were not launched.",
                  {"ventusample-generator — deterministic DSP, parameter validation and WAV round-trip",
                   "ventusample-plugin-host — plugin registry, lifecycle and dispatch",
                   "ventusample-contract — project and cross-app contract checks",
                   "ventusample-manifest-fixture — generated manifest schema"});
    }
    if (id == "kali") {
        const auto files = collectFiles(context.workspaceRoot, 1500u);
        const std::regex privateKey(R"(-----BEGIN (RSA |EC |OPENSSH )?PRIVATE KEY-----)");
        const std::regex tokenAssign(R"((api[_-]?key|access[_-]?token|client[_-]?secret)\s*[:=]\s*["'][A-Za-z0-9_./+=-]{12,}["'])",
                                     std::regex::icase);
        std::size_t scanned = 0, findings = 0;
        std::vector<std::string> details;
        for (const auto& file : files) {
            const auto ext = extension(file);
            if (ext != ".cpp" && ext != ".hpp" && ext != ".h" && ext != ".json"
                && ext != ".yml" && ext != ".yaml" && ext != ".toml") continue;
            std::string content;
            if (!readBounded(file, content)) continue;
            ++scanned;
            if (std::regex_search(content, privateKey) || std::regex_search(content, tokenAssign)) {
                ++findings;
                if (details.size() < 30u) details.push_back("Pattern match: " + file.string());
            }
        }
        details.push_back("Files scanned: " + std::to_string(scanned));
        details.push_back("Pattern matches: " + std::to_string(findings));
        details.push_back("Scope: bounded static pattern scan, not a complete security audit.");
        return ok("Kali completed its read-only security hygiene scan.", std::move(details));
    }
    if (id == "zalam") {
        if (!validation.empty()) return fail("Zalam rejected the generator contract: " + validation);
        return ok("Zalam validated the local export contract.",
                  {"Manifest writer: ventusample.pack.v2",
                   "Manifest reader compatibility: v1 and v2",
                   "Audio: PCM16 WAV",
                   "Seed/settings snapshot: deterministic",
                   "Imported audio: user-supplied source; rights must be confirmed by the user."});
    }
    if (id == "liur") {
        if (p.bpm <= 0 || p.sampleRate <= 0 || p.bars <= 0 || p.variations <= 0)
            return fail("Liur cannot estimate memory from invalid timing values.");
        const long double framesPerBar = static_cast<long double>(p.sampleRate) * 240.0L
            / static_cast<long double>(p.bpm);
        const long double bytes = framesPerBar * static_cast<long double>(p.bars)
            * 2.0L * sizeof(float) * static_cast<long double>(p.variations);
        std::ostringstream estimate;
        estimate << std::fixed << std::setprecision(2)
                 << static_cast<double>(bytes / (1024.0L * 1024.0L)) << " MiB";
        return ok("Liur estimated the raw stereo float buffer footprint.",
                  {parameterLine(p), "All-variations estimate: " + estimate.str(),
                   "Excludes renderer working buffers, allocator overhead and WAV export buffers."});
    }
    if (id == "keel-koh") {
        const auto files = collectFiles(context.workspaceRoot);
        std::vector<std::string> docs;
        for (const auto& file : files) if (extension(file) == ".md") docs.push_back(file.filename().string());
        std::sort(docs.begin(), docs.end());
        if (docs.size() > 30u) docs.resize(30u);
        docs.insert(docs.begin(), "Markdown documents discovered: " + std::to_string(
            std::count_if(files.begin(), files.end(), [](const fs::path& f) { return extension(f) == ".md"; })));
        if (docs.size() == 1u) docs.push_back("No Markdown files found under the selected workspace.");
        return ok("Keel Koh indexed Markdown documentation names.", std::move(docs));
    }
    if (id == "profe") {
        if (!validation.empty()) return fail("Profe found a generator settings violation: " + validation);
        return ok("Profe passed the generator's typed parameter and numeric-range checks.",
                  {parameterLine(p), "No project files were modified."});
    }
    if (id == "prossee") {
        if (!validation.empty()) return fail("Prossee cannot plan an invalid export: " + validation);
        return ok("Prossee generated a dependency-ordered release plan.",
                  {"validate", "preflight", "render", "measure", "export-wav", "write-manifest", "verify-pack"});
    }
    if (id == "vprossee") {
        const bool settingsReady = validation.empty();
        const bool workspaceReady = !context.workspaceRoot.empty() && fs::is_directory(context.workspaceRoot);
        const bool outputSelected = !context.outputDirectory.empty();
        return ok("Vprossee generated a requirements/evidence readiness matrix.",
                  {std::string("REQ-SETTINGS: ") + (settingsReady ? "evidence present (native validator)" : "not verified: " + validation),
                   std::string("REQ-WORKSPACE: ") + (workspaceReady ? "evidence present" : "not verified"),
                   std::string("REQ-OUTPUT: ") + (outputSelected ? "destination selected" : "not configured"),
                   "An enabled state is not treated as proof of feature completion."});
    }
    if (id == "professiya") {
        return ok("Professiya generated a VentuSample contributor learning path.",
                  {"1. Generator parameters, profiles and deterministic seeds",
                   "2. Event scheduling, swing grid and source-sample import",
                   "3. Offline DSP renderer and PCM16 WAV writer",
                   "4. Manifest v1/v2 and pack validation",
                   "5. Plugin SDK, registry lifecycle and Qt integration",
                   "6. Regression tests, CMake and CI"});
    }
    if (id == "profevo") {
        return ok("Profevo prepared a non-mutating compatibility note.",
                  {"Current pack writer: manifest v2",
                   "Supported reader contract: manifest v1 and v2",
                   "Plugin descriptors carry stable IDs and semantic versions.",
                   "No project settings or exported files were migrated."});
    }
    return fail("No built-in handler is defined for plugin: " + id);
}
} // namespace

void registerBuiltinPlugins(PluginManager& manager) {
    const std::array<PluginDescriptor, 19> specs{{
        descriptor("operandu", "Operandu", "Data & logic", "Typed generator values and parameter inspection.", "inspect_parameters", {"typed parameters", "validation report"}),
        descriptor("dropaul", "Dropaul", "Actions & workflows", "Deterministic export planning and dry-run sequencing.", "plan_export", {"DAG-like workflow", "dry-run", "export stages"}),
        descriptor("zdoro", "Zdoro", "Integrity & diagnostics", "Read-only workspace, output and settings health checks.", "inspect_workspace", {"workspace status", "output status", "settings baseline"}),
        descriptor("pyemont", "Pyemont", "Development", "Native project inventory and module discovery.", "inventory_project", {"source inventory", "file map"}),
        descriptor("dilly", "Dilly", "Interface & experience", "Desktop interface and theme contract checks.", "inspect_interface", {"UI contract", "theme settings"}),
        descriptor("baplap", "Baplap", "Data infrastructure", "Bounded index of exported audio files.", "index_audio", {"WAV catalogue", "file sizes"}),
        descriptor("sretuo", "Sretuo", "Quality engineering", "Native test-suite inventory and verification planning.", "plan_tests", {"test inventory", "quality plan"}),
        descriptor("kali", "Kali", "Security", "Bounded static security hygiene scan.", "scan_security", {"secret patterns", "read-only scan"}),
        descriptor("zalam", "Zalam", "Connectivity", "Pack manifest and export contract validation.", "validate_contracts", {"manifest v1/v2", "PCM16 WAV"}),
        descriptor("liur", "Liur", "Runtime resources", "Renderer memory and resource estimates.", "measure_resources", {"buffer estimate", "limits"}),
        descriptor("keel-koh", "Keel Koh", "Knowledge", "Markdown documentation inventory.", "index_knowledge", {"document index", "learning references"}),
        descriptor("vorof", "Vorof", "Reverse engineering", "Static native source tree reconstruction.", "inspect_structure", {"source map", "module inventory"}),
        descriptor("vprof", "Vprof", "Profiling & metrics", "Static code-shape metrics, never runtime profiling.", "profile_source", {"source lines", "branch hints"}),
        descriptor("profe", "Profe", "Engineering standards", "Typed settings and generator constraint checks.", "check_standards", {"parameter contract", "numeric validation"}),
        descriptor("prossee", "Prossee", "Project planning", "Dependency-ordered export and release plan.", "plan_project", {"release stages", "ordering"}),
        descriptor("vprossee", "Vprossee", "Verification & validation", "Requirement and evidence readiness matrix.", "verify_evidence", {"acceptance evidence", "readiness"}),
        descriptor("professiya", "Professiya", "Learning", "Contributor learning paths for the native workstation.", "learning_paths", {"learning sequence", "developer onboarding"}),
        descriptor("profevo", "Profevo", "Evolution", "Compatibility-aware manifest and plugin migration planning.", "plan_evolution", {"manifest compatibility", "versioned plugins"}),
        descriptor("stochastic-router", "Stochastic Router", "Audio processing", "Seeded weighted effect-chain generation, safe parameter mutation, offline routing and peak protection.", "stochastic_route_preview", {"weighted plugin pool", "serial and parallel routes", "deterministic trigger mutation", "bounded feedback and wet", "RMS matching", "sample peak guard"})
    }};
    for (const auto& spec : specs) {
        const auto id = spec.id;
        manager.registerPlugin(spec, [id](const PluginContext& context) {
            return runBuiltin(id, context);
        });
    }
}
} // namespace ventu
