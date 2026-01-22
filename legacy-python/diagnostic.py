import os
import sys

# Add current directory to path so imports work
sys.path.append(os.getcwd())

try:
    from ai.predictor import Predictor
    from config.config import ConfigManager
except ImportError as e:
    print(f"Error importing modules: {e}")
    print("Make sure you are running this from the project root (Colabb folder)")
    sys.exit(1)

def run_diagnostic():
    print("--- Colabb AI Diagnostic Tool ---")
    
    # Check Config
    cm = ConfigManager()
    provider = cm.get_provider()
    api_key = cm.get_api_key()
    
    print(f"1. Configuration:")
    print(f"   Provider: {provider}")
    if api_key:
        masked_key = api_key[:4] + "*" * (len(api_key)-8) + api_key[-4:] if len(api_key) > 8 else "****"
        print(f"   API Key:  {masked_key} (Length: {len(api_key)})")
    else:
        print(f"   API Key:  NOT SET (Please configure via settings icon in the app)")
        
    if not api_key and provider != 'local':
        print("\n[!] FATAL: API Key is missing and provider is not local.")
        return

    # Check Predictor
    print(f"\n2. Testing Prediction (Provider: {provider})...")
    pred = Predictor()
    
    # test validation
    print("   Validating API Key...")
    valid = pred.validate_api_key()
    if valid is True:
        print("   ✅ Validation Success")
    else:
        print(f"   ❌ Validation Failed: {valid}")
        return

    # test simple prediction
    test_prompt = "list files"
    print(f"   Sending query: '{test_prompt}'...")
    try:
        result = pred.predict(test_prompt)
        if result:
            print(f"   ✅ Prediction Received: '{result}'")
        else:
            print(f"   ❌ Prediction returned None (Check logs/console for details)")
    except Exception as e:
        print(f"   ❌ Exception during prediction: {e}")

if __name__ == "__main__":
    run_diagnostic()
