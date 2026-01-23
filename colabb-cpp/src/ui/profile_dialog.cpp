#include "ui/profile_dialog.hpp"
#include <iostream>

namespace colabb {
namespace ui {

ProfileDialog::ProfileDialog(GtkWindow* parent, infrastructure::ProfileManager* manager)
    : parent_window_(parent)
    , manager_(manager)
    , current_profile_name_("")
    , is_dirty_(false) {
    setup_ui();
}

ProfileDialog::~ProfileDialog() {
    if (dialog_) {
        gtk_widget_destroy(dialog_);
    }
}

void ProfileDialog::run() {
    gtk_dialog_run(GTK_DIALOG(dialog_));
}

void ProfileDialog::setup_ui() {
    dialog_ = gtk_dialog_new_with_buttons(
        "Perfiles de Terminal",
        parent_window_,
        GTK_DIALOG_MODAL,
        "Cerrar", GTK_RESPONSE_CLOSE,
        nullptr
    );
    gtk_window_set_default_size(GTK_WINDOW(dialog_), 700, 500);

    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog_));
    // Remove default packing to use our full layout
    
    // Main horizontal paned (split view)
    GtkWidget* paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add(GTK_CONTAINER(content_area), paned);
    gtk_paned_set_position(GTK_PANED(paned), 200);

    // --- LEFT SIDE: Profile List ---
    GtkWidget* left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // List container with scroll
    GtkWidget* scrolled = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    
    profile_list_box_ = gtk_list_box_new();
    g_signal_connect(profile_list_box_, "row-selected", G_CALLBACK(on_profile_row_selected_static), this);
    gtk_container_add(GTK_CONTAINER(scrolled), profile_list_box_);
    
    gtk_box_pack_start(GTK_BOX(left_box), scrolled, TRUE, TRUE, 0);

    // Toolbar for Add/Delete
    GtkWidget* toolbar = gtk_toolbar_new();
    gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
    
    GtkToolItem* add_btn = gtk_tool_button_new(nullptr, nullptr);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(add_btn), "list-add-symbolic");
    g_signal_connect(add_btn, "clicked", G_CALLBACK(on_add_clicked_static), this);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), add_btn, -1);

    GtkToolItem* del_btn = gtk_tool_button_new(nullptr, nullptr);
    gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(del_btn), "list-remove-symbolic");
    g_signal_connect(del_btn, "clicked", G_CALLBACK(on_delete_clicked_static), this);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), del_btn, -1);

    gtk_box_pack_end(GTK_BOX(left_box), toolbar, FALSE, FALSE, 0);
    
    gtk_paned_add1(GTK_PANED(paned), left_box);

    // --- RIGHT SIDE: Editor ---
    GtkWidget* right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(right_box), 20);

    // Form Grid
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    int row = 0;
    
    // Name
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Nombre:"), 0, row, 1, 1);
    name_entry_ = gtk_entry_new();
    gtk_widget_set_sensitive(name_entry_, FALSE); // Name mostly read-only for key, or create new logic
    gtk_grid_attach(GTK_GRID(grid), name_entry_, 1, row++, 1, 1);

    // Font
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Fuente:"), 0, row, 1, 1);
    font_button_ = gtk_font_button_new();
    gtk_grid_attach(GTK_GRID(grid), font_button_, 1, row++, 1, 1);

    // Command
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Comando:"), 0, row, 1, 1);
    command_entry_ = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(command_entry_), "Shell por defecto (vacío)");
    gtk_grid_attach(GTK_GRID(grid), command_entry_, 1, row++, 1, 1);

    // Colors
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Fondo:"), 0, row, 1, 1);
    bg_color_button_ = gtk_color_button_new();
    gtk_grid_attach(GTK_GRID(grid), bg_color_button_, 1, row++, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Texto:"), 0, row, 1, 1);
    fg_color_button_ = gtk_color_button_new();
    gtk_grid_attach(GTK_GRID(grid), fg_color_button_, 1, row++, 1, 1);

    // Blink
    blink_check_ = gtk_check_button_new_with_label("Cursor parpadeante");
    gtk_grid_attach(GTK_GRID(grid), blink_check_, 1, row++, 1, 1);

    gtk_box_pack_start(GTK_BOX(right_box), grid, FALSE, FALSE, 0);

    // Save Button
    GtkWidget* save_btn = gtk_button_new_with_label("Guardar Cambios");
    gtk_style_context_add_class(gtk_widget_get_style_context(save_btn), "suggested-action");
    g_signal_connect(save_btn, "clicked", G_CALLBACK(on_save_clicked_static), this);
    gtk_widget_set_halign(save_btn, GTK_ALIGN_END);
    gtk_box_pack_end(GTK_BOX(right_box), save_btn, FALSE, FALSE, 0);

    gtk_paned_add2(GTK_PANED(paned), right_box);

    gtk_widget_show_all(content_area);
    
    load_profile_list();
    
    // Select default if available
    // (Simplification: just select first)
}

