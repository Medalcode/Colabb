#include "ui/config_dialog.hpp"
#include "domain/ai/ai_provider.hpp"
#include <iostream>

namespace colabb {
namespace ui {

ConfigDialog::ConfigDialog(GtkWindow* parent, infrastructure::ConfigManager* config_manager)
    : dialog_(nullptr)
    , provider_combo_(nullptr)
    , api_key_entry_(nullptr)
    , config_manager_(config_manager) {
    
    setup_ui(parent);
}

ConfigDialog::~ConfigDialog() {
    if (dialog_) {
        gtk_widget_destroy(dialog_);
    }
}

void ConfigDialog::setup_ui(GtkWindow* parent) {
    dialog_ = gtk_dialog_new_with_buttons(
        "Configuración IA",
        parent,
        GTK_DIALOG_MODAL,
        "Cancelar", GTK_RESPONSE_CANCEL,
        nullptr
    );
    
    gtk_window_set_default_size(GTK_WINDOW(dialog_), 400, 200);
    
    GtkWidget* content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog_));
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 10);
    
    // Grid layout
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_container_add(GTK_CONTAINER(content_area), grid);
    
    // Provider combo
    GtkWidget* provider_label = gtk_label_new("Proveedor:");
    gtk_widget_set_halign(provider_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), provider_label, 0, 0, 1, 1);
    
    provider_combo_ = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    gtk_combo_box_text_append_text(provider_combo_, "groq");
    gtk_combo_box_text_append_text(provider_combo_, "openai");
    gtk_combo_box_text_append_text(provider_combo_, "local");
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(provider_combo_), 1, 0, 2, 1);
    
    // API Key entry
    GtkWidget* api_key_label = gtk_label_new("API Key:");
    gtk_widget_set_halign(api_key_label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), api_key_label, 0, 1, 1, 1);
    
    api_key_entry_ = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_placeholder_text(api_key_entry_, "Pegar API Key aquí...");
    gtk_entry_set_visibility(api_key_entry_, FALSE);
    gtk_entry_set_input_purpose(api_key_entry_, GTK_INPUT_PURPOSE_PASSWORD);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(api_key_entry_), 1, 1, 2, 1);
    
    // Save button
    GtkWidget* save_button = gtk_button_new_with_label("Guardar y Validar");
    gtk_style_context_add_class(gtk_widget_get_style_context(save_button), "suggested-action");
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_static), this);
    gtk_grid_attach(GTK_GRID(grid), save_button, 0, 2, 3, 1);
    
    load_current_config();
    gtk_widget_show_all(content_area);
}

void ConfigDialog::load_current_config() {
    std::string provider = config_manager_->get_provider();
    
    if (provider == "groq") {
        gtk_combo_box_set_active(GTK_COMBO_BOX(provider_combo_), 0);
    } else if (provider == "openai") {
        gtk_combo_box_set_active(GTK_COMBO_BOX(provider_combo_), 1);
    } else {
        gtk_combo_box_set_active(GTK_COMBO_BOX(provider_combo_), 2);
    }
}

void ConfigDialog::run() {
    gtk_dialog_run(GTK_DIALOG(dialog_));
}

void ConfigDialog::on_save() {
    const char* provider_text = gtk_combo_box_text_get_active_text(provider_combo_);
    if (!provider_text) {
        return;
    }
    
    std::string provider(provider_text);
    const char* api_key_text = gtk_entry_get_text(api_key_entry_);
    
    // Save provider
    config_manager_->set_provider(provider);
    
    // Save API key if provided
    if (api_key_text && strlen(api_key_text) > 0) {
        std::string api_key(api_key_text);
        config_manager_->set_api_key(provider, api_key);
        
        // Validate connection
        std::unique_ptr<domain::IAIProvider> ai_provider;
        if (provider == "openai") {
            ai_provider = std::make_unique<domain::OpenAIProvider>(api_key);
        } else if (provider == "groq") {
            ai_provider = std::make_unique<domain::GroqProvider>(api_key);
        }
        
        if (ai_provider) {
            bool valid = ai_provider->validate_connection();
            if (valid) {
                show_message("✅ Conexión Exitosa", "La API Key es válida y se ha guardado.");
            } else {
                show_message("❌ Error de Validación", "No se pudo conectar con el proveedor.");
            }
        }
    } else {
        show_message("Configuración Guardada", "Se guardó el proveedor (sin cambiar API Key).");
    }
    
    gtk_dialog_response(GTK_DIALOG(dialog_), GTK_RESPONSE_OK);
}

void ConfigDialog::show_message(const std::string& title, const std::string& message) {
    GtkWidget* msg_dialog = gtk_message_dialog_new(
        GTK_WINDOW(dialog_),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s", title.c_str()
    );
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(msg_dialog), "%s", message.c_str());
    gtk_dialog_run(GTK_DIALOG(msg_dialog));
    gtk_widget_destroy(msg_dialog);
}

void ConfigDialog::on_save_static(GtkButton* button, gpointer user_data) {
    auto* self = static_cast<ConfigDialog*>(user_data);
    self->on_save();
}

} // namespace ui
} // namespace colabb
