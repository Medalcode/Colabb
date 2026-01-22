"""
Ventana principal de la aplicación usando VTE (Terminal Tradicional + IA)
"""
import gi
import os
import threading
import time

gi.require_version('Gtk', '3.0')
gi.require_version('Vte', '2.91')
from gi.repository import Gtk, Gdk, Vte, GLib

from config.config import ConfigManager
from ai.predictor import Predictor
import ctypes

# --- VTE CTYPES WORKAROUND ---
# Fix for "assertion 'attributes == nullptr' failed" in VTE 2.91+ / Python GI
# --- VTE CTYPES WORKAROUND ---
# Fix for "assertion 'attributes == nullptr' failed" in VTE 2.91+ / Python GI
LIBVTE = None
try:
    # Intentar cargar usando find_library primero para ser más compatible
    import ctypes.util
    lib_path = ctypes.util.find_library('vte-2.91')
    if not lib_path:
        # Fallback a path común si find_library falla
        lib_path = "/usr/lib/x86_64-linux-gnu/libvte-2.91.so.0"
    
    print(f"DEBUG: Attempting to load VTE from: {lib_path}")
    LIBVTE = ctypes.CDLL(lib_path)
        
    print(f"DEBUG: VTE Library loaded via ctypes from {lib_path}")

    # char *vte_terminal_get_text_range (...)
    LIBVTE.vte_terminal_get_text_range.argtypes = [
        ctypes.c_void_p, ctypes.c_long, ctypes.c_long, ctypes.c_long, ctypes.c_long, 
        ctypes.c_void_p, ctypes.c_void_p, ctypes.c_void_p
    ]
    LIBVTE.vte_terminal_get_text_range.restype = ctypes.c_char_p
except Exception as e:
    print(f"FATAL WARNING: Could not load libvte via ctypes: {e}") 
    LIBVTE = None
# -----------------------------

class ConfigDialog(Gtk.Dialog):
    def __init__(self, parent):
        Gtk.Dialog.__init__(self, title="Configuración IA", transient_for=parent, flags=0)
        self.set_default_size(400, 200)
        box = self.get_content_area()
        box.set_spacing(10)
        box.set_margin_top(10)
        box.set_margin_bottom(10)
        box.set_margin_start(10)
        box.set_margin_end(10)

        self.provider_combo = Gtk.ComboBoxText()
        self.provider_combo.append_text("groq")
        self.provider_combo.append_text("openai")
        self.provider_combo.append_text("local")
        self.provider_combo.set_active(0)

        self.api_entry = Gtk.Entry()
        self.api_entry.set_placeholder_text("Pegar API Key aquí...")
        self.api_entry.set_visibility(False) # Ocultar caracteres por seguridad

        save_button = Gtk.Button(label="Guardar y Validar")
        save_button.get_style_context().add_class("suggested-action")
        save_button.connect("clicked", self.on_save)

        # Layout
        grid = Gtk.Grid()
        grid.set_column_spacing(10)
        grid.set_row_spacing(10)
        
        grid.attach(Gtk.Label(label="Proveedor:"), 0, 0, 1, 1)
        grid.attach(self.provider_combo, 1, 0, 2, 1)
        
        grid.attach(Gtk.Label(label="API Key:"), 0, 1, 1, 1)
        grid.attach(self.api_entry, 1, 1, 2, 1)
        
        box.pack_start(grid, False, False, 0)
        box.pack_end(save_button, False, False, 10)

        self.config = ConfigManager()
        self.predictor = Predictor()
        self.load_config()
        self.show_all()

    def load_config(self):
        provider = self.config.get_provider()
        if provider == "groq":
            self.provider_combo.set_active(0)
        elif provider == "openai":
            self.provider_combo.set_active(1)
        else:
            self.provider_combo.set_active(2)
            
        # No cargamos la API key al campo por seguridad y porque está encriptada
        # self.api_entry.set_text("***") 

    def on_save(self, widget):
        provider = self.provider_combo.get_active_text()
        api_key = self.api_entry.get_text()
        
        self.config.save_provider(provider)
        
        if api_key:
            self.config.save_api_key(api_key)
            self.predictor.provider = provider
            self.predictor.api_key = api_key
            
            # Feedback inmediato
            valid = self.predictor.validate_api_key()
            if valid is True:
                self.show_message("✅ Conexión Exitosa", "La API Key es válida y se ha guardado.")
            else:
                self.show_message("❌ Error de Validación", f"No se pudo conectar: {valid}")
        else:
            self.show_message("Configuración Guardada", "Se guardó el proveedor (sin cambiar API Key).")

    def show_message(self, title, message):
        dialog = Gtk.MessageDialog(parent=self, flags=0, message_type=Gtk.MessageType.INFO, buttons=Gtk.ButtonsType.OK, text=title)
        dialog.format_secondary_text(message)
        dialog.run()
        dialog.destroy()


