#include "ui/main_window.hpp"
#include "ui/config_dialog.hpp"
#include "domain/ai/ai_provider.hpp"
#include <iostream>

namespace colabb {
namespace ui {

MainWindow::MainWindow()
    : window_(nullptr)
    , header_bar_(nullptr)
    , vbox_(nullptr)
    , suggestion_bar_(nullptr)
    , suggestion_label_(nullptr)
    , apply_button_(nullptr)
    , icon_image_(nullptr)
    , is_predicting_(false) {
    
    config_manager_ = std::make_unique<infrastructure::ConfigManager>();
    terminal_ = std::make_unique<infrastructure::TerminalWidget>();
    
    // Create AI provider based on config
    auto ai_provider = create_ai_provider();
    prediction_service_ = std::make_unique<application::PredictionService>(std::move(ai_provider));
    
    setup_ui();
}

MainWindow::~MainWindow() {
    // GTK widgets are automatically cleaned up
}

void MainWindow::show() {
    gtk_widget_show_all(window_);
}

void MainWindow::setup_ui() {
    // Create window
    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window_), 950, 650);
    g_signal_connect(window_, "destroy", G_CALLBACK(on_destroy_static), this);
    
    // Setup header bar
    setup_header_bar();
    
    // Create vertical box
    vbox_ = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window_), vbox_);
    
    // Add terminal with scrolled window
    GtkWidget* scrolled = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_container_add(GTK_CONTAINER(scrolled), terminal_->widget());
    gtk_box_pack_start(GTK_BOX(vbox_), scrolled, TRUE, TRUE, 0);
    
    // Setup suggestion bar
    setup_suggestion_bar();
    
    // Load CSS
    load_css();
    
    // Spawn shell
    const char* shell = getenv("SHELL");
    if (!shell) shell = "/bin/bash";
    terminal_->spawn_shell(shell);
    
    // Set up terminal key press callback
    terminal_->set_key_press_callback([this](GdkEventKey* event) {
        this->on_key_press(event);
    });
    
    // Focus terminal
    gtk_widget_grab_focus(terminal_->widget());
}

void MainWindow::setup_header_bar() {
    header_bar_ = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar_), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar_), "Colabb Terminal");
    gtk_window_set_titlebar(GTK_WINDOW(window_), header_bar_);
    
    // Config button
    GtkWidget* config_btn = gtk_button_new_from_icon_name("emblem-system-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(config_btn, "Configuración IA");
    g_signal_connect(config_btn, "clicked", G_CALLBACK(on_config_clicked_static), this);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar_), config_btn);
}

void MainWindow::setup_suggestion_bar() {
    suggestion_bar_ = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(suggestion_bar_), "suggestion-bar");
    
    // Icon
    icon_image_ = GTK_IMAGE(gtk_image_new_from_icon_name("dialog-idea-symbolic", GTK_ICON_SIZE_MENU));
    gtk_box_pack_start(GTK_BOX(suggestion_bar_), GTK_WIDGET(icon_image_), FALSE, FALSE, 5);
    
    // Label
    suggestion_label_ = GTK_LABEL(gtk_label_new("Escribe '?' y espera la sugerencia..."));
    gtk_label_set_ellipsize(suggestion_label_, PANGO_ELLIPSIZE_END);
    gtk_label_set_xalign(suggestion_label_, 0.0);
    gtk_box_pack_start(GTK_BOX(suggestion_bar_), GTK_WIDGET(suggestion_label_), TRUE, TRUE, 0);
    
    // Apply button
    apply_button_ = GTK_BUTTON(gtk_button_new_with_label("Aplicar (Ctrl+Space)"));
    gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(apply_button_)), "suggest-btn");
    gtk_widget_set_sensitive(GTK_WIDGET(apply_button_), FALSE);
    g_signal_connect(apply_button_, "clicked", G_CALLBACK(on_apply_suggestion_static), this);
    gtk_box_pack_end(GTK_BOX(suggestion_bar_), GTK_WIDGET(apply_button_), FALSE, FALSE, 5);
    
    gtk_box_pack_end(GTK_BOX(vbox_), suggestion_bar_, FALSE, FALSE, 0);
}

void MainWindow::load_css() {
    const char* css_data = R"(
        .suggestion-bar {
            background-color: #252526;
            color: #eeeeee;
            padding: 6px 12px;
            border-top: 1px solid #3e3e42;
        }
        .suggest-btn {
            background-color: #0e639c;
            color: white;
            border-radius: 4px;
            font-weight: bold;
        }
        .suggest-btn:disabled {
            background-color: #3e3e42;
            color: #888;
        }
    )";
    
    GtkCssProvider* provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css_data, -1, nullptr);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

