import matplotlib.pyplot as plt
import re
import os
from datetime import datetime
from dotenv import load_dotenv

def obtener_ruta_base():
    """Retorna el directorio donde se encuentra este script .py"""
    return os.path.dirname(os.path.abspath(__file__))

# Cargar variables de entorno
dir_actual = obtener_ruta_base()
dir_proyecto = os.path.dirname(dir_actual)
ruta_env = os.path.join(dir_proyecto, '.env')
load_dotenv(ruta_env)

print("Generando grafico de rendimiento...")

# Obtener ruta del archivo de logs
archivo_logs = os.path.join(dir_actual, 'data', 'logs_tiempos.txt')

# Leer el archivo de logs de tiempos
try:
    with open(archivo_logs, "r") as f:
        lines = f.readlines()
except FileNotFoundError:
    print(f"ERROR: No se encontro el archivo {archivo_logs}")
    exit(1)

# Parsear las tuplas (threads, tiempo)
threads = []
tiempos = []

for line in lines:
    # Formato esperado: (2, 1234.56)
    match = re.match(r'\((\d+),\s*([\d.]+)\)', line.strip())
    if match:
        threads.append(int(match.group(1)))
        tiempos.append(float(match.group(2)))

if not threads:
    print("ERROR: No se encontraron datos validos en logs_tiempos.txt")
    exit(1)

print(f"Datos encontrados: {len(threads)} mediciones")
print(f"Threads: {threads}")
print(f"Tiempos: {tiempos}")

# Crear el grafico
plt.figure(figsize=(10, 6))
plt.plot(threads, tiempos, marker='o', linewidth=2, markersize=8, color='blue')
plt.xlabel('Numero de Threads', fontsize=12)
plt.ylabel('Tiempo (ms)', fontsize=12)
plt.title('Rendimiento del Indice Invertido Paralelo', fontsize=14, fontweight='bold')
plt.grid(True, alpha=0.3)
plt.xticks(threads)

# Obtener ruta de salida desde .env
ruta_relativa = os.getenv('GRAFICOS_REN', 'data/graficos/graficador_rendimiento_indice')
if ruta_relativa.startswith('data/'):
    ruta_graficos = os.path.join(dir_actual, ruta_relativa)
else:
    ruta_graficos = os.path.join(dir_proyecto, ruta_relativa)

# Crear directorio si no existe
os.makedirs(ruta_graficos, exist_ok=True)

# Guardar el grafico con timestamp
fecha_hora = datetime.now().strftime("%Y-%m-%d %H-%M-%S")
output_path = os.path.join(ruta_graficos, f"{fecha_hora}.png")
plt.savefig(output_path, dpi=300, bbox_inches='tight')
print(f"Grafico guardado exitosamente en: {output_path}")

# Vaciar el archivo de logs
try:
    with open(archivo_logs, "w") as f:
        pass  
    print(f"Archivo de logs vaciado: {archivo_logs}")
except OSError as e:
    print(f"Advertencia: No se pudo vaciar el archivo de logs: {e}")



