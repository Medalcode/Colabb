#ifndef COLABB_PLUGIN_LOADER_HPP
#define COLABB_PLUGIN_LOADER_HPP

#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include "domain/ai/ai_provider.hpp"

namespace colabb {
namespace infrastructure {
namespace plugins {

class PluginLoader {
public:
    PluginLoader();
    ~PluginLoader();

    // Load a provider from a shared library. Returns nullptr on failure.
    // The returned unique_ptr will call the plugin's destroy function when destroyed.
    std::unique_ptr<domain::IAIProvider> loadProvider(const std::string& path, const std::string& config_json);

private:
    std::mutex mutex_;
    // Opaque handles to loaded libraries (kept alive for program lifetime)
    std::vector<void*> handles_;
};

} // namespace plugins
} // namespace infrastructure
} // namespace colabb

#endif // COLABB_PLUGIN_LOADER_HPP
