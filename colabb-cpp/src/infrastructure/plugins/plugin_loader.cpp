#include "infrastructure/plugins/plugin_loader.hpp"

#include <iostream>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

using namespace colabb::infrastructure::plugins;

typedef domain::IAIProvider* (*create_fn_t)(const char*);
typedef void (*destroy_fn_t)(domain::IAIProvider*);

PluginLoader::PluginLoader() = default;

PluginLoader::~PluginLoader() {
    // Intentionally keep libraries loaded until program exit.
    // Future improvement: allow unload when no providers remain.
}

std::unique_ptr<domain::IAIProvider> PluginLoader::loadProvider(const std::string& path, const std::string& config_json) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (path.empty()) return nullptr;

#if defined(_WIN32)
    HMODULE handle = LoadLibraryA(path.c_str());
    if (!handle) {
        std::cerr << "PluginLoader: LoadLibrary failed: " << path << "\n";
        return nullptr;
    }
    auto create_fn = reinterpret_cast<create_fn_t>(GetProcAddress(handle, "create_provider"));
    auto destroy_fn = reinterpret_cast<destroy_fn_t>(GetProcAddress(handle, "destroy_provider"));
    if (!create_fn || !destroy_fn) {
        std::cerr << "PluginLoader: missing symbols in: " << path << "\n";
        FreeLibrary(handle);
        return nullptr;
    }
#else
    void* handle = dlopen(path.c_str(), RTLD_NOW);
    if (!handle) {
        std::cerr << "PluginLoader: dlopen failed: " << dlerror() << "\n";
        return nullptr;
    }
    dlerror();
    auto create_fn = reinterpret_cast<create_fn_t>(dlsym(handle, "create_provider"));
    const char* dlsym_err = dlerror();
    if (dlsym_err) {
        std::cerr << "PluginLoader: dlsym create_provider error: " << dlsym_err << "\n";
        dlclose(handle);
        return nullptr;
    }
    auto destroy_fn = reinterpret_cast<destroy_fn_t>(dlsym(handle, "destroy_provider"));
    dlsym_err = dlerror();
    if (dlsym_err) {
        std::cerr << "PluginLoader: dlsym destroy_provider error: " << dlsym_err << "\n";
        dlclose(handle);
        return nullptr;
    }
#endif

    domain::IAIProvider* raw = nullptr;
    try {
        raw = create_fn(config_json.c_str());
    } catch (const std::exception& e) {
        std::cerr << "PluginLoader: create_provider threw: " << e.what() << "\n";
        raw = nullptr;
    } catch (...) {
        std::cerr << "PluginLoader: create_provider threw unknown exception\n";
        raw = nullptr;
    }

    if (!raw) {
        // creation failed; close handle
#if defined(_WIN32)
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
        return nullptr;
    }

    // Keep the handle alive for program lifetime to ensure symbols remain valid
    handles_.push_back(handle);

    // Wrap raw pointer with custom deleter that calls plugin's destroy function
    auto deleter = [destroy_fn](domain::IAIProvider* p) {
        if (p && destroy_fn) {
            destroy_fn(p);
        }
    };

    return std::unique_ptr<domain::IAIProvider>(raw, deleter);
}
