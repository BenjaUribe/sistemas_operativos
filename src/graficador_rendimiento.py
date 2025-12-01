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


# Leer el archivo de logs de tiempos y extraer n_lote
n_lote = None
threads = []
tiempos = []
try:
    with open(archivo_logs, "r") as f:
        lines = f.readlines()
        if lines:
            # Leer n_lote de la primera línea
            match_lote = re.match(r'n_lote=(\d+)', lines[0].strip())
            if match_lote:
                n_lote = int(match_lote.group(1))
            # Procesar el resto de líneas como datos
            for line in lines[1:]:
                match = re.match(r'\((\d+),\s*([\d.]+)\)', line.strip())
                if match:
                    threads.append(int(match.group(1)))
                    tiempos.append(float(match.group(2)))
except FileNotFoundError:
    print(f"ERROR: No se encontro el archivo {archivo_logs}")
    exit(1)

if not threads or n_lote is None:
    print("ERROR: No se encontraron datos validos o n_lote en logs_tiempos.txt")
    exit(1)


print(f"Datos encontrados: {len(threads)} mediciones")
print(f"Threads: {threads}")
print(f"Tiempos: {tiempos}")
print(f"Tamaño de lote (n_lote): {n_lote}")


# Crear el grafico
plt.figure(figsize=(10, 6))
plt.plot(threads, tiempos, marker='o', linewidth=2, markersize=8, color='blue', label=f'Tamaño de lote: {n_lote}')
plt.xlabel('Numero de Threads', fontsize=12)
plt.ylabel('Tiempo (ms)', fontsize=12)
plt.title('Rendimiento del Indice Invertido Paralelo', fontsize=14, fontweight='bold')
plt.grid(True, alpha=0.3)
plt.xticks(threads)

# Etiquetas de tiempo sobre cada punto
for x, y in zip(threads, tiempos):
    plt.annotate(f'{y:.2f} ms', (x, y), textcoords="offset points", xytext=(0,10), ha='center', fontsize=10, color='black', bbox=dict(boxstyle="round,pad=0.2", fc="white", alpha=0.7, lw=0))

# Leyenda con el tamaño de lote
plt.legend()

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



