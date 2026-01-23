#ifndef COLABB_PROFILE_DIALOG_HPP
#define COLABB_PROFILE_DIALOG_HPP

#include "infrastructure/config/profile_manager.hpp"
#include <gtk/gtk.h>
#include <string>
#include <vector>

namespace colabb {
namespace ui {

class ProfileDialog {
public:
    ProfileDialog(GtkWindow* parent, infrastructure::ProfileManager* manager);
    ~ProfileDialog();

    void run();

private:
    GtkWidget* dialog_;
    GtkWindow* parent_window_;
    infrastructure::ProfileManager* manager_;
    
    // UI Widgets
    GtkWidget* profile_list_box_;
    GtkWidget* name_entry_;
    GtkWidget* font_button_;
    GtkWidget* bg_color_button_;
    GtkWidget* fg_color_button_;
    GtkWidget* command_entry_;
    GtkWidget* blink_check_;
    GtkWidget* content_stack_; // To switch between empty state and editor

    // State
    std::string current_profile_name_;
    bool is_dirty_;

    // Setup
    void setup_ui();
    void load_profile_list();
    void load_profile_details(const std::string& name);
    
    // Handlers
    void on_profile_selected(GtkListBoxRow* row);
    void on_add_profile();
    void on_delete_profile();
    void on_save_changes();
    void on_set_default();

    // Static callbacks
    static void on_profile_row_selected_static(GtkListBox* box, GtkListBoxRow* row, gpointer user_data);
    static void on_add_clicked_static(GtkButton* button, gpointer user_data);
    static void on_delete_clicked_static(GtkButton* button, gpointer user_data);
    static void on_save_clicked_static(GtkButton* button, gpointer user_data);
    static void on_response_static(GtkDialog* dialog, gint response_id, gpointer user_data);
    
    // Helpers
    void clear_editor();
};

} // namespace ui
} // namespace colabb

#endif // COLABB_PROFILE_DIALOG_HPP
