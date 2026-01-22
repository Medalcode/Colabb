import gi
import ctypes
import sys
gi.require_version('Gtk', '3.0')
gi.require_version('Vte', '2.91')
from gi.repository import Gtk, Vte, GLib

try:
    libvte = ctypes.CDLL("libvte-2.91.so.0")
    
    # char *vte_terminal_get_text_range (VteTerminal *terminal,
    #                                    long start_row, long start_col,
    #                                    long end_row, long end_col,
    #                                    VteSelectionFunc is_selected,
    #                                    gpointer user_data,
    #                                    GArray *attributes);
    libvte.vte_terminal_get_text_range.argtypes = [
        ctypes.c_void_p,
        ctypes.c_long,
        ctypes.c_long,
        ctypes.c_long,
        ctypes.c_long,
        ctypes.c_void_p,
        ctypes.c_void_p,
        ctypes.c_void_p
    ]
    libvte.vte_terminal_get_text_range.restype = ctypes.c_char_p
    print("Lib loaded.")
except Exception as e:
    print(f"Lib fail: {e}")
    sys.exit(1)

def probe():
    t = Vte.Terminal()
    # Spawn to get some text
    t.spawn_sync(Vte.PtyFlags.DEFAULT, None, ["/bin/echo", "HELLO_CTYPES"], [], GLib.SpawnFlags.DEFAULT, None, None)
    
    while Gtk.events_pending():
        Gtk.main_iteration()

    # Get pointer
    ptr = hash(t)
    print(f"Pointer: {ptr}")

    # Try read row 0
    # Using range(0,0) to (-1, -1) which should cover everything or specific row
    res = libvte.vte_terminal_get_text_range(ctypes.c_void_p(ptr), 0, 0, 1, -1, None, None, None)
    print(f"Result: {res}")

probe()
