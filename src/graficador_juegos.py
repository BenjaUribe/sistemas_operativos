import matplotlib.pyplot as plt
import re
from dotenv import load_dotenv
import os
from datetime import datetime

def obtener_ruta_base():
    """Retorna el directorio donde se encuentra este script .py"""
    return os.path.dirname(os.path.abspath(__file__))

# Cargar variables de entorno con la ruta correcta
dir_actual = obtener_ruta_base()
dir_proyecto = os.path.dirname(dir_actual) 
ruta_env = os.path.join(dir_proyecto, '.env')
load_dotenv(ruta_env)

def obtener_timestamp():
    """Retorna un string con la fecha y hora actual en formato YYYY-MM-DD HH-MM-SS"""
    return datetime.now().strftime("%Y-%m-%d %H-%M-%S")

def obtener_ruta_grafico(tipo_grafico):
    """
    Retorna la ruta absoluta para guardar un gráfico específico
    tipo_grafico:'tipos', 'tiempos', 'precision', 'turnos'
    """
    dir_actual = obtener_ruta_base()  
    
    # Mapeo de tipos de gráfico a variables de entorno
    mapeo_env = {
        'tipos': 'GRAFICOS_TIP',
        'tiempos': 'GRAFICOS_TIM',
        'precision': 'GRAFICOS_PRE',
        'turnos': 'GRAFICOS_TUR'
    }
    
    var_env = mapeo_env.get(tipo_grafico)
    if not var_env:
        ruta_grafico = os.path.join(dir_actual, 'data', 'graficos')
    else:
        ruta_relativa = os.getenv(var_env, 'data/graficos')
        if ruta_relativa.startswith('data/'):
            ruta_grafico = os.path.join(dir_actual, ruta_relativa)
        else:
            dir_proyecto = os.path.dirname(dir_actual)
            ruta_grafico = os.path.join(dir_proyecto, ruta_relativa)
    
    os.makedirs(ruta_grafico, exist_ok=True)
    return ruta_grafico

def leer_estadisticas(archivo='estadisticas_partidas.txt'):
    """Lee el archivo de estadísticas y retorna una lista de partidas"""
    partidas = []
    
    dir_actual = obtener_ruta_base()
    dir_proyecto = os.path.dirname(dir_actual)
    
    rutas_posibles = [
        os.path.join(dir_actual, 'data', archivo),  # src/data/
        os.path.join(dir_proyecto, 'data', archivo),  # raiz/data/
        os.path.join(dir_proyecto, archivo),
        os.path.join(dir_actual, archivo),
    ]
    
    ruta_completa = None
    for ruta in rutas_posibles:
        ruta_normalizada = os.path.normpath(ruta)
        print(f"Buscando en: {ruta_normalizada}")
        if os.path.exists(ruta_normalizada):
            ruta_completa = ruta_normalizada
            print(f" Archivo encontrado en: {ruta_completa}\n")
            break
    
    if not ruta_completa:
        print(f"\n Error: No se encontró el archivo '{archivo}' en ninguna ubicación")
        print("\nUbicaciones verificadas:")
        for ruta in rutas_posibles:
            print(f"  - {os.path.normpath(ruta)}")
        return partidas
    
    with open(ruta_completa, 'r', encoding='utf-8') as f:
        for linea in f:
            linea = linea.strip()
            if not linea: continue
            
            patron = r'\((-?\d+),(-?\d+)\),\((-?\d+),(-?\d+)\),\((-?\d+),(-?\d+)\),\((-?\d+),(-?\d+)\),(\w+),(\d+),(\d+)'
            match = re.match(patron, linea)
            
            if match:
                partida = {
                    'j1_aciertos': int(match.group(1)),
                    'j1_fallos': int(match.group(2)),
                    'j2_aciertos': int(match.group(3)),
                    'j2_fallos': int(match.group(4)),
                    'j3_aciertos': int(match.group(5)),
                    'j3_fallos': int(match.group(6)),
                    'j4_aciertos': int(match.group(7)),
                    'j4_fallos': int(match.group(8)),
                    'tipo': match.group(9),
                    'duracion': int(match.group(10)),
                    'turnos': int(match.group(11))
                }
                partidas.append(partida)
    
    return partidas

def calcular_precision(aciertos, fallos):
    if aciertos < 0 or fallos < 0: return None
    total = aciertos + fallos
    return 0 if total == 0 else (aciertos / total) * 100

