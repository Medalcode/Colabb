#ifndef COLABB_MAIN_WINDOW_HPP
#define COLABB_MAIN_WINDOW_HPP

#include "infrastructure/terminal/vte_terminal.hpp"
#include "infrastructure/config/config_manager.hpp"
#include "application/prediction_service.hpp"
#include "domain/models/suggestion.hpp"
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
    GtkWidget* suggestion_bar_;
    GtkLabel* suggestion_label_;
    GtkButton* apply_button_;
    GtkImage* icon_image_;
    
    // Components
    std::unique_ptr<infrastructure::TerminalWidget> terminal_;
    std::unique_ptr<infrastructure::ConfigManager> config_manager_;
    std::unique_ptr<application::PredictionService> prediction_service_;
    
    // State
    std::string input_buffer_;
    std::optional<domain::Suggestion> current_suggestion_;
    std::string last_query_;
    bool is_predicting_;
    
    // UI setup
    void setup_ui();
    void setup_header_bar();
    void setup_suggestion_bar();
    void load_css();
    
    // Event handlers
    void on_key_press(GdkEventKey* event);
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
    
    // Helper methods
    void update_suggestion_ui(const std::string& text, bool enable_button);
    std::unique_ptr<domain::IAIProvider> create_ai_provider();
};

} // namespace ui
} // namespace colabb

#endif // COLABB_MAIN_WINDOW_HPP
