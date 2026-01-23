#include "infrastructure/terminal/vte_terminal.hpp"
#include <fstream>
#include <sstream>
#include <regex>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>

namespace colabb {
namespace infrastructure {

TerminalWidget::TerminalWidget() 
    : vte_widget_(VTE_TERMINAL(vte_terminal_new()))
    , session_log_path_()
    , key_press_callback_(nullptr) {
    
    // Set up terminal appearance
    GdkRGBA bg_color, fg_color;
    gdk_rgba_parse(&bg_color, "#1e1e1e");
    gdk_rgba_parse(&fg_color, "#f0f0f0");
    vte_terminal_set_colors(vte_widget_, &fg_color, &bg_color, nullptr, 0);
    
    // Enable mouse autohide
    vte_terminal_set_mouse_autohide(vte_widget_, TRUE);
    
    // Connect key press event
    g_signal_connect(GTK_WIDGET(vte_widget_), "key-press-event",
                     G_CALLBACK(on_key_press_static), this);
}

TerminalWidget::~TerminalWidget() {
    // Cleanup log file
    if (!session_log_path_.empty()) {
        unlink(session_log_path_.c_str());
    }
}

void TerminalWidget::spawn_shell(const std::string& shell_path) {
    const char* home = getenv("HOME");
    if (!home) home = "/home";
    
    // Set up session log
    session_log_path_ = std::string(home) + "/.colabb_session.log";
    
    // Remove old log
    unlink(session_log_path_.c_str());
    
    // Wrap shell in 'script' command for logging
    const char* argv[] = {
        "/usr/bin/script",
        "-q",
        "-f",
        session_log_path_.c_str(),
        "-c",
        shell_path.c_str(),
        nullptr
    };
    
    const char* envp[] = { nullptr };
    
    GError* error = nullptr;
    vte_terminal_spawn_sync(
        vte_widget_,
        VTE_PTY_DEFAULT,
        home,
        const_cast<char**>(argv),
        const_cast<char**>(envp),
        G_SPAWN_DEFAULT,
        nullptr, nullptr,
        nullptr,
        nullptr,
        &error
    );
    
    if (error) {
        g_printerr("Failed to spawn shell: %s\n", error->message);
        g_error_free(error);
    }
}

std::string TerminalWidget::get_current_line() {
    // Read from log file
    std::string log_content = read_log_tail(500);
    if (log_content.empty()) {
        return "";
    }
    
    // Strip ANSI codes
    std::string clean = strip_ansi_codes(log_content);
    
    // Get last line
    auto lines = std::vector<std::string>();
    std::istringstream stream(clean);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    
    return lines.empty() ? "" : lines.back();
}

std::string TerminalWidget::get_context(int num_lines) {
    std::string log_content = read_log_tail(2000);
    if (log_content.empty()) {
        return "";
    }
    
    std::string clean = strip_ansi_codes(log_content);
    
    // Get last N lines
    auto lines = std::vector<std::string>();
    std::istringstream stream(clean);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    
    // Take last num_lines
    int start = std::max(0, static_cast<int>(lines.size()) - num_lines);
    std::ostringstream result;
    for (int i = start; i < static_cast<int>(lines.size()); ++i) {
        result << lines[i] << "\n";
    }
    
    return result.str();
}

void TerminalWidget::feed_text(const std::string& text) {
    vte_terminal_feed_child(vte_widget_, text.c_str(), text.length());
}

void TerminalWidget::clear_line() {
    // Send Ctrl+U to clear line
    const char ctrl_u = 0x15;
    vte_terminal_feed_child(vte_widget_, &ctrl_u, 1);
}

void TerminalWidget::set_key_press_callback(KeyPressCallback callback) {
    key_press_callback_ = std::move(callback);
}

gboolean TerminalWidget::on_key_press_static(GtkWidget* widget, GdkEventKey* event, gpointer user_data) {
    auto* self = static_cast<TerminalWidget*>(user_data);
    if (self && self->key_press_callback_) {
        self->key_press_callback_(event);
    }
    return FALSE; // Propagate event
}

std::string TerminalWidget::read_log_tail(size_t bytes) {
    if (session_log_path_.empty()) {
        return "";
    }
    
    std::ifstream file(session_log_path_, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return "";
    }
    
    std::streamsize size = file.tellg();
    if (size == 0) {
        return "";
    }
    
    // Seek to tail
    std::streamsize offset = std::max<std::streamsize>(0, size - bytes);
    file.seekg(offset);
    
    std::string content;
    content.resize(size - offset);
    file.read(&content[0], size - offset);
    
    return content;
}

std::string TerminalWidget::strip_ansi_codes(const std::string& text) {
    // Regex to match ANSI escape sequences
    static const std::regex ansi_regex(R"(\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~]))");
    return std::regex_replace(text, ansi_regex, "");
}

std::string TerminalWidget::get_current_directory() {
    const char* uri = vte_terminal_get_current_directory_uri(vte_widget_);
    if (!uri) return "";
    
    char* filename = g_filename_from_uri(uri, nullptr, nullptr);
    if (!filename) return "";
    
    std::string path(filename);
    g_free(filename);
    return path;
}

bool TerminalWidget::search_text(const std::string& pattern, bool case_sensitive, bool regex) {
    if (pattern.empty()) {
        clear_search();
        return false;
    }
    
    GError* error = nullptr;
    VteRegex* vte_regex = nullptr;
    
    if (regex) {
        // Create regex for search
        // Use 0 for default flags, add CASELESS if needed
        guint32 flags = 0;
        if (!case_sensitive) {
            flags = 1; // PCRE2_CASELESS equivalent
        }
        
        vte_regex = vte_regex_new_for_search(pattern.c_str(), pattern.length(), flags, &error);
        
        if (error) {
            g_printerr("Search regex error: %s\n", error->message);
            g_error_free(error);
            return false;
        }
    }
    
    // Set search parameters
    vte_terminal_search_set_regex(vte_widget_, vte_regex, 0);
    vte_terminal_search_set_wrap_around(vte_widget_, TRUE);
    
    if (vte_regex) {
        vte_regex_unref(vte_regex);
    }
    
    // Perform first search
    return vte_terminal_search_find_next(vte_widget_);
}

bool TerminalWidget::search_next() {
    return vte_terminal_search_find_next(vte_widget_);
}

bool TerminalWidget::search_previous() {
    return vte_terminal_search_find_previous(vte_widget_);
}

void TerminalWidget::clear_search() {
    vte_terminal_search_set_regex(vte_widget_, nullptr, 0);
}

void TerminalWidget::apply_profile(const domain::TerminalProfile& profile) {
    // Font
    std::string font_desc = profile.font_family + " " + std::to_string(profile.font_size);
    PangoFontDescription* font = pango_font_description_from_string(font_desc.c_str());
    vte_terminal_set_font(vte_widget_, font);
    pango_font_description_free(font);
    
    // Colors
    GdkRGBA bg, fg;
    gdk_rgba_parse(&bg, profile.background_color.c_str());
    gdk_rgba_parse(&fg, profile.foreground_color.c_str());
    
    // Palette
    std::vector<GdkRGBA> palette_rgba;
    for (const auto& color_hex : profile.palette) {
        GdkRGBA color;
        gdk_rgba_parse(&color, color_hex.c_str());
        palette_rgba.push_back(color);
    }
    
    vte_terminal_set_colors(vte_widget_, &fg, &bg, 
                            palette_rgba.data(), palette_rgba.size());
    
    // Cursor
    vte_terminal_set_cursor_blink_mode(vte_widget_, 
        profile.cursor_blink ? VTE_CURSOR_BLINK_ON : VTE_CURSOR_BLINK_OFF);
        
    // Scrollback
    vte_terminal_set_scrollback_lines(vte_widget_, profile.scrollback_lines);
    
    // Shell command update is slightly harder as it's spawn-time usually, 
    // but maybe we store it for next spawn.
}

} // namespace infrastructure
} // namespace colabb
