# Sistema de Búsqueda con Cache y Motor de Búsqueda

## Arquitectura del Sistema

```
Buscador (Cliente)  →  Cache (Servidor/Middleware)  →  Motor de Búsqueda (Servidor)
     ↓                           ↓                              ↓
Puerto Local          Puerto 8080 (CACHE_PORT)      Puerto 8081 (MOTOR_PORT)
```

### Flujo de Comunicación:

1. **Buscador** envía query a **Cache** via socket
2. **Cache** verifica si tiene la respuesta en memoria:
   - **Cache Hit**: Responde directamente al Buscador
   - **Cache Miss**: Consulta al **Motor de Búsqueda**
3. **Motor** carga el índice, busca TOPK resultados y responde a Cache
4. **Cache** guarda la respuesta (política LRU), suma tiempos y responde al Buscador

## Formato de Mensajes JSON

### Consulta (Buscador → Cache → Motor):
```json
{"query": "El anillo", "topk": 3}
```

### Respuesta (Motor → Cache → Buscador):

**Cache Hit:**
```json
{
  "query": "El anillo",
  "resultados": [
    {"libro": "El_señor_de_los_anillos_2.txt", "frecuencia": 142},
    {"libro": "El_Hobbit.txt", "frecuencia": 89}
  ],
  "origen_respuesta": "cache",
  "tiempo_cache_ms": 0.523,
  "tiempo_total_ms": 0.523
}
```

**Cache Miss:**
```json
{
  "query": "El anillo",
  "resultados": [
    {"libro": "El_señor_de_los_anillos_2.txt", "frecuencia": 142},
    {"libro": "El_Hobbit.txt", "frecuencia": 89}
  ],
  "origen_respuesta": "motor",
  "tiempo_cache_ms": 1.2,
  "tiempo_motor_ms": 145.8,
  "tiempo_total_ms": 147.0
}
```

## Configuración (.env)

```
CACHE_SIZE=5      # Capacidad máxima de la cache
TOPK=3            # Número de resultados a retornar
CACHE_PORT=8080   # Puerto del servidor Cache
MOTOR_PORT=8081   # Puerto del servidor Motor
```

## Compilación

### Compilar todo el sistema:
```bash
cd src
make buscador_sistema
```

### Compilar componentes individuales:
```bash
make motor_busqueda   # Motor de búsqueda
make cache            # Cache middleware
make buscador_sistope # Buscador cliente
```

## Ejecución

### Paso 1: Iniciar el Motor de Búsqueda (Terminal 1)
```bash
cd src
./motor_busqueda data/indice.idx data/MAPA-LIBROS.map
```

Salida esperada:
```
[Motor de Búsqueda - PID: XXXX]
Cargando mapa de libros...
Mapa de libros cargado: 143 libros
Cargando índice invertido...
Índice cargado: 144533 palabras únicas
Motor de Búsqueda escuchando en puerto 8081...
```

### Paso 2: Iniciar la Cache (Terminal 2)
```bash
cd src
./cache
```

Salida esperada:
```
=== CACHE MIDDLEWARE ===
Tamaño de caché: 5
Puerto Cache: 8080
Puerto Motor: 8081
[PID: XXXX]

Cache escuchando en puerto 8080...
Esperando consultas del Buscador...
```

### Paso 3: Ejecutar el Buscador (Terminal 3)
```bash
cd src
./buscador_sistOpe data/indice.idx
```

Luego ingrese su búsqueda cuando se solicite:
```
Ingrese la palabra/frase a buscar: El anillo
```

## Ejemplo de Ejecución

### Primera búsqueda (Cache Miss):
```
=== BUSCADOR DE LIBROS ===
[PID: 1234]
Cache Puerto: 8080
TOPK: 3

Ingrese la palabra/frase a buscar: El anillo

→ Enviando consulta a Cache...

============================================================
           RESULTADOS DE LA BÚSQUEDA
============================================================

🔍 Query: "El anillo"
🔧 Origen: MOTOR DE BÚSQUEDA (Miss)

📚 Libros encontrados:
------------------------------------------------------------
   1. El_señor_de_los_anillos_2.txt (frecuencia: 142)
   2. El_Hobbit.txt (frecuencia: 89)
   3. El_Senor_de_los_Anillos_3.txt (frecuencia: 67)

⏱️  Tiempos de ejecución:
------------------------------------------------------------
   Cache: 1.234 ms
   Motor: 145.678 ms
   TOTAL: 146.912 ms
============================================================

Tiempo total del cliente: 147.523 ms
```

### Segunda búsqueda idéntica (Cache Hit):
```
Ingrese la palabra/frase a buscar: El anillo

→ Enviando consulta a Cache...

============================================================
           RESULTADOS DE LA BÚSQUEDA
============================================================

🔍 Query: "El anillo"
📦 Origen: CACHE (Hit)

📚 Libros encontrados:
------------------------------------------------------------
   1. El_señor_de_los_anillos_2.txt (frecuencia: 142)
   2. El_Hobbit.txt (frecuencia: 89)
   3. El_Senor_de_los_Anillos_3.txt (frecuencia: 67)

⏱️  Tiempos de ejecución:
------------------------------------------------------------
   Cache: 0.523 ms
   TOTAL: 0.523 ms
============================================================

Tiempo total del cliente: 1.234 ms
```

## Componentes del Sistema

### 1. Motor de Búsqueda (`motor_busqueda.cpp`)
- Servidor socket en MOTOR_PORT (8081)
- Carga el índice invertido en memoria
- Implementa búsqueda TOPK
- Mapea IDs a nombres reales de libros
- Calcula y reporta tiempo de procesamiento

### 2. Cache (`cache.cpp`)
- Servidor socket en CACHE_PORT (8080) - recibe del Buscador
- Cliente socket al MOTOR_PORT (8081) - consulta al Motor
- Implementa caché LRU con tamaño CACHE_SIZE
- Distingue entre Cache Hit y Cache Miss
- Acumula tiempos de procesamiento

### 3. Buscador (`buscador_sistOpe.cpp`)
- Cliente socket que se conecta a CACHE_PORT
- Envía queries en formato JSON
- Parsea y muestra resultados formateados
- Mide tiempos extremo a extremo

## Política de Caché (LRU)

Cuando la caché alcanza CACHE_SIZE:
1. Se elimina la entrada menos recientemente usada
2. Se guarda la nueva entrada
3. El timestamp se actualiza en cada acceso (lectura)

## Notas Importantes

- Los tres procesos deben estar ejecutándose simultáneamente
- El Motor debe iniciarse PRIMERO (antes de la Cache)
- La Cache debe estar lista ANTES de ejecutar el Buscador
- Los puertos 8080 y 8081 deben estar disponibles
- El archivo de índice debe existir en `data/indice.idx`

## Depuración

Si hay problemas de conexión:
1. Verificar que los procesos estén ejecutándose
2. Verificar que los puertos no estén ocupados: `netstat -an | findstr "8080 8081"`
3. Revisar la configuración en `.env`
4. Verificar que el índice esté correctamente generado

## Limpieza

```bash
make clean
```

Esto elimina todos los ejecutables generados.
