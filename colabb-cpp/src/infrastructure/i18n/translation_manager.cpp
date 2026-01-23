#include "infrastructure/i18n/translation_manager.hpp"

namespace colabb {
namespace infrastructure {

TranslationManager& TranslationManager::instance() {
    static TranslationManager instance;
    return instance;
}

TranslationManager::TranslationManager() {
    load_language("es"); // Default
}

void TranslationManager::load_language(const std::string& lang_code) {
    current_lang_ = lang_code;
    translations_.clear();
    
    if (lang_code == "es") {
        load_es();
    } else {
        load_en();
    }
}

std::string TranslationManager::get(const std::string& key) {
    auto it = translations_.find(key);
    if (it != translations_.end()) {
        return it->second;
    }
    return key; // Fallback to key
}

void TranslationManager::load_es() {
    translations_["menu.file"] = "Archivo";
    translations_["menu.new_window"] = "Nueva ventana";
    translations_["menu.preferences"] = "Preferencias IA";
    translations_["menu.profiles"] = "Perfiles";
    translations_["menu.explain_error"] = "Explicar Error";
    translations_["menu.about"] = "Acerca de";
    translations_["menu.main_tooltip"] = "Menú principal";
    translations_["tab.new"] = "Nueva pestaña";
    translations_["tab.new_tooltip"] = "Nueva pestaña (Ctrl+Shift+T)";
    translations_["search.tooltip"] = "Buscar";
    translations_["suggestion.apply"] = "Aplicar";
    translations_["dialog.about.title"] = "Acerca de Colabb Terminal";
    translations_["dialog.about.comment"] = "Terminal moderna con asistencia de IA\n\nEscribe '?' seguido de tu consulta.\nPresiona Tab o Ctrl+Space para aceptar.";
    // Add more as needed
}

void TranslationManager::load_en() {
    translations_["menu.file"] = "File";
    translations_["menu.new_window"] = "New Window";
    translations_["menu.preferences"] = "AI Preferences";
    translations_["menu.profiles"] = "Profiles";
    translations_["menu.explain_error"] = "Explain Error";
    translations_["menu.about"] = "About";
    translations_["menu.main_tooltip"] = "Main Menu";
    translations_["tab.new"] = "New Tab";
    translations_["tab.new_tooltip"] = "New Tab (Ctrl+Shift+T)";
    translations_["search.tooltip"] = "Search";
    translations_["suggestion.apply"] = "Apply";
    translations_["dialog.about.title"] = "About Colabb Terminal";
    translations_["dialog.about.comment"] = "Modern terminal with AI assistance\n\nType '?' followed by your query.\nPress Tab or Ctrl+Space to accept.";
}

} // namespace infrastructure
} // namespace colabb
