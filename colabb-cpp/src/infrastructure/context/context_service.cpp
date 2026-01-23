#include "infrastructure/context/context_service.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

namespace colabb {
namespace infrastructure {

ContextService::ContextService() {}

ContextService::ProjectInfo ContextService::detect_context(const std::string& current_path) {
    ProjectInfo info;
    info.is_git_repo = false;
    
    try {
        if (!fs::exists(current_path) || !fs::is_directory(current_path)) {
            return info;
        }

        info.project_name = get_project_name(current_path);
        detect_languages_and_tools(current_path, info);
        detect_git_status(current_path, info);

    } catch (const std::exception& e) {
        std::cerr << "Error detecting context: " << e.what() << std::endl;
    }

    return info;
}

std::string ContextService::get_context_prompt(const std::string& current_path) {
    auto info = detect_context(current_path);
    
    std::string prompt = "Project Context:\n";
    prompt += "- Name: " + info.project_name + "\n";
    
    if (!info.languages.empty()) {
        prompt += "- Languages: ";
        for (const auto& lang : info.languages) prompt += lang + ", ";
        prompt = prompt.substr(0, prompt.length() - 2) + "\n";
    }
    
    if (!info.build_tools.empty()) {
        prompt += "- Build Tools: ";
        for (const auto& tool : info.build_tools) prompt += tool + ", ";
        prompt = prompt.substr(0, prompt.length() - 2) + "\n";
    }
    
    if (info.is_git_repo) {
        prompt += "- Git: Yes";
        if (!info.git_branch.empty()) {
            prompt += " (Branch: " + info.git_branch + ")";
        }
        prompt += "\n";
    }
    
    return prompt;
}

std::string ContextService::get_project_name(const std::string& path) {
    return fs::path(path).filename().string();
}

void ContextService::detect_languages_and_tools(const std::string& path, ProjectInfo& info) {
    // Check for build files first to infer tools and likely languages
    if (has_file(path, "CMakeLists.txt")) {
        info.build_tools.push_back("CMake");
        info.languages.push_back("C++");
    }
    if (has_file(path, "Makefile")) {
        info.build_tools.push_back("Make");
        // Could be C, C++, Go, etc.
    }
    if (has_file(path, "package.json")) {
        info.build_tools.push_back("NPM");
        info.languages.push_back("JavaScript/TypeScript");
    }
    if (has_file(path, "requirements.txt") || has_file(path, "pyproject.toml") || has_file(path, "poetry.lock")) {
        info.build_tools.push_back("Pip/Poetry");
        info.languages.push_back("Python");
    }
    if (has_file(path, "Cargo.toml")) {
        info.build_tools.push_back("Cargo");
        info.languages.push_back("Rust");
    }
    if (has_file(path, "go.mod")) {
        info.build_tools.push_back("Go");
        info.languages.push_back("Go");
    }
    
    // Scan direct children for extensions if language list is empty
    if (info.languages.empty()) {
        bool has_cpp = false, has_py = false, has_js = false;
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".cpp" || ext == ".hpp" || ext == ".cc" || ext == ".c") has_cpp = true;
                else if (ext == ".py") has_py = true;
                else if (ext == ".js" || ext == ".ts") has_js = true;
            }
        }
        if (has_cpp) info.languages.push_back("C/C++");
        if (has_py) info.languages.push_back("Python");
        if (has_js) info.languages.push_back("JavaScript");
    }
    
    // Deduplicate
    std::sort(info.languages.begin(), info.languages.end());
    info.languages.erase(std::unique(info.languages.begin(), info.languages.end()), info.languages.end());
    std::sort(info.build_tools.begin(), info.build_tools.end());
    info.build_tools.erase(std::unique(info.build_tools.begin(), info.build_tools.end()), info.build_tools.end());
}

void ContextService::detect_git_status(const std::string& path, ProjectInfo& info) {
    if (fs::exists(path + "/.git")) {
        info.is_git_repo = true;
        
        // Try to read HEAD to get branch
        std::ifstream head_file(path + "/.git/HEAD");
        if (head_file.is_open()) {
            std::string line;
            std::getline(head_file, line); // e.g., "ref: refs/heads/main"
            if (line.rfind("ref: refs/heads/", 0) == 0) {
                info.git_branch = line.substr(16);
            } else {
                info.git_branch = "DETACHED";
            }
        }
    }
}

bool ContextService::has_file(const std::string& path, const std::string& filename) {
    return fs::exists(path + "/" + filename);
}

} // namespace infrastructure
} // namespace colabb
