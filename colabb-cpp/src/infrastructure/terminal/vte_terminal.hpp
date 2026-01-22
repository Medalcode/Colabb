#ifndef COLABB_TERMINAL_WIDGET_HPP
#define COLABB_TERMINAL_WIDGET_HPP

#include <gtk/gtk.h>
#include <vte/vte.h>
#include <string>
#include <functional>
#include <memory>

namespace colabb {
namespace infrastructure {

class TerminalWidget {
public:
    TerminalWidget();
    ~TerminalWidget();

    // Terminal operations
    void spawn_shell(const std::string& shell_path);
    std::string get_current_line();
    std::string get_context(int num_lines = 20);
    void feed_text(const std::string& text);
    void clear_line();

    // Event callbacks
    using KeyPressCallback = std::function<void(GdkEventKey*)>;
    void set_key_press_callback(KeyPressCallback callback);

    // Widget access
    GtkWidget* widget() const { return GTK_WIDGET(vte_widget_); }

private:
    ::VteTerminal* vte_widget_;
    std::string session_log_path_;
    KeyPressCallback key_press_callback_;

    // Static callback wrapper for GTK
    static gboolean on_key_press_static(GtkWidget* widget, GdkEventKey* event, gpointer user_data);
    
    // Helper methods
    std::string read_log_tail(size_t bytes = 2000);
    std::string strip_ansi_codes(const std::string& text);
};

} // namespace infrastructure
} // namespace colabb

#endif // COLABB_TERMINAL_WIDGET_HPP
