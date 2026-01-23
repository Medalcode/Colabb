#ifndef COLABB_CONTEXT_SERVICE_HPP
#define COLABB_CONTEXT_SERVICE_HPP

#include <string>
#include <vector>
#include <unordered_set>

namespace colabb {
namespace infrastructure {

class ContextService {
public:
    struct ProjectInfo {
        std::vector<std::string> languages;   // e.g., "C++", "Python"
        std::vector<std::string> build_tools; // e.g., "CMake", "Make", "NPM"
        bool is_git_repo;
        std::string git_branch;
        std::string project_name;
    };

    ContextService();
    ~ContextService() = default;

    // Analyzes the directory at current_path to extract context
    ProjectInfo detect_context(const std::string& current_path);

    // Generates a prompt string summarizing the context
    std::string get_context_prompt(const std::string& current_path);

private:
    // Helpers
    std::string get_project_name(const std::string& path);
    void detect_languages_and_tools(const std::string& path, ProjectInfo& info);
    void detect_git_status(const std::string& path, ProjectInfo& info);
    
    // Signatures
    bool has_file(const std::string& path, const std::string& filename);
    bool has_extension(const std::string& path, const std::string& ext);
};

} // namespace infrastructure
} // namespace colabb

#endif // COLABB_CONTEXT_SERVICE_HPP
