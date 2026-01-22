"""
Ejecuta comandos Bash y retorna la salida
"""
import subprocess

def execute_command(command):
    try:
        result = subprocess.run(command, shell=True, capture_output=True, text=True)
        return result.stdout, result.stderr
    except Exception as e:
        return '', str(e)
