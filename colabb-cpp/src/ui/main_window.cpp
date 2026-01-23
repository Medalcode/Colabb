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
    , notebook_(nullptr)
    , suggestion_revealer_(nullptr)
    , suggestion_box_(nullptr)
    , suggestion_label_(nullptr)
    , apply_button_(nullptr)
    , icon_image_(nullptr)
    , is_predicting_(false)
    , debounce_timer_id_(0) {
    
    config_manager_ = std::make_unique<infrastructure::ConfigManager>();
    profile_manager_ = std::make_unique<infrastructure::ProfileManager>();
    context_service_ = std::make_unique<infrastructure::ContextService>();
    
    // Create AI provider based on config
    auto ai_provider = create_ai_provider();
    prediction_service_ = std::make_unique<application::PredictionService>(std::move(ai_provider));
    
    // Create suggestion cache
    suggestion_cache_ = std::make_unique<application::SuggestionCache>();
    
    // Create search bar
    search_bar_ = std::make_unique<SearchBar>(nullptr);
    search_bar_->set_search_callback([this](const std::string& query, bool cs, bool regex) {
        this->on_search_query(query, cs, regex);
    });
    search_bar_->set_navigation_callback([this](bool next) {
        this->on_search_navigate(next);
    });
    
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
    
    // Add search bar (hidden by default)
    gtk_box_pack_start(GTK_BOX(vbox_), search_bar_->widget(), FALSE, FALSE, 0);
    
    // Create notebook for tabs
    notebook_ = GTK_NOTEBOOK(gtk_notebook_new());
    gtk_notebook_set_scrollable(notebook_, TRUE);
    gtk_notebook_set_show_border(notebook_, FALSE);
    
    // Create TabManager
    tab_manager_ = std::make_unique<TabManager>(notebook_);
    tab_manager_->set_profile_manager(profile_manager_.get());
    tab_manager_->set_tab_created_callback([this](TabManager::TabInfo* tab) {
        this->on_tab_created(tab);
    });
    tab_manager_->set_tab_closed_callback([this](int index) {
        this->on_tab_closed(index);
    });
    
    // Setup suggestion overlay (will be added to each tab)
    setup_suggestion_overlay();
    
    // Add notebook to vbox
    gtk_box_pack_start(GTK_BOX(vbox_), GTK_WIDGET(notebook_), TRUE, TRUE, 0);
    
    // Load CSS
    load_css();
    
    // Create first tab
    tab_manager_->create_tab("Terminal");
}

void MainWindow::setup_header_bar() {
    header_bar_ = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar_), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar_), "Terminal");
    gtk_window_set_titlebar(GTK_WINDOW(window_), header_bar_);
    
    // Add hamburger menu and search button
    setup_search_button();
    setup_hamburger_menu();
    
    // Add New Tab button (left side)
    GtkWidget* new_tab_btn = gtk_button_new_from_icon_name("tab-new-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(new_tab_btn, 
        infrastructure::TranslationManager::instance().get("tab.new_tooltip").c_str());
    g_signal_connect(new_tab_btn, "clicked", G_CALLBACK(on_new_tab_static), this);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar_), new_tab_btn);
}

void MainWindow::setup_hamburger_menu() {
    GtkWidget* menu_button = gtk_menu_button_new();
    gtk_button_set_image(GTK_BUTTON(menu_button),
        gtk_image_new_from_icon_name("open-menu-symbolic", GTK_ICON_SIZE_BUTTON));
    gtk_widget_set_tooltip_text(menu_button, 
        infrastructure::TranslationManager::instance().get("menu.main_tooltip").c_str());
    
    GtkWidget* menu = gtk_menu_new();
    
    // Nueva ventana
    GtkWidget* new_window = gtk_menu_item_new_with_label(
        infrastructure::TranslationManager::instance().get("menu.new_window").c_str());
    g_signal_connect(new_window, "activate", 
        G_CALLBACK(on_new_window_static), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), new_window);
    
    // Separator
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), 
        gtk_separator_menu_item_new());
    
    // Preferencias (AI Config)
    GtkWidget* prefs = gtk_menu_item_new_with_label(
        infrastructure::TranslationManager::instance().get("menu.preferences").c_str());
    g_signal_connect(prefs, "activate", 
        G_CALLBACK(on_config_clicked_static), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), prefs);

    // Perfiles
    GtkWidget* profiles = gtk_menu_item_new_with_label(
        infrastructure::TranslationManager::instance().get("menu.profiles").c_str());
    g_signal_connect(profiles, "activate",
        G_CALLBACK(on_profiles_clicked_static), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), profiles);

    // Separator
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), 
        gtk_separator_menu_item_new());

    // Explicar Error
    std::string explain_label = infrastructure::TranslationManager::instance().get("menu.explain_error") + " (Ctrl+Alt+E)";
    GtkWidget* explain = gtk_menu_item_new_with_label(explain_label.c_str());
    g_signal_connect(explain, "activate",
        G_CALLBACK(on_explain_error_clicked_static), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), explain);
    
    // Acerca de
    GtkWidget* about = gtk_menu_item_new_with_label(
        infrastructure::TranslationManager::instance().get("menu.about").c_str());
    g_signal_connect(about, "activate", 
        G_CALLBACK(on_about_clicked_static), this);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), about);
    
    gtk_menu_button_set_popup(GTK_MENU_BUTTON(menu_button), menu);
    gtk_widget_show_all(menu);
    
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar_), menu_button);
}

