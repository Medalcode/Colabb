#include "ui/search_bar.hpp"

namespace colabb {
namespace ui {

SearchBar::SearchBar(GtkWidget* parent)
    : revealer_(nullptr)
    , search_box_(nullptr)
    , search_entry_(nullptr)
    , case_sensitive_(nullptr)
    , regex_mode_(nullptr)
    , prev_btn_(nullptr)
    , next_btn_(nullptr)
    , close_btn_(nullptr) {
    
    // Create revealer for smooth show/hide
    revealer_ = gtk_revealer_new();
    gtk_revealer_set_transition_type(GTK_REVEALER(revealer_),
        GTK_REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
    gtk_revealer_set_transition_duration(GTK_REVEALER(revealer_), 150);
    
    // Create search box
    search_box_ = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_style_context_add_class(gtk_widget_get_style_context(search_box_), "search-bar");
    gtk_widget_set_margin_start(search_box_, 10);
    gtk_widget_set_margin_end(search_box_, 10);
    gtk_widget_set_margin_top(search_box_, 6);
    gtk_widget_set_margin_bottom(search_box_, 6);
    
    // Search entry
    search_entry_ = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_placeholder_text(search_entry_, "Buscar...");
    gtk_widget_set_size_request(GTK_WIDGET(search_entry_), 300, -1);
    g_signal_connect(search_entry_, "changed", 
        G_CALLBACK(on_search_changed_static), this);
    g_signal_connect(search_entry_, "key-press-event",
        G_CALLBACK(on_key_press_static), this);
    gtk_box_pack_start(GTK_BOX(search_box_), GTK_WIDGET(search_entry_), FALSE, FALSE, 0);
    
    // Previous button
    prev_btn_ = GTK_BUTTON(gtk_button_new_from_icon_name("go-up-symbolic", GTK_ICON_SIZE_BUTTON));
    gtk_widget_set_tooltip_text(GTK_WIDGET(prev_btn_), "Anterior (Shift+F3)");
    g_signal_connect(prev_btn_, "clicked", G_CALLBACK(on_prev_clicked_static), this);
    gtk_box_pack_start(GTK_BOX(search_box_), GTK_WIDGET(prev_btn_), FALSE, FALSE, 0);
    
    // Next button
    next_btn_ = GTK_BUTTON(gtk_button_new_from_icon_name("go-down-symbolic", GTK_ICON_SIZE_BUTTON));
    gtk_widget_set_tooltip_text(GTK_WIDGET(next_btn_), "Siguiente (F3)");
    g_signal_connect(next_btn_, "clicked", G_CALLBACK(on_next_clicked_static), this);
    gtk_box_pack_start(GTK_BOX(search_box_), GTK_WIDGET(next_btn_), FALSE, FALSE, 0);
    
    // Case sensitive checkbox
    case_sensitive_ = GTK_CHECK_BUTTON(gtk_check_button_new_with_label("Aa"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(case_sensitive_), "Distinguir mayúsculas");
    g_signal_connect(case_sensitive_, "toggled", G_CALLBACK(on_search_changed_static), this);
    gtk_box_pack_start(GTK_BOX(search_box_), GTK_WIDGET(case_sensitive_), FALSE, FALSE, 0);
    
    // Regex checkbox
    regex_mode_ = GTK_CHECK_BUTTON(gtk_check_button_new_with_label(".*"));
    gtk_widget_set_tooltip_text(GTK_WIDGET(regex_mode_), "Expresión regular");
    g_signal_connect(regex_mode_, "toggled", G_CALLBACK(on_search_changed_static), this);
    gtk_box_pack_start(GTK_BOX(search_box_), GTK_WIDGET(regex_mode_), FALSE, FALSE, 0);
    
    // Close button
    close_btn_ = GTK_BUTTON(gtk_button_new_from_icon_name("window-close-symbolic", GTK_ICON_SIZE_BUTTON));
    gtk_widget_set_tooltip_text(GTK_WIDGET(close_btn_), "Cerrar (Escape)");
    g_signal_connect(close_btn_, "clicked", G_CALLBACK(on_close_clicked_static), this);
    gtk_box_pack_end(GTK_BOX(search_box_), GTK_WIDGET(close_btn_), FALSE, FALSE, 0);
    
    // Add to revealer
    gtk_container_add(GTK_CONTAINER(revealer_), search_box_);
    
    // Initially hidden
    gtk_revealer_set_reveal_child(GTK_REVEALER(revealer_), FALSE);
}

SearchBar::~SearchBar() {
    // GTK widgets are automatically cleaned up
}

void SearchBar::show() {
    gtk_revealer_set_reveal_child(GTK_REVEALER(revealer_), TRUE);
    gtk_widget_show_all(revealer_);
    focus_entry();
}

void SearchBar::hide() {
    gtk_revealer_set_reveal_child(GTK_REVEALER(revealer_), FALSE);
}

bool SearchBar::is_visible() const {
    return gtk_revealer_get_reveal_child(GTK_REVEALER(revealer_));
}

void SearchBar::set_search_callback(SearchCallback callback) {
    search_callback_ = std::move(callback);
}

void SearchBar::set_navigation_callback(NavigationCallback callback) {
    navigation_callback_ = std::move(callback);
}

void SearchBar::focus_entry() {
    gtk_widget_grab_focus(GTK_WIDGET(search_entry_));
}

void SearchBar::on_search_changed() {
    if (search_callback_) {
        std::string query = gtk_entry_get_text(search_entry_);
        bool case_sensitive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(case_sensitive_));
        bool regex = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(regex_mode_));
        search_callback_(query, case_sensitive, regex);
    }
}

void SearchBar::on_next_clicked() {
    if (navigation_callback_) {
        navigation_callback_(true); // next
    }
}

void SearchBar::on_prev_clicked() {
    if (navigation_callback_) {
        navigation_callback_(false); // previous
    }
}

void SearchBar::on_close_clicked() {
    hide();
}

// Static callbacks
void SearchBar::on_search_changed_static(GtkEntry* entry, gpointer user_data) {
    auto* self = static_cast<SearchBar*>(user_data);
    self->on_search_changed();
}

void SearchBar::on_next_clicked_static(GtkButton* button, gpointer user_data) {
    auto* self = static_cast<SearchBar*>(user_data);
    self->on_next_clicked();
}

void SearchBar::on_prev_clicked_static(GtkButton* button, gpointer user_data) {
    auto* self = static_cast<SearchBar*>(user_data);
    self->on_prev_clicked();
}

void SearchBar::on_close_clicked_static(GtkButton* button, gpointer user_data) {
    auto* self = static_cast<SearchBar*>(user_data);
    self->on_close_clicked();
}

gboolean SearchBar::on_key_press_static(GtkWidget* widget, GdkEventKey* event, gpointer user_data) {
    auto* self = static_cast<SearchBar*>(user_data);
    
    // Escape: close search
    if (event->keyval == GDK_KEY_Escape) {
        self->hide();
        return TRUE;
    }
    
    // Enter/Return: search next
    if (event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter) {
        self->on_next_clicked();
        return TRUE;
    }
    
    return FALSE;
}

} // namespace ui
} // namespace colabb
