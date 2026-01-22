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

} // namespace infrastructure
} // namespace colabb
