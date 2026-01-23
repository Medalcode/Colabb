#ifndef COLABB_SEARCH_BAR_HPP
#define COLABB_SEARCH_BAR_HPP

#include <gtk/gtk.h>
#include <string>
#include <functional>

namespace colabb {
namespace ui {

class SearchBar {
public:
    explicit SearchBar(GtkWidget* parent);
    ~SearchBar();
    
    void show();
    void hide();
    bool is_visible() const;
    
    GtkWidget* widget() const { return revealer_; }
    
    // Callbacks
    using SearchCallback = std::function<void(const std::string&, bool, bool)>;
    using NavigationCallback = std::function<void(bool)>; // true = next, false = prev
    
    void set_search_callback(SearchCallback callback);
    void set_navigation_callback(NavigationCallback callback);
    
    void focus_entry();
    
private:
    GtkWidget* revealer_;
    GtkWidget* search_box_;
    GtkEntry* search_entry_;
    GtkCheckButton* case_sensitive_;
    GtkCheckButton* regex_mode_;
    GtkButton* prev_btn_;
    GtkButton* next_btn_;
    GtkButton* close_btn_;
    
    SearchCallback search_callback_;
    NavigationCallback navigation_callback_;
    
    // Event handlers
    void on_search_changed();
    void on_next_clicked();
    void on_prev_clicked();
    void on_close_clicked();
    
    // Static GTK callbacks
    static void on_search_changed_static(GtkEntry* entry, gpointer user_data);
    static void on_next_clicked_static(GtkButton* button, gpointer user_data);
    static void on_prev_clicked_static(GtkButton* button, gpointer user_data);
    static void on_close_clicked_static(GtkButton* button, gpointer user_data);
    static gboolean on_key_press_static(GtkWidget* widget, GdkEventKey* event, gpointer user_data);
};

} // namespace ui
} // namespace colabb

#endif // COLABB_SEARCH_BAR_HPP