void MainWindow::setup_search_button() {
    GtkWidget* search_button = gtk_button_new_from_icon_name(
        "edit-find-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(search_button, 
        infrastructure::TranslationManager::instance().get("search.tooltip").c_str());
    
    g_signal_connect(search_button, "clicked", 
        G_CALLBACK(on_search_clicked_static), this);
    
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar_), search_button);
}

void MainWindow::setup_suggestion_overlay() {
    // Create revealer for smooth show/hide animation
    suggestion_revealer_ = gtk_revealer_new();
    gtk_revealer_set_transition_type(GTK_REVEALER(suggestion_revealer_),
        GTK_REVEALER_TRANSITION_TYPE_SLIDE_UP);
    gtk_revealer_set_transition_duration(GTK_REVEALER(suggestion_revealer_), 200);
    
    // Create suggestion box
    suggestion_box_ = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_style_context_add_class(
        gtk_widget_get_style_context(suggestion_box_), 
        "suggestion-overlay");
    gtk_widget_set_margin_start(suggestion_box_, 10);
    gtk_widget_set_margin_end(suggestion_box_, 10);
    gtk_widget_set_margin_bottom(suggestion_box_, 10);
    
    // Icon
    icon_image_ = GTK_IMAGE(gtk_image_new_from_icon_name("dialog-idea-symbolic", GTK_ICON_SIZE_MENU));
    gtk_box_pack_start(GTK_BOX(suggestion_box_), GTK_WIDGET(icon_image_), FALSE, FALSE, 5);

    // Spinner
    spinner_ = GTK_SPINNER(gtk_spinner_new());
    gtk_box_pack_start(GTK_BOX(suggestion_box_), GTK_WIDGET(spinner_), FALSE, FALSE, 5);
    
    // Label
    suggestion_label_ = GTK_LABEL(gtk_label_new(""));
    gtk_label_set_ellipsize(suggestion_label_, PANGO_ELLIPSIZE_END);
    gtk_label_set_xalign(suggestion_label_, 0.0);
    gtk_box_pack_start(GTK_BOX(suggestion_box_), GTK_WIDGET(suggestion_label_), TRUE, TRUE, 0);
    
    // Apply button
    std::string apply_label = infrastructure::TranslationManager::instance().get("suggestion.apply") + " (Ctrl+Space)";
    apply_button_ = GTK_BUTTON(gtk_button_new_with_label(apply_label.c_str()));
    gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(apply_button_)), "suggest-btn");
    gtk_widget_set_sensitive(GTK_WIDGET(apply_button_), FALSE);
    g_signal_connect(apply_button_, "clicked", G_CALLBACK(on_apply_suggestion_static), this);
    gtk_box_pack_end(GTK_BOX(suggestion_box_), GTK_WIDGET(apply_button_), FALSE, FALSE, 5);
    
    // Add to revealer
    gtk_container_add(GTK_CONTAINER(suggestion_revealer_), suggestion_box_);
    
    // Add to overlay at bottom-center
    // gtk_overlay_add_overlay(GTK_OVERLAY(overlay_), suggestion_revealer_); // Removed as per instruction
    gtk_widget_set_halign(suggestion_revealer_, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(suggestion_revealer_, GTK_ALIGN_END);
    
    // Initially hidden
    gtk_revealer_set_reveal_child(GTK_REVEALER(suggestion_revealer_), FALSE);
}

