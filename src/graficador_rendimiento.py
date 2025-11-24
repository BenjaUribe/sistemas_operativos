import matplotlib.pyplot as plt
import re

print("Generando grafico de rendimiento...")

# Leer el archivo de logs de tiempos
try:
    with open("data/logs_tiempos.txt", "r") as f:
        lines = f.readlines()
except FileNotFoundError:
    print("ERROR: No se encontro el archivo data/logs_tiempos.txt")
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

# Guardar el grafico
output_path = "data/graficos/grafico_rendimiento.png"
plt.savefig(output_path, dpi=300, bbox_inches='tight')
print(f"Grafico guardado exitosamente en: {output_path}")

# Vaciar el archivo de logs
try:
    with open("data/logs_tiempos.txt", "w") as f:
        pass  # Abre en modo escritura y cierra, dejándolo vacío
except OSError as e:
    print(f"Advertencia: No se pudo vaciar el archivo de logs: {e}")



