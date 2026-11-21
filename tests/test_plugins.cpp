#include "plugin_manager.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

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
    require(plugins.size() == 18u, "all eighteen shared products must be registered as plugins");
    require(manager.find("Dropaul") != nullptr, "display names should resolve");
    require(manager.find("keel-koh") != nullptr, "stable plugin IDs should resolve");
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
    testInvalidAndUnknownPlugin();
    std::cout << "VentuSample plugin tests passed.\n";
    return 0;
}
