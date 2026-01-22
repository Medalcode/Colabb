
import ctypes
from gi.repository import Gtk, Gdk, Vte, GLib

# --- CTYPES LOAD ---
try:
    # Ajusta el path si es necesario (ej. usando find /usr/lib -name ...)
    libvte = ctypes.CDLL("libvte-2.91.so.0")
    
    # char *vte_terminal_get_text_range (VteTerminal *terminal,
    #                                    long start_row, long start_col,
    #                                    long end_row, long end_col,
    #                                    VteSelectionFunc is_selected,
    #                                    gpointer user_data,
    #                                    GArray *attributes);
    
    libvte.vte_terminal_get_text_range.argtypes = [
        ctypes.c_void_p, # terminal instance
        ctypes.c_long,   # start_row
        ctypes.c_long,   # start_col
        ctypes.c_long,   # end_row
        ctypes.c_long,   # end_col
        ctypes.c_void_p, # is_selected (func ptr)
        ctypes.c_void_p, # user_data
        ctypes.c_void_p  # attributes (GArray*) -> We want NULL here!
    ]
    libvte.vte_terminal_get_text_range.restype = ctypes.c_char_p # Returns string
    print("DEBUG: libvte loaded successfully via ctypes")
except Exception as e:
    print(f"DEBUG: Failed to load libvte via ctypes: {e}")
    libvte = None

def get_vte_text_ctypes(terminal, row):
    if not libvte:
        return None
    
    try:
        # hash(terminal) might not be the pointer. We need the GObject pointer.
        # Python GI objects wrap the pointer. 
        # We can use hash(terminal) in some versions, but better: 
        # terminal.__gpointer__ ? No.
        # hash(terminal) is usually the address of the wrapper or the object?
        # Actually proper way:
        ptr = hash(terminal) 
        
        # NOTE: getting the real pointer from GI object is tricky in pure python without help.
        # However, ``int(terminal)`` often doesn't work.
        # ``hash(terminal)`` is often the pointer value in GObject overrides.
        
        # Let's try to get text of just the current row
        # start_row, start_col, end_row, end_col
        # We want the whole row, so 0 to -1?
        
        # But wait, we need the pointer. 
        # Let's trust that for now, or fallback to the previous broken method if this crashes.
        # Actually, let's look for a safer way to get the pointer if possible.
        pass
    except:
        pass
    return None