void ProfileDialog::load_profile_list() {
    // Clear list
    GList* children = gtk_container_get_children(GTK_CONTAINER(profile_list_box_));
    for (GList* iter = children; iter != nullptr; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

    // Populate
    auto names = manager_->get_profile_names();
    std::string default_name = manager_->get_default_profile_name();

    for (const auto& name : names) {
        GtkWidget* row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        GtkWidget* label = gtk_label_new(name.c_str());
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(row_box), label, TRUE, TRUE, 0);

        if (name == default_name) {
            GtkWidget* badge = gtk_label_new(" (Default) ");
             gtk_style_context_add_class(gtk_widget_get_style_context(badge), "dim-label");
            gtk_box_pack_start(GTK_BOX(row_box), badge, FALSE, FALSE, 0);
        }

        gtk_widget_show_all(row_box);
        gtk_list_box_insert(GTK_LIST_BOX(profile_list_box_), row_box, -1);
        
        // Store name as object data
        g_object_set_data_full(G_OBJECT(row_box), "profile_name", g_strdup(name.c_str()), g_free);
    }
}

void ProfileDialog::on_profile_selected(GtkListBoxRow* row) {
    if (!row) return;
    
    GtkWidget* child = gtk_bin_get_child(GTK_BIN(row));
    const char* name = (const char*)g_object_get_data(G_OBJECT(child), "profile_name");
    
    if (name) {
        load_profile_details(name);
    }
}

void ProfileDialog::load_profile_details(const std::string& name) {
    auto profile = manager_->get_profile(name);
    current_profile_name_ = name;
    
    gtk_entry_set_text(GTK_ENTRY(name_entry_), profile.name.c_str());
    
    // Font
    std::string font_str = profile.font_family + " " + std::to_string(profile.font_size);
    gtk_font_button_set_font_name(GTK_FONT_BUTTON(font_button_), font_str.c_str());
    
    // Command
    gtk_entry_set_text(GTK_ENTRY(command_entry_), profile.startup_command.c_str());
    
    // Colors
    GdkRGBA bg, fg;
    gdk_rgba_parse(&bg, profile.background_color.c_str());
    gdk_rgba_parse(&fg, profile.foreground_color.c_str());
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(bg_color_button_), &bg);
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(fg_color_button_), &fg);
    
    // Check
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(blink_check_), profile.cursor_blink);
}

void ProfileDialog::on_save_changes() {
    if (current_profile_name_.empty()) return;
    
    domain::TerminalProfile profile = manager_->get_profile(current_profile_name_);
    
    // Update fields
    // Font
    const char* font_name = gtk_font_button_get_font_name(GTK_FONT_BUTTON(font_button_));
    PangoFontDescription* desc = pango_font_description_from_string(font_name);
    if (desc) {
        const char* family = pango_font_description_get_family(desc);
        if (family) profile.font_family = family;
        profile.font_size = pango_font_description_get_size(desc) / PANGO_SCALE;
        pango_font_description_free(desc);
    }

    // Command
    profile.startup_command = gtk_entry_get_text(GTK_ENTRY(command_entry_));

    // Colors
    GdkRGBA bg, fg;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(bg_color_button_), &bg);
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(fg_color_button_), &fg);
    
    char* bg_str = gdk_rgba_to_string(&bg);
    char* fg_str = gdk_rgba_to_string(&fg);
    profile.background_color = bg_str;
    profile.foreground_color = fg_str;
    g_free(bg_str);
    g_free(fg_str);

    // Blink
    profile.cursor_blink = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(blink_check_));

    manager_->save_profile(profile);
}

void ProfileDialog::on_add_profile() {
    // Determine new name
    int count = 1;
    std::string base_name = "Nuevo Perfil";
    std::string name = base_name;
    auto names = manager_->get_profile_names();
    
    while (true) {
        bool exists = false;
        for (const auto& n : names) {
            if (n == name) { exists = true; break; }
        }
        if (!exists) break;
        name = base_name + " " + std::to_string(count++);
    }
    
    // Copy default
    auto profile = domain::TerminalProfile::create_default();
    profile.name = name;
    manager_->save_profile(profile);
    
    // Refresh
    load_profile_list();
    
    // Select new (implement finding the row...)
}

void ProfileDialog::on_delete_profile() {
    if (current_profile_name_.empty()) return;
    
    // Confirm dialog omitted for brevity
    manager_->delete_profile(current_profile_name_);
    
    current_profile_name_ = "";
    load_profile_list();
    // Clear right side
}

// Static callbacks
void ProfileDialog::on_profile_row_selected_static(GtkListBox* box, GtkListBoxRow* row, gpointer user_data) {
    auto* self = static_cast<ProfileDialog*>(user_data);
    self->on_profile_selected(row);
}

void ProfileDialog::on_add_clicked_static(GtkButton* button, gpointer user_data) {
    auto* self = static_cast<ProfileDialog*>(user_data);
    self->on_add_profile();
}

void ProfileDialog::on_delete_clicked_static(GtkButton* button, gpointer user_data) {
    auto* self = static_cast<ProfileDialog*>(user_data);
    self->on_delete_profile();
}

void ProfileDialog::on_save_clicked_static(GtkButton* button, gpointer user_data) {
    auto* self = static_cast<ProfileDialog*>(user_data);
    self->on_save_changes();
}

} // namespace ui
} // namespace colabb
