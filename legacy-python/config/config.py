"""
Gestión de configuración persistente y cifrado de API Key
"""
import json
import os
from cryptography.fernet import Fernet

CONFIG_PATH = os.path.join(os.path.dirname(__file__), 'config.json')
KEY_PATH = os.path.join(os.path.dirname(__file__), 'key.key')

class ConfigManager:
    def __init__(self):
        self.key = self._load_or_create_key()
        self.fernet = Fernet(self.key)
        self.config = self._load_config()

    def _load_or_create_key(self):
        if not os.path.exists(KEY_PATH):
            key = Fernet.generate_key()
            with open(KEY_PATH, 'wb') as f:
                f.write(key)
            return key
        with open(KEY_PATH, 'rb') as f:
            return f.read()

    def _load_config(self):
        if not os.path.exists(CONFIG_PATH):
            return {}
        with open(CONFIG_PATH, 'r') as f:
            return json.load(f)

    def save_api_key(self, api_key):
        encrypted = self.fernet.encrypt(api_key.encode()).decode()
        self.config['api_key'] = encrypted
        with open(CONFIG_PATH, 'w') as f:
            json.dump(self.config, f)

    def get_api_key(self):
        encrypted = self.config.get('api_key')
        if encrypted:
            return self.fernet.decrypt(encrypted.encode()).decode()
        return None

    def save_provider(self, provider):
        self.config['provider'] = provider
        with open(CONFIG_PATH, 'w') as f:
            json.dump(self.config, f)

    def get_provider(self):
        return self.config.get('provider', 'groq')