void MainWindow::on_key_press(GdkEventKey* event) {
    // Ctrl+Space: Apply suggestion
    if ((event->state & GDK_CONTROL_MASK) && event->keyval == GDK_KEY_space) {
        on_apply_suggestion();
        return;
    }
    
    // Escape: Reset
    if (event->keyval == GDK_KEY_Escape) {
        on_escape_pressed();
        return;
    }
    
    // Enter: Clear buffer
    if (event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter) {
        input_buffer_.clear();
        update_suggestion_ui("Escribe '?' y espera la sugerencia...", false);
        return;
    }
    
    // Printable characters
    if (event->keyval < 256 && g_unichar_isprint(event->keyval)) {
        input_buffer_ += static_cast<char>(event->keyval);
    }
    
    // Backspace
    else if (event->keyval == GDK_KEY_BackSpace && !input_buffer_.empty()) {
        input_buffer_.pop_back();
    }
    
    // Ctrl+C, Ctrl+U, Ctrl+L: Clear buffer
    else if (event->state & GDK_CONTROL_MASK) {
        if (event->keyval == GDK_KEY_c || event->keyval == GDK_KEY_u || event->keyval == GDK_KEY_l) {
            input_buffer_.clear();
        }
    }
    
    // Process input after a short delay (debounce)
    g_timeout_add(300, [](gpointer user_data) -> gboolean {
        auto* self = static_cast<MainWindow*>(user_data);
        self->process_input_buffer();
        return G_SOURCE_REMOVE;
    }, this);
}

void MainWindow::process_input_buffer() {
    // Check for totem '?'
    size_t totem_pos = input_buffer_.find('?');
    if (totem_pos == std::string::npos) {
        update_suggestion_ui("Escribe '?' y espera la sugerencia...", false);
        return;
    }
    
    // Extract query after last '?'
    std::string query = input_buffer_.substr(totem_pos + 1);
    
    // Trim whitespace
    query.erase(0, query.find_first_not_of(" \t"));
    query.erase(query.find_last_not_of(" \t") + 1);
    
    if (query.empty() || query == last_query_) {
        return;
    }
    
    last_query_ = query;
    
    if (!is_predicting_) {
        is_predicting_ = true;
        update_suggestion_ui("Consultando IA para: " + query + "...", false);
        
        // Get context from terminal
        std::string context = terminal_->get_context(20);
        
        // Request prediction
        prediction_service_->predict_async(query, context,
            [this](std::optional<domain::Suggestion> suggestion) {
                // This callback runs in worker thread, use g_idle_add for UI update
                g_idle_add([](gpointer user_data) -> gboolean {
                    auto* pair = static_cast<std::pair<MainWindow*, std::optional<domain::Suggestion>>*>(user_data);
                    pair->first->on_prediction_result(pair->second);
                    delete pair;
                    return G_SOURCE_REMOVE;
                }, new std::pair<MainWindow*, std::optional<domain::Suggestion>>(this, suggestion));
            });
    }
}

void MainWindow::on_prediction_result(std::optional<domain::Suggestion> suggestion) {
    is_predicting_ = false;
    
    if (suggestion) {
        current_suggestion_ = suggestion;
        update_suggestion_ui("Sugerencia: " + suggestion->command + " (Ctrl+Space)", true);
        gtk_image_set_from_icon_name(icon_image_, "emoji-objects-symbolic", GTK_ICON_SIZE_MENU);
    } else {
        current_suggestion_ = std::nullopt;
        update_suggestion_ui("Sin sugerencias", false);
        gtk_image_set_from_icon_name(icon_image_, "dialog-idea-symbolic", GTK_ICON_SIZE_MENU);
    }
}

void MainWindow::on_apply_suggestion() {
    if (!current_suggestion_) {
        return;
    }
    
    // Clear current line and feed suggestion
    terminal_->clear_line();
    terminal_->feed_text(current_suggestion_->command);
    
    // Reset state
    input_buffer_.clear();
    last_query_.clear();
    current_suggestion_ = std::nullopt;
    update_suggestion_ui("Aplicado. Presiona Enter para ejecutar.", false);
    
    // Focus terminal
    gtk_widget_grab_focus(terminal_->widget());
}

void MainWindow::on_config_clicked() {
    ConfigDialog dialog(GTK_WINDOW(window_), config_manager_.get());
    dialog.run();
    
    // Recreate AI provider with new config
    auto ai_provider = create_ai_provider();
    prediction_service_ = std::make_unique<application::PredictionService>(std::move(ai_provider));
}

void MainWindow::on_escape_pressed() {
    input_buffer_.clear();
    last_query_.clear();
    current_suggestion_ = std::nullopt;
    update_suggestion_ui("IA Reseteada. Escribe '?'...", false);
}

void MainWindow::update_suggestion_ui(const std::string& text, bool enable_button) {
    gtk_label_set_text(suggestion_label_, text.c_str());
    gtk_widget_set_sensitive(GTK_WIDGET(apply_button_), enable_button);
}

std::unique_ptr<domain::IAIProvider> MainWindow::create_ai_provider() {
    std::string provider = config_manager_->get_provider();
    std::string api_key = config_manager_->get_api_key(provider);
    
    if (provider == "openai") {
        return std::make_unique<domain::OpenAIProvider>(api_key);
    } else {
        // Default to Groq
        return std::make_unique<domain::GroqProvider>(api_key);
    }
}

// Static callbacks
void MainWindow::on_config_clicked_static(GtkButton* button, gpointer user_data) {
    auto* self = static_cast<MainWindow*>(user_data);
    self->on_config_clicked();
}

void MainWindow::on_apply_suggestion_static(GtkButton* button, gpointer user_data) {
    auto* self = static_cast<MainWindow*>(user_data);
    self->on_apply_suggestion();
}

void MainWindow::on_destroy_static(GtkWidget* widget, gpointer user_data) {
    gtk_main_quit();
}

} // namespace ui
} // namespace colabb
