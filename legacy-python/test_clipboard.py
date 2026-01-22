
import gi
import time
gi.require_version('Gtk', '3.0')
gi.require_version('Vte', '2.91')
from gi.repository import Gtk, Vte, Gdk

def test_clipboard():
    win = Gtk.Window()
    term = Vte.Terminal()
    win.add(term)
    win.show_all()
    
    # Put some text
    term.feed(b"Line 1: Error critical\nLine 2: Success\n")
    
    # Wait for rendering
    while Gtk.events_pending(): Gtk.main_iteration()
    
    print("Selecting all...")
    term.select_all()
    
    print("Copying...")
    # COPY_CLIPBOARD = 1
    term.copy_clipboard()
    
    # Wait for clipboard event
    time.sleep(0.5)
    while Gtk.events_pending(): Gtk.main_iteration()
    
    clipboard = Gtk.Clipboard.get(Gdk.SELECTION_CLIPBOARD)
    text = clipboard.wait_for_text()
    
    print(f"--- CLIPBOARD CONTENT ---\n{text}\n-------------------------")
    
    if text and "Error critical" in text:
        print("SUCCESS: We can read via clipboard!")
    else:
        print("FAIL: Clipboard did not return expected text.")

test_clipboard()