void MainWindow::load_css() {
    const char* css_data = R"(
        .suggestion-overlay {
            background-color: alpha(@theme_bg_color, 0.95);
            border: 1px solid @borders;
            border-radius: 8px;
            padding: 10px 14px;
            box-shadow: 0 2px 8px rgba(0,0,0,0.2);
        }
        .suggest-btn {
            background-color: @theme_selected_bg_color;
            color: @theme_selected_fg_color;
            border-radius: 5px;
            padding: 6px 12px;
            font-weight: 500;
        }
        .suggest-btn:disabled {
            opacity: 0.5;
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

void MainWindow::show_suggestion_overlay() {
    gtk_revealer_set_reveal_child(GTK_REVEALER(suggestion_revealer_), TRUE);
}

void MainWindow::hide_suggestion_overlay() {
    gtk_revealer_set_reveal_child(GTK_REVEALER(suggestion_revealer_), FALSE);
}

bool MainWindow::on_key_press(GdkEventKey* event) {
    // Ctrl+Shift+F: Toggle search
    if ((event->state & GDK_CONTROL_MASK) && 
        (event->state & GDK_SHIFT_MASK) && 
        event->keyval == GDK_KEY_f) {
        toggle_search();
        return true;
    }
    
    // F3: Search navigation
    if (event->keyval == GDK_KEY_F3) {
        if (event->state & GDK_SHIFT_MASK) {
            on_search_navigate(false); // previous
        } else {
            on_search_navigate(true); // next
        }
        return true;
    }
    
    // Ctrl+Shift+T: New tab
    if ((event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) &&
        event->keyval == GDK_KEY_t) {
        on_new_tab();
        return true;
    }
    
    // Ctrl+Shift+W: Close tab
    if ((event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) &&
        event->keyval == GDK_KEY_w) {
        on_close_tab();
        return true;
    }
    
    // Ctrl+PageUp/PageDown: Navigate tabs
    if (event->state & GDK_CONTROL_MASK) {
        if (event->keyval == GDK_KEY_Page_Up) {
            tab_manager_->previous_tab();
            return true;
        } else if (event->keyval == GDK_KEY_Page_Down) {
            tab_manager_->next_tab();
            return true;
        }
    }
    
    // Ctrl+Space: Apply suggestion
    if ((event->state & GDK_CONTROL_MASK) && event->keyval == GDK_KEY_space) {
        on_apply_suggestion();
        return true;
    }
    
    // Tab: Apply suggestion if active
    if (event->keyval == GDK_KEY_Tab) {
        if (current_suggestion_) {
            on_apply_suggestion();
            return true;
        }
    }
    
    // Ctrl+Alt+E: Explain Error
    if ((event->state & GDK_CONTROL_MASK) && (event->state & GDK_MOD1_MASK) && event->keyval == GDK_KEY_e) {
        on_explain_error();
        return true;
    }
    
    // Escape: Reset
    if (event->keyval == GDK_KEY_Escape) {
        on_escape_pressed();
        return true;
    }
    
    // Enter: Clear buffer
    if (event->keyval == GDK_KEY_Return || event->keyval == GDK_KEY_KP_Enter) {
        input_buffer_.clear();
        update_suggestion_ui("Escribe '?' y espera la sugerencia...", false);
        return false; // Don't consume enter, let terminal handle it
    }
    
    // Capture input for prediction
    if (!event->state || (event->state & GDK_SHIFT_MASK)) {
        // Printable characters
        if (event->keyval < 256 && g_unichar_isprint(event->keyval)) {
            input_buffer_ += static_cast<char>(event->keyval);
        }
        // Backspace
        else if (event->keyval == GDK_KEY_BackSpace && !input_buffer_.empty()) {
            input_buffer_.pop_back();
        }
    }
    
    // Ctrl+C, Ctrl+U, Ctrl+L: Clear buffer
    if (event->state & GDK_CONTROL_MASK) {
        if (event->keyval == GDK_KEY_c || event->keyval == GDK_KEY_u || event->keyval == GDK_KEY_l) {
            input_buffer_.clear();
        }
    }
    
    // Process input after adaptive delay (debounce)
    // Cancel previous timer
    if (debounce_timer_id_ > 0) {
        g_source_remove(debounce_timer_id_);
        debounce_timer_id_ = 0;
    }
    
    // Adaptive delay: shorter for longer queries
    int delay = input_buffer_.length() > 10 ? 200 : 350;
    
    debounce_timer_id_ = g_timeout_add(delay, [](gpointer user_data) -> gboolean {
        auto* self = static_cast<MainWindow*>(user_data);
        self->process_input_buffer();
        self->debounce_timer_id_ = 0;
        return G_SOURCE_REMOVE;
    }, this);
    
    return false;
}

void MainWindow::process_input_buffer() {
    // Check for totem '?'
    size_t totem_pos = input_buffer_.find('?');
    if (totem_pos == std::string::npos) {
        hide_suggestion_overlay();
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
    
    // Check cache first
    auto cached = suggestion_cache_->get(query);
    if (cached) {
        current_suggestion_ = cached;
        show_suggestion_overlay();
        update_suggestion_ui("💡 " + cached->command + " (cached)", true);
        gtk_image_set_from_icon_name(icon_image_, "emoji-objects-symbolic", GTK_ICON_SIZE_MENU);
        return;
    }
    
    if (!is_predicting_) {
        is_predicting_ = true;
        show_suggestion_overlay();
        update_suggestion_ui("Consultando IA...", false);
        gtk_spinner_start(spinner_);
        gtk_widget_show(GTK_WIDGET(spinner_));
        gtk_widget_hide(GTK_WIDGET(icon_image_));
        
        // Get context from terminal
        auto* terminal = get_current_terminal();
        if (!terminal) return;
        
        std::string term_context = terminal->get_context(20);
        std::string cwd = terminal->get_current_directory();
        std::string project_context = context_service_->get_context_prompt(cwd);
        
        std::string full_context = project_context + "\n\nRecent Terminal Output:\n" + term_context;
        
        // Request prediction
        prediction_service_->predict_async(query, full_context,
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
    gtk_spinner_stop(spinner_);
    gtk_widget_hide(GTK_WIDGET(spinner_));
    gtk_widget_show(GTK_WIDGET(icon_image_));

    if (suggestion) {
        current_suggestion_ = suggestion;
        
        // Cache the suggestion
        if (!last_query_.empty()) {
            suggestion_cache_->put(last_query_, *suggestion);
        }
        
        show_suggestion_overlay();
        update_suggestion_ui("💡 " + suggestion->command, true);
        gtk_image_set_from_icon_name(icon_image_, "emoji-objects-symbolic", GTK_ICON_SIZE_MENU);
    } else {
        current_suggestion_ = std::nullopt;
        update_suggestion_ui("Sin sugerencias", false);
        gtk_image_set_from_icon_name(icon_image_, "dialog-warning-symbolic", GTK_ICON_SIZE_MENU);
        // Auto-hide after 3 seconds
        g_timeout_add(3000, [](gpointer user_data) -> gboolean {
            auto* self = static_cast<MainWindow*>(user_data);
            self->hide_suggestion_overlay();
            return G_SOURCE_REMOVE;
        }, this);
    }
}

void MainWindow::on_apply_suggestion() {
    if (!current_suggestion_) {
        return;
    }
    
    auto* terminal = get_current_terminal();
    if (!terminal) return;
    
    // Clear current line and feed suggestion
    terminal->clear_line();
    terminal->feed_text(current_suggestion_->command);
    
    // Reset state
    input_buffer_.clear();
    last_query_.clear();
    current_suggestion_ = std::nullopt;
    hide_suggestion_overlay();
    
    // Focus terminal
    gtk_widget_grab_focus(terminal->widget());
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
    hide_suggestion_overlay();
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

void MainWindow::on_new_window_static(GtkMenuItem* item, gpointer user_data) {
    auto* self = static_cast<MainWindow*>(user_data);
    self->on_new_window();
}

void MainWindow::on_about_clicked_static(GtkMenuItem* item, gpointer user_data) {
    auto* self = static_cast<MainWindow*>(user_data);
    self->on_about_clicked();
}

void MainWindow::on_search_clicked_static(GtkButton* button, gpointer user_data) {
    auto* self = static_cast<MainWindow*>(user_data);
    self->on_search_clicked();
}

void MainWindow::on_new_tab_static(GtkButton* button, gpointer user_data) {
    auto* self = static_cast<MainWindow*>(user_data);
    self->on_new_tab();
}

void MainWindow::on_new_window() {
    // Create a new instance of MainWindow
    MainWindow* new_win = new MainWindow();
    new_win->show();
}

void MainWindow::on_about_clicked() {
    GtkWidget* about = gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about), "Colabb Terminal");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about), "1.0.0 (C++ Edition)");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about), 
        "Terminal moderna con asistencia de IA\n\n"
        "Escribe '?' seguido de tu consulta para activar la IA.\n"
        "Presiona Ctrl+Space para aplicar sugerencias.");
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about), "https://github.com/Medalcode/Colabb");
    gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(about), "utilities-terminal");
    
    gtk_window_set_transient_for(GTK_WINDOW(about), GTK_WINDOW(window_));
    gtk_dialog_run(GTK_DIALOG(about));
    gtk_widget_destroy(about);
}

