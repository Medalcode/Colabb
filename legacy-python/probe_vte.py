import gi
import sys
import time

try:
    gi.require_version('Gtk', '3.0')
    gi.require_version('Vte', '2.91')
    from gi.repository import Gtk, Vte, GLib
except Exception as e:
    print(f"Setup Error: {e}")
    sys.exit(1)

def on_vte_contents_changed(terminal):
    print("Content changed, probing...")
    probe_methods(terminal)
    Gtk.main_quit()

def probe_methods(t):
    print("\n--- PROBING VTE METHODS ---")
    
    # 1. get_text_range
    print("1. Testing get_text_range(0,0, 0,-1, None)...")
    try:
        res = t.get_text_range(0, 0, 0, -1, None)
        print(f"   Result: {type(res)} -> {res}")
    except Exception as e:
        print(f"   FAIL: {e}")

    # 1b. get_text_range without last arg (if allowed)
    print("1b. Testing get_text_range(0,0, 0,-1)...")
    try:
        res = t.get_text_range(0, 0, 0, -1)
        print(f"   Result: {type(res)} -> {res}")
    except Exception as e:
        print(f"   FAIL: {e}")

    # 2. get_text
    print("2. Testing get_text(None)...")
    try:
        res = t.get_text(None)
        print(f"   Result: {type(res)} -> {res}")
    except Exception as e:
        print(f"   FAIL: {e}")

    # 3. get_text with explicit callback None
    print("3. Testing get_text(None, None) if supported...")
    try:
        res = t.get_text(None, None)
        print(f"   Result: {type(res)} -> {res}")
    except Exception as e:
        print(f"   FAIL: {e}")

    # 4. get_cursor_position
    print("4. Testing get_cursor_position()...")
    try:
        res = t.get_cursor_position()
        print(f"   Result: {res}")
    except Exception as e:
        print(f"   FAIL: {e}")

win = Gtk.Window()
term = Vte.Terminal()
win.add(term)
win.show_all()

# Spawn a shell that echoes something
term.spawn_sync(
    Vte.PtyFlags.DEFAULT,
    None,
    ["/bin/bash", "-c", "echo 'TEST_LINE'; sleep 1"],
    [],
    GLib.SpawnFlags.DEFAULT,
    None,
    None,
)

# Connect signal to probe once we have data
term.connect("contents-changed", on_vte_contents_changed)

# Timeout in case signal doesn't fire
GLib.timeout_add(2000, Gtk.main_quit)

Gtk.main()
