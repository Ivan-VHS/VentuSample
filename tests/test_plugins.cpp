#include "plugin_manager.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#if !defined(_WIN32)
#include <sys/stat.h>
#endif

namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void testBuiltinRegistry() {
    ventu::PluginManager manager;
    const auto plugins = manager.plugins();
    require(plugins.size() == 19u, "all nineteen shared products must be registered as plugins");
    require(manager.find("Dropaul") != nullptr, "display names should resolve");
    require(manager.find("keel-koh") != nullptr, "stable plugin IDs should resolve");
    require(manager.find("stochastic-router") != nullptr, "Stochastic Router must be registered as a built-in plugin");
    require(manager.find("patakee") == nullptr, "retired workflow names must not be accepted");
    for (const auto& plugin : plugins) {
        require(manager.isEnabled(plugin.id), "built-in plugins should be enabled by default");
        require(manager.find(plugin.id) != nullptr, "every registry entry must have a descriptor");
    }
}

void testEnableDisableAndDispatch() {
    ventu::PluginManager manager;
    ventu::PluginContext context;
    context.parameters.bpm = 93;
    const auto enabled = manager.execute("operandu", context);
    require(enabled.success && enabled.summary.find("Operandu") != std::string::npos,
            "Operandu should inspect the active generator parameters");
    require(manager.setEnabled("Dropaul", false), "registered plugins should be disableable");
    const auto disabled = manager.execute("Dropaul", context);
    require(!disabled.success && disabled.summary.find("disabled") != std::string::npos,
            "disabled plugins must not run");
    require(manager.setEnabled("dropaul", true), "plugins should be re-enableable by stable ID");
    const auto plan = manager.execute("dropaul", context);
    require(plan.success && plan.details.size() >= 5u,
            "Dropaul should create a read-only export dry-run plan");
    const auto stochastic = manager.execute("stochastic-router", context);
    require(stochastic.success && stochastic.summary.find("preview route") != std::string::npos,
            "Stochastic Router should produce a bounded in-memory preview");
}

void testExtensionRegistration() {
    ventu::PluginManager manager;
    ventu::PluginDescriptor descriptor{
        "test-extension", "Test Extension", "Tests", "1.0.0",
        "Test SDK registration.", "test", {"registration"}
    };
    const bool registered = manager.registerPlugin(
        descriptor,
        [](const ventu::PluginContext&) {
            return ventu::PluginResult{true, "extension invoked", {}};
        },
        false);
    require(registered, "a host extension should register through the public SDK");
    require(!manager.isEnabled("test-extension"), "extensions can start disabled");
    require(!manager.execute("test-extension", {}).success, "disabled extension must be blocked");
    require(manager.setEnabled("test-extension", true), "host should enable a registered extension");
    require(manager.execute("test-extension", {}).summary == "extension invoked",
            "host should dispatch to the registered extension");
    require(!manager.registerPlugin(descriptor, [](const ventu::PluginContext&) {
        return ventu::PluginResult{true, "duplicate", {}};
    }), "duplicate plugin IDs or names must be rejected");
    require(!manager.registerPlugin(
        {"Bad ID", "Bad Plugin", "Tests", "1.0.0", "Invalid id", "test", {}},
        [](const ventu::PluginContext&) { return ventu::PluginResult{true, "bad", {}}; }),
        "invalid plugin IDs must be rejected");
}