void MainWindow::on_search_clicked() {
    toggle_search();
}

void MainWindow::toggle_search() {
    auto* terminal = get_current_terminal();
    if (!terminal) return;
    
    if (search_bar_->is_visible()) {
        search_bar_->hide();
        terminal->clear_search();
        gtk_widget_grab_focus(terminal->widget());
    } else {
        search_bar_->show();
    }
}

void MainWindow::on_search_query(const std::string& query, bool case_sensitive, bool regex) {
    auto* terminal = get_current_terminal();
    if (!terminal) return;
    
    if (query.empty()) {
        terminal->clear_search();
        return;
    }
    
    terminal->search_text(query, case_sensitive, regex);
}

void MainWindow::on_search_navigate(bool next) {
    auto* terminal = get_current_terminal();
    if (!terminal) return;
    
    if (next) {
        terminal->search_next();
    } else {
        terminal->search_previous();
    }
}

void MainWindow::on_new_tab() {
    tab_manager_->create_tab("");
}

void MainWindow::on_close_tab() {
    tab_manager_->close_current_tab();
}

void MainWindow::on_tab_created(TabManager::TabInfo* tab) {
    // Add suggestion overlay to this tab's overlay
    if (suggestion_revealer_) {
        gtk_overlay_add_overlay(GTK_OVERLAY(tab->overlay), suggestion_revealer_);
    }
    
    // Set up terminal key press callback for this tab
    tab->terminal->set_key_press_callback([this](GdkEventKey* event) {
        return this->on_key_press(event);
    });
    
    // Focus the terminal
    gtk_widget_grab_focus(tab->terminal->widget());
}