class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Colabb Terminal")
        print("DEBUG: LATEST VERSION LOADED (Keylogger Mode)", flush=True)
        self.set_default_size(950, 650)
        
        # Header Bar con botón de Config
        header = Gtk.HeaderBar()
        header.set_show_close_button(True)
        header.set_title("Colabb Terminal")
        self.set_titlebar(header)
        
        config_btn = Gtk.Button.new_from_icon_name("emblem-system-symbolic", Gtk.IconSize.BUTTON)
        config_btn.set_tooltip_text("Configuración IA")
        config_btn.connect("clicked", self.on_config_clicked)
        header.pack_end(config_btn)

        # Configuración y Backend
        self.params_lock = threading.Lock()
        self.config = ConfigManager()
        self.predictor = Predictor()
        
        # Contenedor Vertical
        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=0)
        self.add(vbox)

        # --- Terminal VTE ---
        self.terminal = Vte.Terminal()
        self.terminal.set_mouse_autohide(True)
        
        # Colores decentes por defecto (Dark)
        bg = Gdk.RGBA(); bg.parse("#1e1e1e")
        fg = Gdk.RGBA(); fg.parse("#f0f0f0")
        self.terminal.set_colors(fg, bg, [])
        
        self.spawn_shell()
        
        # Scroll para la terminal
        scrolled_window = Gtk.ScrolledWindow()
        scrolled_window.add(self.terminal)
        vbox.pack_start(scrolled_window, True, True, 0)
        
        # --- Barra de Sugerencias (Status Bar) ---
        self.suggestion_box = Gtk.Box(orientation=Gtk.Orientation.HORIZONTAL, spacing=10)
        self.suggestion_box.get_style_context().add_class("suggestion-bar")
        
        self.icon_img = Gtk.Image.new_from_icon_name("dialog-idea-symbolic", Gtk.IconSize.MENU)
        self.suggestion_box.pack_start(self.icon_img, False, False, 5)
        
        self.suggestion_label = Gtk.Label(label="Escribe '?' y espera la sugerencia...")
        self.suggestion_label.set_ellipsize(3) # END
        self.suggestion_label.set_xalign(0)
        self.suggestion_box.pack_start(self.suggestion_label, True, True, 0)
        
        self.apply_btn = Gtk.Button(label="Aplicar (Ctrl+Space)")
        self.apply_btn.connect("clicked", self.apply_suggestion)
        self.apply_btn.get_style_context().add_class("suggest-btn")
        self.apply_btn.set_sensitive(False)
        self.suggestion_box.pack_end(self.apply_btn, False, False, 5)
        
        vbox.pack_end(self.suggestion_box, False, False, 0)

        # Eventos
        self.terminal.connect("key-release-event", self.on_key_release)
        self.terminal.connect("key-press-event", self.on_key_press)
        
        # State
        self.debounce_timer = None
        self.current_suggestion = None
        self.last_input_text = ""
        self.is_predicting = False
        self.predict_req_id = 0

        # Cargar CSS
        self._load_css()
        
        # Focus
        self.terminal.grab_focus()
        self.local_buffer = ""

    def spawn_shell(self):
        shell = os.environ.get("SHELL", "/bin/bash")
        home = os.environ.get("HOME", "/home")
        
        # LOGGING STRATEGY: Wrap shell in 'script' to capture output
        # This allows us to read the screen/errors for the AI since VTE is broken.
        self.session_log = os.path.join(home, ".colabb_session.log")
        # Clear previous log
        if os.path.exists(self.session_log):
            try: os.remove(self.session_log)
            except: pass
            
        command = ["/usr/bin/script", "-q", "-f", self.session_log, "-c", shell]
        
        try:
            self.terminal.spawn_sync(
                Vte.PtyFlags.DEFAULT, home, command, [],
                GLib.SpawnFlags.DO_NOT_REAP_CHILD, None, None,
            )
            print(f"DEBUG: Shell spawned wrapped in script logging to {self.session_log}")
        except Exception as e:
            print(f"Error spawning shell: {e}")

    def _load_css(self):
        css = """
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
        """
        provider = Gtk.CssProvider()
        provider.load_from_data(css.encode())
        Gtk.StyleContext.add_provider_for_screen(
            Gdk.Screen.get_default(), provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION
        )

    def on_config_clicked(self, widget):
        dialog = ConfigDialog(self)
        dialog.run()
        dialog.destroy()
        # Recargar predictor
        self.predictor = Predictor()

    def on_key_press(self, widget, event):
        # Hotkey para aplicar sugerencia
        if event.state & Gdk.ModifierType.CONTROL_MASK and event.keyval == Gdk.KEY_space:
            self.apply_suggestion(None)
            return True
        
        # Escape para resetear estado manual
        if event.keyval == Gdk.KEY_Escape:
             self.local_buffer = ""
             self.suggestion_label.set_text("IA Reseteada. Escribe '?'...")
             self.apply_btn.set_sensitive(False)
             return True

        # Teclas de control que limpian el buffer
        if event.keyval in [Gdk.KEY_Return, Gdk.KEY_KP_Enter]:
            self.local_buffer = ""
            self.current_suggestion = None
            self.suggestion_label.set_text("Escribe '?' y espera la sugerencia...")
            self.apply_btn.set_sensitive(False)
            return False
            
        return False

    def on_key_release(self, widget, event):
        # Gestión manual del buffer de entrada
        val = event.keyval
        
        # Caracteres imprimibles
        if val < 255 and chr(val).isprintable():
            char = chr(val)
            self.local_buffer += char
        
        # Backspace
        elif val == Gdk.KEY_BackSpace:
            self.local_buffer = self.local_buffer[:-1]
            
        # Atajos de limpieza comunes (Ctrl+C, Ctrl+U, Ctrl+L)
        elif event.state & Gdk.ModifierType.CONTROL_MASK:
            # c=99, u=117, l=108
            if val in [99, 117, 108]: 
                self.local_buffer = ""

        self._trigger_prediction()

    def _trigger_prediction(self):
        self.predict_req_id += 1
        req_id = self.predict_req_id
        GLib.timeout_add(300, self._check_prediction_request, req_id)

    def _check_prediction_request(self, req_id):
        if req_id != self.predict_req_id:
            return False
        self._process_current_buffer()
        return False

    def _process_current_buffer(self):
        try:
            # Usamos el buffer local en lugar de leer VTE
            raw_line = self.local_buffer
            
            clean_input = ""
            if "?" in raw_line:
                # Tomamos lo que esté después del ULTIMO '?'
                clean_input = raw_line.split("?")[-1].strip()
            
            # Si el buffer está muy sucio o vacío, o no tiene '?'
            if not "?" in raw_line or len(raw_line) > 200:
                # Auto-limpieza si se hace enorme
                if len(raw_line) > 200: self.local_buffer = ""
                
                current_lbl = self.suggestion_label.get_text()
                target_lbl = "Escribe '?' y espera la sugerencia..."
                if current_lbl != target_lbl and not self.is_predicting:
                     GLib.idle_add(self.suggestion_label.set_text, target_lbl)
                     GLib.idle_add(self.apply_btn.set_sensitive, False)
                return False

            if clean_input and clean_input != self.last_input_text:
                self.last_input_text = clean_input
                if not self.is_predicting:
                    msg = f"Consultando IA para: {clean_input}..."
                    print(f"DEBUG: Triggering AI -> {msg}")
                    GLib.idle_add(self.suggestion_label.set_text, msg)
                    
                    # --- READ CONTEXT FROM LOG ---
                    context = ""
                    try:
                        if hasattr(self, 'session_log') and os.path.exists(self.session_log):
                            with open(self.session_log, 'rb') as f:
                                # Read tail
                                try:
                                    f.seek(-2000, 2)
                                except: pass
                                raw_data = f.read().decode('utf-8', errors='ignore')
                                
                                # Strip ANSI codes (simple regex)
                                import re
                                ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
                                context = ansi_escape.sub('', raw_data)
                                
                                # Keep last 20 lines
                                lines = context.splitlines()
                                context = "\n".join(lines[-20:])
                    except Exception as e:
                        print(f"DEBUG: Failed to read context: {e}")

                    full_prompt = f"Context:\n{context}\n\nQuery: {clean_input}"
                    
                    self.is_predicting = True
                    threading.Thread(target=self._run_prediction_thread, args=(full_prompt,)).start()
            
            return False
        except Exception as e:
            print(f"DEBUG: Error in buffer processing: {e}")
            return False

    def _run_prediction_thread(self, text):
        try:
            suggestion = self.predictor.predict(text)
        except Exception as e:
            suggestion = None
            print(f"Error predictor: {e}")
            
        GLib.idle_add(self._update_ui_with_suggestion, suggestion)

    def _update_ui_with_suggestion(self, suggestion):
        self.is_predicting = False
        if suggestion:
            self.current_suggestion = suggestion
            self.suggestion_label.set_text(f"Sugerencia: {suggestion} (Ctrl+Space)")
            self.apply_btn.set_sensitive(True)
            self.icon_img.set_from_icon_name("emoji-objects-symbolic", Gtk.IconSize.MENU)
        else:
            self.current_suggestion = None
            self.suggestion_label.set_text("Sin sugerencias")
            self.apply_btn.set_sensitive(False)
            self.icon_img.set_from_icon_name("dialog-idea-symbolic", Gtk.IconSize.MENU)

    def apply_suggestion(self, btn):
        if self.current_suggestion:
            cmds = [b'\x15', self.current_suggestion.encode('utf-8')]
            for c in cmds:
                self.terminal.feed_child(c)
            self.terminal.grab_focus()
            
            # CRITICAL FIX: Limpiar buffer y estado
            self.local_buffer = ""
            self.last_input_text = ""
            self.suggestion_label.set_text("Aplicado. Presiona Enter para ejecutar.")
            self.current_suggestion = None
            self.apply_btn.set_sensitive(False)

def main_window():
    win = MainWindow()
    win.connect("destroy", Gtk.main_quit)
    win.show_all()
    Gtk.main()
