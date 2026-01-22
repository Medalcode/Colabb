#ifndef COLABB_CONFIG_DIALOG_HPP
#define COLABB_CONFIG_DIALOG_HPP

#include "infrastructure/config/config_manager.hpp"
#include <gtk/gtk.h>

namespace colabb {
namespace ui {

class ConfigDialog {
public:
    ConfigDialog(GtkWindow* parent, infrastructure::ConfigManager* config_manager);
    ~ConfigDialog();
    
    void run();
    
private:
    GtkWidget* dialog_;
    GtkComboBoxText* provider_combo_;
    GtkEntry* api_key_entry_;
    infrastructure::ConfigManager* config_manager_;
    
    void setup_ui(GtkWindow* parent);
    void load_current_config();
    void on_save();
    void show_message(const std::string& title, const std::string& message);
    
    // Static callbacks
    static void on_save_static(GtkButton* button, gpointer user_data);
};

} // namespace ui
} // namespace colabb

#endif // COLABB_CONFIG_DIALOG_HPP
