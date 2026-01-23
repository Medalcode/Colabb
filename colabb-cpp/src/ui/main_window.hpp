#ifndef COLABB_MAIN_WINDOW_HPP
#define COLABB_MAIN_WINDOW_HPP

#include "infrastructure/terminal/vte_terminal.hpp"
#include "infrastructure/config/config_manager.hpp"
#include "infrastructure/config/profile_manager.hpp"
#include "infrastructure/context/context_service.hpp"
#include "infrastructure/i18n/translation_manager.hpp"
#include "ui/profile_dialog.hpp"
#include "application/prediction_service.hpp"
#include "application/suggestion_cache.hpp"
#include "domain/models/suggestion.hpp"
#include "ui/search_bar.hpp"
#include "ui/tab_manager.hpp"
#include <gtk/gtk.h>
#include <memory>
#include <string>
#include <optional>

namespace colabb {
namespace ui {

class MainWindow {
public:
    MainWindow();
    ~MainWindow();
    
    void show();
    
private:
    // GTK widgets
    GtkWidget* window_;
    GtkWidget* header_bar_;
    GtkWidget* vbox_;
    GtkNotebook* notebook_;
    GtkWidget* suggestion_revealer_;
    GtkWidget* suggestion_box_;
    GtkLabel* suggestion_label_;
    GtkButton* apply_button_;
    GtkImage* icon_image_;
    GtkSpinner* spinner_;
    
    // Components
    std::unique_ptr<TabManager> tab_manager_;
    std::unique_ptr<infrastructure::ConfigManager> config_manager_;
    std::unique_ptr<infrastructure::ProfileManager> profile_manager_;
    std::unique_ptr<infrastructure::ContextService> context_service_;
    std::unique_ptr<application::PredictionService> prediction_service_;
    std::unique_ptr<application::SuggestionCache> suggestion_cache_;
    std::unique_ptr<SearchBar> search_bar_;
    
    // State
    std::string input_buffer_;
    std::optional<domain::Suggestion> current_suggestion_;
    std::string last_query_;
    bool is_predicting_;
    guint debounce_timer_id_;
    
    // UI setup
    void setup_ui();
    void setup_header_bar();
    void setup_hamburger_menu();
    void setup_search_button();
    void setup_suggestion_overlay();
    void load_css();
    
    // Suggestion overlay control
    void show_suggestion_overlay();
    void hide_suggestion_overlay();
    
    // Event handlers
    bool on_key_press(GdkEventKey* event);
    void on_apply_suggestion();
    void on_config_clicked();
    void on_escape_pressed();
    
    // Prediction handling
    void process_input_buffer();
    void on_prediction_result(std::optional<domain::Suggestion> suggestion);
    
    // Static callbacks for GTK
    static void on_config_clicked_static(GtkButton* button, gpointer user_data);
    static void on_apply_suggestion_static(GtkButton* button, gpointer user_data);
    static void on_destroy_static(GtkWidget* widget, gpointer user_data);
    static void on_new_window_static(GtkMenuItem* item, gpointer user_data);
    static void on_about_clicked_static(GtkMenuItem* item, gpointer user_data);
    static void on_report_issue_clicked_static(GtkMenuItem* item, gpointer user_data);
    static void on_search_clicked_static(GtkButton* button, gpointer user_data);
    static void on_new_tab_static(GtkButton* button, gpointer user_data);
    static void on_profiles_clicked_static(GtkMenuItem* item, gpointer user_data);
    static void on_explain_error_clicked_static(GtkMenuItem* item, gpointer user_data);
    
    // Menu handlers
    void on_new_window();
    void on_about_clicked();
    void on_report_issue_clicked();
    void on_search_clicked();
    void on_profiles_clicked();
    void on_explain_error();
    
    // Tab handlers
    void on_new_tab();
    void on_close_tab();
    void on_tab_created(TabManager::TabInfo* tab);
    void on_tab_closed(int index);
    void on_tab_switched(GtkWidget* page, guint page_num);
    
    // Search handlers
    void toggle_search();
    void on_search_query(const std::string& query, bool case_sensitive, bool regex);
    void on_search_navigate(bool next);

    // Helper methods
    void update_suggestion_ui(const std::string& text, bool enable_button);
    std::unique_ptr<domain::IAIProvider> create_ai_provider();
    infrastructure::TerminalWidget* get_current_terminal();
};

} // namespace ui
} // namespace colabb

#endif // COLABB_MAIN_WINDOW_HPP
