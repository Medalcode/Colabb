#include "ui/tab_manager.hpp"
#include <iostream>

namespace colabb {
namespace ui {

TabManager::TabManager(GtkNotebook* notebook)
    : notebook_(notebook)
    , next_tab_id_(1) {
}

TabManager::~TabManager() {
}

int TabManager::create_tab(const std::string& title) {
    auto tab = std::make_unique<TabInfo>();
    tab->title = title.empty() ? ("Terminal " + std::to_string(next_tab_id_++)) : title;
    tab->index = tabs_.size();
    
    // Create terminal
    tab->terminal = std::make_unique<infrastructure::TerminalWidget>();
    
    // Apply profile
    if (settings_manager_) {
        std::string def_name = settings_manager_->get_default_profile_name();
        auto profile = settings_manager_->get_profile(def_name);
        tab->terminal->apply_profile(profile);
    }
    
    // Create tab content
    GtkWidget* content = create_tab_content(tab.get());
    
    // Create tab label
    GtkWidget* label = create_tab_label(tab->title, tab->index);
    
    // Add to notebook
    int page_num = gtk_notebook_append_page(notebook_, content, label);
    gtk_notebook_set_tab_reorderable(notebook_, content, TRUE);
    
    // Store tab info BEFORE switching page so callbacks can find it
    TabInfo* raw_ptr = tab.get();
    tabs_.push_back(std::move(tab));
    
    // Switch to new tab
    gtk_notebook_set_current_page(notebook_, page_num);
    
    // Spawn shell
    const char* shell = getenv("SHELL");
    if (!shell) shell = "/bin/bash";
    raw_ptr->terminal->spawn_shell(shell);
    
    // Callback
    if (tab_created_callback_) {
        tab_created_callback_(raw_ptr);
    }
    
    gtk_widget_show_all(content);
    return page_num;
}

void TabManager::close_tab(int index) {
    if (index < 0 || index >= static_cast<int>(tabs_.size())) return;
    if (tabs_.size() == 1) return;
    
    gtk_notebook_remove_page(notebook_, index);
    tabs_.erase(tabs_.begin() + index);
    
    for (size_t i = index; i < tabs_.size(); ++i) {
        tabs_[i]->index = i;
    }
    
    if (tab_closed_callback_) tab_closed_callback_(index);
}

void TabManager::close_current_tab() {
    close_tab(gtk_notebook_get_current_page(notebook_));
}

TabManager::TabInfo* TabManager::get_tab(int index) {
    if (index < 0 || index >= static_cast<int>(tabs_.size())) return nullptr;
    return tabs_[index].get();
}

TabManager::TabInfo* TabManager::get_current_tab() {
    return get_tab(gtk_notebook_get_current_page(notebook_));
}

int TabManager::get_current_tab_index() {
    return gtk_notebook_get_current_page(notebook_);
}

int TabManager::get_tab_count() {
    return tabs_.size();
}

void TabManager::next_tab() {
    int current = gtk_notebook_get_current_page(notebook_);
    int total = gtk_notebook_get_n_pages(notebook_);
    gtk_notebook_set_current_page(notebook_, (current + 1) % total);
}

void TabManager::previous_tab() {
    int current = gtk_notebook_get_current_page(notebook_);
    int total = gtk_notebook_get_n_pages(notebook_);
    gtk_notebook_set_current_page(notebook_, (current - 1 + total) % total);
}

void TabManager::switch_to_tab(int index) {
    gtk_notebook_set_current_page(notebook_, index);
}

void TabManager::refresh_all_tabs() {
    if (!settings_manager_) return;
    std::string def_name = settings_manager_->get_default_profile_name();
    auto profile = settings_manager_->get_profile(def_name);
    for (auto& tab : tabs_) {
        tab->terminal->apply_profile(profile);
    }
}

void TabManager::set_tab_created_callback(TabCreatedCallback callback) {
    tab_created_callback_ = std::move(callback);
}

void TabManager::set_tab_closed_callback(TabClosedCallback callback) {
    tab_closed_callback_ = std::move(callback);
}

void TabManager::set_settings_manager(infrastructure::SettingsManager* manager) {
    settings_manager_ = manager;
}

GtkWidget* TabManager::create_tab_label(const std::string& title, int index) {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    GtkWidget* label = gtk_label_new(title.c_str());
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    
    GtkWidget* close_btn = gtk_button_new_from_icon_name("window-close-symbolic", GTK_ICON_SIZE_MENU);
    gtk_button_set_relief(GTK_BUTTON(close_btn), GTK_RELIEF_NONE);
    gtk_widget_set_focus_on_click(close_btn, FALSE);
    
    g_signal_connect(close_btn, "clicked", G_CALLBACK(on_tab_close_clicked_static), this);
    gtk_box_pack_end(GTK_BOX(box), close_btn, FALSE, FALSE, 0);
    
    gtk_widget_show_all(box);
    return box;
}

GtkWidget* TabManager::create_tab_content(TabInfo* tab) {
    tab->overlay = gtk_overlay_new();
    tab->scrolled_window = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_container_add(GTK_CONTAINER(tab->scrolled_window), tab->terminal->widget());
    gtk_container_add(GTK_CONTAINER(tab->overlay), tab->scrolled_window);
    return tab->overlay;
}

void TabManager::on_tab_close_clicked_static(GtkButton* button, gpointer user_data) {
    auto* self = static_cast<TabManager*>(user_data);
    GtkWidget* box = gtk_widget_get_parent(GTK_WIDGET(button));
    if (!box) return;

    int n_pages = gtk_notebook_get_n_pages(self->notebook_);
    for (int i = 0; i < n_pages; i++) {
        GtkWidget* page = gtk_notebook_get_nth_page(self->notebook_, i);
        if (gtk_notebook_get_tab_label(self->notebook_, page) == box) {
            self->on_tab_close_clicked(i);
            return;
        }
    }
}

void TabManager::on_tab_close_clicked(int index) {
    close_tab(index);
}

} // namespace ui
} // namespace colabb