def graficar_precision_ultima_partida(partidas):
    if not partidas: return
    ultima = partidas[-1]
    fig, ax = plt.subplots(figsize=(10, 6))
    
    if ultima['tipo'] == '1v1':
        jugadores = ['Jugador 1', 'Jugador 2']
        precisiones = [
            calcular_precision(ultima['j1_aciertos'], ultima['j1_fallos']),
            calcular_precision(ultima['j2_aciertos'], ultima['j2_fallos'])
        ]
        colores = ['#3498db', '#e74c3c']
        bars = ax.bar(jugadores, precisiones, color=colores, alpha=0.8, edgecolor='black', linewidth=2)
        
        for bar, prec in zip(bars, precisiones):
            height = bar.get_height()
            ax.text(bar.get_x() + bar.get_width()/2., height,
                   f'{prec:.1f}%', ha='center', va='bottom', fontsize=12, fontweight='bold')
        
        ax.set_title('Precisión Última Partida - 1v1', fontsize=16, fontweight='bold', pad=20)
    else:
        equipos = ['Equipo 1', 'Equipo 2']
        prec_e1_j1 = calcular_precision(ultima['j1_aciertos'], ultima['j1_fallos'])
        prec_e1_j2 = calcular_precision(ultima['j2_aciertos'], ultima['j2_fallos'])
        prec_e2_j1 = calcular_precision(ultima['j3_aciertos'], ultima['j3_fallos'])
        prec_e2_j2 = calcular_precision(ultima['j4_aciertos'], ultima['j4_fallos'])
        
        prec_e1 = (prec_e1_j1 + prec_e1_j2) / 2 if prec_e1_j1 is not None and prec_e1_j2 is not None else 0
        prec_e2 = (prec_e2_j1 + prec_e2_j2) / 2 if prec_e2_j1 is not None and prec_e2_j2 is not None else 0
        
        precisiones = [prec_e1, prec_e2]
        colores = ['#2ecc71', '#f39c12']
        bars = ax.bar(equipos, precisiones, color=colores, alpha=0.8, edgecolor='black', linewidth=2)
        
        for bar, prec in zip(bars, precisiones):
            height = bar.get_height()
            ax.text(bar.get_x() + bar.get_width()/2., height,
                   f'{prec:.1f}%', ha='center', va='bottom', fontsize=12, fontweight='bold')
        
        ax.set_title('Precisión Última Partida - 2v2', fontsize=16, fontweight='bold', pad=20)
    
    ax.set_ylabel('Precisión (%)', fontsize=14, fontweight='bold')
    ax.set_ylim(0, 105)
    ax.grid(axis='y', linestyle='--', alpha=0.7)
    
    plt.tight_layout()
    ruta_guardado = obtener_ruta_grafico('precision')
    timestamp = obtener_timestamp()
    archivo_salida = os.path.join(ruta_guardado, f'{timestamp}.png')
    
    plt.savefig(archivo_salida, dpi=300, bbox_inches='tight')
    plt.close()
    print(f" Gráfico guardado: {archivo_salida}")

def graficar_porcentaje_tipos_partida(partidas):
    if not partidas: return
    tipos = [p['tipo'] for p in partidas]
    count_1v1 = tipos.count('1v1')
    count_2v2 = tipos.count('2v2')
    total_partidas = len(partidas)
    
    fig, ax = plt.subplots(figsize=(10, 8))
    sizes = [count_1v1, count_2v2]
    labels = [f'1v1 ({count_1v1})', f'2v2 ({count_2v2})']
    colors = ['#3498db', '#2ecc71']
    explode = (0.05, 0.05)
    
    wedges, texts, autotexts = ax.pie(sizes, explode=explode, labels=labels, colors=colors,
                                        autopct='%1.1f%%', shadow=True, startangle=90,
                                        textprops={'fontsize': 14, 'fontweight': 'bold'})
    
    ax.set_title('Distribución de Tipos de Partida', fontsize=16, fontweight='bold', pad=20)
    
    # Agregar leyenda con el total de partidas en la esquina superior
    ax.text(1.3, 1.0, f'Total de partidas: {total_partidas}', 
            transform=ax.transAxes,
            fontsize=12, 
            fontweight='bold',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='lightgray', edgecolor='black', linewidth=2),
            verticalalignment='top',
            horizontalalignment='right')
    
    plt.tight_layout()
    ruta_guardado = obtener_ruta_grafico('tipos')
    timestamp = obtener_timestamp()
    archivo_salida = os.path.join(ruta_guardado, f'{timestamp}.png')
    
    plt.savefig(archivo_salida, dpi=300, bbox_inches='tight')
    plt.close()
    print(f" Gráfico guardado: {archivo_salida}")

