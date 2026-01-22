"""
Lógica de predicción de comandos usando IA (Groq, OpenAI, local)
"""
import requests
from config.config import ConfigManager

class Predictor:
    def __init__(self):
        self.config = ConfigManager()
        self.provider = self.config.get_provider()
        self.api_key = self.config.get_api_key()

    def validate_api_key(self):
        if self.provider == 'groq':
            try:
                # Usar endpoint de completions para validar la API Key
                url = 'https://api.groq.com/openai/v1/chat/completions'
                headers = {"Authorization": f"Bearer {self.api_key}", "Content-Type": "application/json"}
                data = {
                    "model": "llama-3.1-8b-instant",
                    "messages": [{"role": "user", "content": "Hola"}],
                    "max_tokens": 1
                }
                response = requests.post(url, headers=headers, json=data)
                if response.status_code == 200:
                    return True
                else:
                    error_msg = f"{response.status_code} - {response.text}"
                    print(f"Groq API error: {error_msg}")
                    return error_msg
            except Exception as e:
                print(f"Groq API exception: {e}")
                return str(e)
        elif self.provider == 'openai':
            try:
                response = requests.get('https://api.openai.com/v1/models', headers={"Authorization": f"Bearer {self.api_key}"})
                if response.status_code == 200:
                    return True
                else:
                    return f"{response.status_code} - {response.text}"
            except Exception as e:
                return str(e)
        elif self.provider == 'local':
            return True
        return False

    def predict(self, prompt):
        """Sugiere un comando basado en la intención."""
        system_prompt = "Eres un experto en terminal Linux. Devuelve SOLO el comando bash sugerido, sin explicaciones."
        if self.provider == 'groq':
            url = 'https://api.groq.com/openai/v1/chat/completions'
            headers = {"Authorization": f"Bearer {self.api_key}", "Content-Type": "application/json"}

            data = {
                "model": "llama-3.1-8b-instant",
                "messages": [
                    {"role": "system", "content": system_prompt},
                    {"role": "user", "content": f"Sugerencia para: {prompt}"}
                ],
                "max_tokens": 50
            }
            try:
                response = requests.post(url, headers=headers, json=data)
                if response.status_code == 200:
                    content = response.json()['choices'][0]['message']['content'].strip()
                    # Limpiar markdown formatting del output
                    content = content.replace("```bash", "").replace("```", "").replace("`", "").strip()
                    return content
                else:
                    print(f"DEBUG: Groq API Error {response.status_code}: {response.text}")
            except Exception as e:
                print(f"DEBUG: Error in Predictor.predict: {e}")
                pass
            return None
        elif self.provider == 'openai':
            # Implementación simplificada para OpenAI Chat
            url = 'https://api.openai.com/v1/chat/completions'
            headers = {"Authorization": f"Bearer {self.api_key}"}
            data = {
                "model": "gpt-3.5-turbo",
                "messages": [
                    {"role": "system", "content": system_prompt},
                    {"role": "user", "content": prompt}
                ]
            }
            try:
                response = requests.post(url, headers=headers, json=data)
                if response.status_code == 200:
                    content = response.json()['choices'][0]['message']['content'].strip()
                    return content.replace("```bash", "").replace("```", "").replace("`", "").strip()
            except:
                pass
            return None
        elif self.provider == 'local':
            return f"echo 'Sugerencia local para: {prompt}'"
        return None

    def explain_error(self, context):
        """Explica un error dado el comando y la salida."""
        system_prompt = "Eres un asistente experto en depuración de Linux. Explica brevemente por qué falló el comando y sugiere una solución."
        query = f"Analiza este error:\n{context}"
        
        if self.provider == 'groq':
            url = 'https://api.groq.com/openai/v1/chat/completions'
            headers = {"Authorization": f"Bearer {self.api_key}", "Content-Type": "application/json"}
            data = {
                "model": "llama-3.1-8b-instant",
                "messages": [
                    {"role": "system", "content": system_prompt},
                    {"role": "user", "content": query}
                ],
                "max_tokens": 300
            }
            try:
                response = requests.post(url, headers=headers, json=data)
                if response.status_code == 200:
                    return response.json()['choices'][0]['message']['content'].strip()
            except Exception as e:
                return f"Error conectando con Groq: {e}"
                
        elif self.provider == 'openai':
            url = 'https://api.openai.com/v1/chat/completions'
            headers = {"Authorization": f"Bearer {self.api_key}"}
            data = {
                "model": "gpt-3.5-turbo",
                "messages": [
                    {"role": "system", "content": system_prompt},
                    {"role": "user", "content": query}
                ]
            }
            try:
                response = requests.post(url, headers=headers, json=data)
                if response.status_code == 200:
                    return response.json()['choices'][0]['message']['content'].strip()
            except Exception as e:
                return f"Error conectando con OpenAI: {e}"
        
        elif self.provider == 'local':
            return "Modo local: Revisa la sintaxis del comando (man command)."
            
        return "No se pudo generar explicación."
