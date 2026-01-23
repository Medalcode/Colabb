#ifndef COLABB_TERMINAL_PROFILE_HPP
#define COLABB_TERMINAL_PROFILE_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace colabb {
namespace domain {

struct TerminalProfile {
    std::string name;

    // Appearance
    std::string background_color;  // Hex format "#RRGGBB"
    std::string foreground_color;  // Hex format "#RRGGBB"
    std::vector<std::string> palette; // 16 colors for ANSI support

    // Font
    std::string font_family;       // e.g., "Monospace"
    int font_size;                 // e.g., 12

    // Behavior
    std::string startup_command;   // e.g., "/bin/bash", "/usr/bin/fish"
    int scrollback_lines;          // e.g., 10000
    bool cursor_blink;

    // Factory methods
    static TerminalProfile create_default() {
        TerminalProfile p;
        p.name = "Default";
        p.background_color = "#1e1e1e";
        p.foreground_color = "#f0f0f0";
        // Standard GNOME Terminal styled palette (approximate)
        p.palette = {
            "#2e3436", "#cc0000", "#4e9a06", "#c4a000", "#3465a4", "#75507b", "#06989a", "#d3d7cf", // 0-7
            "#555753", "#ef2929", "#8ae234", "#fce94f", "#729fcf", "#ad7fa8", "#34e2e2", "#eeeeec"  // 8-15
        };
        p.font_family = "Monospace";
        p.font_size = 12;
        p.startup_command = ""; // Empty means default shell
        p.scrollback_lines = 10000;
        p.cursor_blink = true;
        return p;
    }

    static TerminalProfile create_light() {
        TerminalProfile p = create_default();
        p.name = "Light";
        p.background_color = "#ffffff";
        p.foreground_color = "#1e1e1e";
        return p;
    }
};

// JSON Serialization declarations
void to_json(nlohmann::json& j, const TerminalProfile& p);
void from_json(const nlohmann::json& j, TerminalProfile& p);

} // namespace domain
} // namespace colabb

#endif // COLABB_TERMINAL_PROFILE_HPP