void testManifestDiscoveryAndLifecycle() {
    namespace fs = std::filesystem;
    const auto root = fs::temp_directory_path() / "ventusample-plugin-platform-test";
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root / "hello", ec);
    require(!ec, "test extension package directory should be created");
    const auto manifestPath = root / "hello" / "plugin.json";
    {
        std::ofstream manifest(manifestPath, std::ios::binary);
        manifest << R"json({
  "schema": "ivan-vhs.plugin.v1",
  "id": "hello-extension",
  "name": "Hello Extension",
  "version": "1.0.0",
  "api_version": 1,
  "host": { "id": "ventu" },
  "domain": "examples",
  "description": "Metadata-only extension discovery test.",
  "capabilities": ["commands"],
  "entrypoint": "plugin.cpp:register_plugin"
})json";
    }

    ventu::PluginManager manager;
    std::vector<std::string> diagnostics;
    require(manager.discoverManifests(root, &diagnostics) == 1u,
            "the host should discover a valid plugin manifest");
    const auto* discovered = manager.find("hello-extension");
    require(discovered != nullptr, "discovered metadata should enter the registry");
    require(discovered->source == "manifest", "manifest records should be distinguishable from built-ins");
    require(discovered->manifestPath == manifestPath.string(), "manifest source path should be retained");
    require(!manager.isEnabled("hello-extension"), "manifest-only plugins must default to disabled");
    require(!manager.setEnabled("hello-extension", true),
            "metadata discovery must not enable code without a reviewed handler");
    const auto pending = manager.execute("hello-extension", {});
    require(!pending.success && pending.summary.find("disabled") != std::string::npos,
            "manifest discovery must keep metadata-only plugins disabled until a reviewed handler is attached");

    int initialized = 0;
    int stopped = 0;
    require(manager.attachHandler(
        "hello-extension",
        [](const ventu::PluginContext&) {
            return ventu::PluginResult{true, "reviewed extension invoked", {"SDK handler"}};
        },
        [&](const ventu::PluginContext&) { ++initialized; },
        [&](const ventu::PluginContext&) { ++stopped; }),
        "reviewed host code should be attachable to discovered metadata");
    require(manager.setEnabled("hello-extension", true), "attached extension should be enableable");
    const auto result = manager.execute("hello-extension", {});
    require(result.success && result.summary == "reviewed extension invoked",
            "enabled extension should execute via the SDK handler");
    require(initialized == 1, "plugin initialization hook should run once before first dispatch");
    const auto statuses = manager.statuses();
    bool activeFound = false;
    for (const auto& status : statuses) {
        if (status.descriptor.id == "hello-extension") {
            activeFound = status.initialized && status.enabled && status.handlerAttached;
        }
    }
    require(activeFound, "diagnostics should expose enabled and initialized state");
    require(manager.setEnabled("hello-extension", false), "extension should disable cleanly");
    require(stopped == 1, "shutdown hook should run on disable");
    require(manager.setEnabled("hello-extension", true), "extension can be re-enabled for a hot-swap guard check");
    require(!manager.hotSwapHandler("hello-extension", [](const ventu::PluginContext&) {
        return ventu::PluginResult{true, "unsafe live replacement", {}};
    }), "an enabled handler must never be replaced");
    require(manager.setEnabled("hello-extension", false), "extension should stop before replacement");
    require(manager.hotSwapHandler("hello-extension", [](const ventu::PluginContext&) {
        return ventu::PluginResult{true, "hot-swapped reviewed handler", {"new generation"}};
    }), "disabled stopped handlers should be replaceable with reviewed host code");
    require(manager.setEnabled("hello-extension", true), "hot-swapped handler should be enableable");
    const auto swapped = manager.execute("hello-extension", {});
    require(swapped.success && swapped.summary == "hot-swapped reviewed handler",
            "dispatch should use the replacement handler after the guarded swap");
    fs::remove_all(root, ec);
}

void testManifestReaderRejectsOversizedAndSpecialInputs() {
    namespace fs = std::filesystem;
    const auto root = fs::temp_directory_path() / "ventusample-plugin-safe-reader-test";
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root / "valid", ec);
    require(!ec, "safe reader test directory should be created");

    const auto validManifest = root / "valid" / "plugin.json";
    {
        std::ofstream manifest(validManifest, std::ios::binary);
        manifest << R"json({
  "schema": "ivan-vhs.plugin.v1",
  "id": "safe-reader-valid",
  "name": "Safe Reader Valid",
  "version": "1.0.0",
  "api_version": 1,
  "host": { "id": "ventu" },
  "domain": "tests",
  "description": "Valid bounded metadata fixture.",
  "capabilities": ["commands"],
  "entrypoint": "ignored.cpp:register"
})json";
    }

    fs::create_directories(root / "oversized", ec);
    require(!ec, "oversized manifest directory should be created");
    {
        std::ofstream oversized(root / "oversized" / "plugin.json", std::ios::binary);
        oversized << "{\"payload\":\"";
        for (std::size_t i = 0u; i < 70u * 1024u; ++i) oversized.put('x');
        oversized << "\"}";
    }

    fs::create_directories(root / "invalid-utf8", ec);
    require(!ec, "invalid UTF-8 fixture directory should be created");
    {
        std::string malformed = R"json({"schema":"ivan-vhs.plugin.v1","id":"invalid-utf8","name":"bad)json";
        malformed.push_back(static_cast<char>(0xC0));
        malformed += R"json(","version":"1.0.0","api_version":1,"host":{"id":"ventu"},"domain":"tests","description":"invalid UTF-8","capabilities":["commands"],"entrypoint":"ignored"})json";
        std::ofstream file(root / "invalid-utf8" / "plugin.json", std::ios::binary);
        file.write(malformed.data(), static_cast<std::streamsize>(malformed.size()));
    }

    fs::create_directories(root / "symlinked", ec);
    require(!ec, "symlink fixture directory should be created");
    fs::create_symlink(validManifest, root / "symlinked" / "plugin.json", ec);
    const bool symlinkCreated = !ec;
    ec.clear();

