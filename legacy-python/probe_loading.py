
import ctypes
import ctypes.util
import os

print("--- PROBING LIBVTE LOADING ---")

# 1. Try find_library
name = ctypes.util.find_library('vte-2.91')
print(f"find_library('vte-2.91'): {name}")

# 2. Try generic load
try:
    lib = ctypes.CDLL("libvte-2.91.so.0")
    print(f"CDLL('libvte-2.91.so.0'): SUCCESS - {lib}")
except Exception as e:
    print(f"CDLL('libvte-2.91.so.0'): FAIL - {e}")

# 3. Try absolute path
abs_path = "/usr/lib/x86_64-linux-gnu/libvte-2.91.so.0"
print(f"Checking absolute path exists: {os.path.exists(abs_path)}")
try:
    lib = ctypes.CDLL(abs_path)
    print(f"CDLL('{abs_path}'): SUCCESS - {lib}")
except Exception as e:
    print(f"CDLL('{abs_path}'): FAIL - {e}")

# 4. Try loading global
try:
    global_lib = ctypes.CDLL(None)
    symbol = global_lib.vte_terminal_get_text_range
    print(f"Global Namespace 'vte_terminal_get_text_range': FOUND - {symbol}")
except Exception as e:
    print(f"Global Namespace 'vte_terminal_get_text_range': FAIL - {e}")
