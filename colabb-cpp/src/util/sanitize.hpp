// Sanitization and risk-check utilities for Colabb Terminal IA suggestions
#pragma once
#include <string>
#include <regex>

namespace colabb {
namespace util {

// Basic sanitizer: blocks obvious dangerous shell constructions
inline bool is_command_safe(const std::string& cmd) {
    // Block pipes, redirections, subshell, background, and sudo for now
    static const std::regex risky_patterns(R"([|&;<>
`$(){}]|\bsudo\b|\brm\s+-rf\b|\bshutdown\b|\breboot\b)");
    return !std::regex_search(cmd, risky_patterns);
}

// Basic shell escaping: (future improvement - make it stricter)
inline std::string shell_escape(const std::string& cmd) {
    // No real escaping yet, placeholder in case of future expansion
    // Only allow printable ASCII for the first version
    std::string clean;
    for (char c : cmd) {
        if ((c >= 32 && c <= 126) || c == '\n' || c == '\t') clean += c;
    }
    return clean;
}

} // namespace util
} // namespace colabb