#if !defined(_WIN32)
    fs::create_directories(root / "special", ec);
    require(!ec, "special file fixture directory should be created");
    const auto fifo = root / "special" / "plugin.json";
    require(::mkfifo(fifo.c_str(), 0600) == 0, "FIFO fixture should be created");
#endif

    ventu::PluginManager manager;
    std::vector<std::string> diagnostics;
    const auto discovered = manager.discoverManifests(root, &diagnostics);
    require(discovered == 1u, "only the valid regular manifest should be discovered");
    require(manager.find("safe-reader-valid") != nullptr, "valid manifest should be retained");
    bool oversizedReported = false;
    for (const auto& diagnostic : diagnostics) {
        if (diagnostic.find("manifest unreadable or larger than 64 KiB") != std::string::npos)
            oversizedReported = true;
    }
    require(oversizedReported, "oversized manifest should produce a bounded diagnostic");
    require(manager.find("invalid-utf8") == nullptr,
            "manifest files with malformed UTF-8 must never enter the plugin registry");
    if (symlinkCreated) {
        require(manager.find("safe-reader-valid") != nullptr,
                "symlinked manifest must not cause duplicate metadata registration");
    }
    fs::remove_all(root, ec);
}

void testTopLevelManifestFieldsAreNotShadowedByNestedKeys() {
    namespace fs = std::filesystem;
    const auto root = fs::temp_directory_path() / "ventusample-plugin-top-level-fields-test";
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root, ec);
    require(!ec, "top-level manifest fixture directory should be created");
    {
        std::ofstream manifest(root / "plugin.json", std::ios::binary);
        manifest << R"json({
  "schema": "ivan-vhs.plugin.v1",
  "host": { "id": "ventu" },
  "metadata": { "id": "nested-id", "api_version": 99, "capabilities": ["wrong.capability"] },
  "id": "top-level-plugin",
  "name": "Top Level Plugin",
  "version": "1.0.0",
  "api_version": 1,
  "domain": "tests",
  "description": "Top-level field extraction test.",
  "capabilities": ["commands"],
  "entrypoint": "ignored.cpp:register"
})json";
    }
    fs::create_directories(root / "duplicate", ec);
    require(!ec, "duplicate-key fixture directory should be created");
    {
        std::ofstream manifest(root / "duplicate" / "plugin.json", std::ios::binary);
        manifest << R"json({
  "schema": "ivan-vhs.plugin.v1",
  "id": "duplicate-first",
  "id": "duplicate-second",
  "name": "Duplicate Key Fixture",
  "version": "1.0.0",
  "api_version": 1,
  "host": { "id": "ventu" },
  "domain": "tests",
  "description": "Duplicate keys are invalid.",
  "capabilities": ["commands"],
  "entrypoint": "ignored.cpp:register"
})json";
    }

    ventu::PluginManager manager;
    std::vector<std::string> diagnostics;
    const auto discovered = manager.discoverManifests(root, &diagnostics);
    if (discovered != 1u) {
        std::cerr << "DEBUG discovered=" << discovered << " diagnostics:";
        for (const auto& diagnostic : diagnostics) std::cerr << " [" << diagnostic << "]";
        std::cerr << '\\n';
    }
    require(discovered == 1u,
            "nested host and metadata keys must not shadow top-level manifest fields");
    const auto* plugin = manager.find("top-level-plugin");
    require(plugin != nullptr, "the top-level plugin ID must be registered");
    require(plugin->entrypoint == "ignored.cpp:register", "top-level entrypoint must be selected");
    require(plugin->capabilities.size() == 1u && plugin->capabilities[0] == "commands",
            "nested capabilities must not shadow the top-level capability list");
    require(manager.find("nested-id") == nullptr,
            "nested metadata identifiers must not be registered as plugin IDs");
    fs::remove_all(root, ec);
}

void testInvalidAndUnknownPlugin() {
    ventu::PluginManager manager;
    ventu::PluginContext context;
    context.parameters.bpm = 20;
    const auto invalid = manager.execute("profe", context);
    require(!invalid.success, "Profe must reject invalid generator parameters");
    const auto unknown = manager.execute("does-not-exist", context);
    require(!unknown.success, "unknown plugin should return a bounded error");
}
}

int main() {
    testBuiltinRegistry();
    testEnableDisableAndDispatch();
    testExtensionRegistration();
    testManifestDiscoveryAndLifecycle();
    testManifestReaderRejectsOversizedAndSpecialInputs();
    testTopLevelManifestFieldsAreNotShadowedByNestedKeys();
    testInvalidAndUnknownPlugin();
    std::cout << "VentuSample plugin tests passed.\n";
    return 0;
}
