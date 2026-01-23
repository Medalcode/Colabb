#ifndef COLABB_TAB_MANAGER_HPP
#define COLABB_TAB_MANAGER_HPP

#include "infrastructure/terminal/vte_terminal.hpp"
#include <gtk/gtk.h>
#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace colabb {
namespace ui {

class TabManager {
public:
    struct TabInfo {
        std::unique_ptr<infrastructure::TerminalWidget> terminal;
        GtkWidget* overlay;
        GtkWidget* scrolled_window;
        GtkWidget* suggestion_revealer;
        GtkWidget* suggestion_box;
        GtkLabel* suggestion_label;
        GtkButton* apply_button;
        GtkImage* icon_image;
        std::string title;
        int index;
    };
    
    explicit TabManager(GtkNotebook* notebook);
    ~TabManager();
    
    // Tab operations
    int create_tab(const std::string& title = "Terminal");
    void close_tab(int index);
    void close_current_tab();
    
    // Tab access
    TabInfo* get_tab(int index);
    TabInfo* get_current_tab();
    int get_current_tab_index();
    int get_tab_count();
    
    // Navigation
    void next_tab();
    void previous_tab();
    void switch_to_tab(int index);
    
    // Callbacks
    using TabCreatedCallback = std::function<void(TabInfo*)>;
    using TabClosedCallback = std::function<void(int)>;
    
    void set_tab_created_callback(TabCreatedCallback callback);
    void set_tab_closed_callback(TabClosedCallback callback);
    
private:
    GtkNotebook* notebook_;
    std::vector<std::unique_ptr<TabInfo>> tabs_;
    int next_tab_id_;
    
    TabCreatedCallback tab_created_callback_;
    TabClosedCallback tab_closed_callback_;
    
    GtkWidget* create_tab_label(const std::string& title, int index);
    GtkWidget* create_tab_content(TabInfo* tab);
    
    // Static callbacks
    static void on_tab_close_clicked_static(GtkButton* button, gpointer user_data);
    
    void on_tab_close_clicked(int index);
};

} // namespace ui
} // namespace colabb

#endif // COLABB_TAB_MANAGER_HPP