def graficar_tiempo_ultimas_partidas(partidas, n=10):
    if not partidas: return
    ultimas = partidas[-n:] if len(partidas) >= n else partidas
    
    fig, ax = plt.subplots(figsize=(14, 7))
    numeros = list(range(1, len(ultimas) + 1))
    duraciones = [p['duracion'] for p in ultimas]
    colores = ['#3498db' if p['tipo'] == '1v1' else '#2ecc71' for p in ultimas]
    
    bars = ax.bar(numeros, duraciones, color=colores, alpha=0.8, edgecolor='black', linewidth=1.5)
    
    for bar, dur in zip(bars, duraciones):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height,
               f'{dur}s', ha='center', va='bottom', fontsize=10)
    
    ax.set_title(f'Duración de las Últimas {len(ultimas)} Partidas', fontsize=16, fontweight='bold', pad=20)
    ax.set_xlabel('Número de Partida', fontsize=14, fontweight='bold')
    ax.set_ylabel('Duración (segundos)', fontsize=14, fontweight='bold')
    ax.grid(axis='y', linestyle='--', alpha=0.7)
    
    from matplotlib.patches import Patch
    legend_elements = [Patch(facecolor='#3498db', label='1v1'),
                      Patch(facecolor='#2ecc71', label='2v2')]
    ax.legend(handles=legend_elements, loc='upper right')
    
    plt.tight_layout()
    ruta_guardado = obtener_ruta_grafico('tiempos')
    timestamp = obtener_timestamp()
    archivo_salida = os.path.join(ruta_guardado, f'{timestamp}.png')
    
    plt.savefig(archivo_salida, dpi=300, bbox_inches='tight')
    plt.close()
    print(f" Gráfico guardado: {archivo_salida}")

def graficar_turnos_ultimas_partidas(partidas, n=10):
    if not partidas: return
    ultimas = partidas[-n:] if len(partidas) >= n else partidas
    
    fig, ax = plt.subplots(figsize=(14, 7))
    numeros = list(range(1, len(ultimas) + 1))
    turnos = [p['turnos'] for p in ultimas]
    colores = ['#e74c3c' if p['tipo'] == '1v1' else '#f39c12' for p in ultimas]
    
    bars = ax.bar(numeros, turnos, color=colores, alpha=0.8, edgecolor='black', linewidth=1.5)
    
    for bar, turn in zip(bars, turnos):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height,
               f'{turn}', ha='center', va='bottom', fontsize=10)
    
    ax.set_title(f'Turnos de las Últimas {len(ultimas)} Partidas', fontsize=16, fontweight='bold', pad=20)
    ax.set_xlabel('Número de Partida', fontsize=14, fontweight='bold')
    ax.set_ylabel('Cantidad de Turnos', fontsize=14, fontweight='bold')
    ax.grid(axis='y', linestyle='--', alpha=0.7)
    
    from matplotlib.patches import Patch
    legend_elements = [Patch(facecolor='#e74c3c', label='1v1'),
                      Patch(facecolor='#f39c12', label='2v2')]
    ax.legend(handles=legend_elements, loc='upper right')
    
    plt.tight_layout()
    ruta_guardado = obtener_ruta_grafico('turnos')
    timestamp = obtener_timestamp()
    archivo_salida = os.path.join(ruta_guardado, f'{timestamp}.png')
    
    plt.savefig(archivo_salida, dpi=300, bbox_inches='tight')
    plt.close()
    print(f" Gráfico guardado: {archivo_salida}")

def main():
    print("=== Graficador de Estadísticas de Battleship ===\n")
    partidas = leer_estadisticas()
    
    if not partidas:
        print("No hay datos para graficar.")
        return

    print(f"\nTotal de partidas encontradas: {len(partidas)}")
    print("\nGenerando gráficos...")
    
    graficar_precision_ultima_partida(partidas)
    graficar_porcentaje_tipos_partida(partidas)
    graficar_tiempo_ultimas_partidas(partidas)
    graficar_turnos_ultimas_partidas(partidas)
    
    print(f"\n Todos los gráficos generados correctamente")

if __name__ == "__main__":
    main()