void MainWindow::on_tab_closed(int index) {
    // Clean up if needed
    std::cout << "Tab closed: " << index << std::endl;
}

infrastructure::TerminalWidget* MainWindow::get_current_terminal() {
    auto* tab = tab_manager_->get_current_tab();
    return tab ? tab->terminal.get() : nullptr;
}

void MainWindow::on_profiles_clicked_static(GtkMenuItem* item, gpointer user_data) {
    auto* self = static_cast<MainWindow*>(user_data);
    self->on_profiles_clicked();
}

void MainWindow::on_profiles_clicked() {
    ProfileDialog dialog(GTK_WINDOW(window_), profile_manager_.get());
    dialog.run();
    
    // Apply changes (if any) to open tabs
    tab_manager_->refresh_all_tabs();
}

void MainWindow::on_explain_error_clicked_static(GtkMenuItem* item, gpointer user_data) {
    auto* self = static_cast<MainWindow*>(user_data);
    self->on_explain_error();
}

void MainWindow::on_explain_error() {
    auto* terminal = get_current_terminal();
    if (!terminal) return;
    
    // Capture output
    std::string output = terminal->get_context(40);
    std::string cwd = terminal->get_current_directory();
    std::string project_context = context_service_->get_context_prompt(cwd);
    
    std::string prompt = project_context + 
        "\n\nAnalyze the following terminal output. Explain any errors found and suggest a fix:\n\n" + output;
        
    // Reuse prediction UI
    if (!is_predicting_) {
        is_predicting_ = true;
        show_suggestion_overlay();
        update_suggestion_ui("Analizando error...", false);
        gtk_spinner_start(spinner_);
        gtk_widget_show(GTK_WIDGET(spinner_));
        gtk_widget_hide(GTK_WIDGET(icon_image_));
        
        prediction_service_->predict_async("Explain Error", prompt,
            [this](std::optional<domain::Suggestion> suggestion) {
                g_idle_add([](gpointer user_data) -> gboolean {
                    auto* pair = static_cast<std::pair<MainWindow*, std::optional<domain::Suggestion>>*>(user_data);
                    pair->first->on_prediction_result(pair->second);
                    delete pair;
                    return G_SOURCE_REMOVE;
                }, new std::pair<MainWindow*, std::optional<domain::Suggestion>>(this, suggestion));
            });
    }
}

} // namespace ui
} // namespace colabb
