# Sistemas Operativos
**Proyecto asignatura INFO198 (Sistemas Operativos)**

---

##  Tabla de Contenidos
- [Descripción](#descripción)
- [Requisitos](#requisitos)
- [Instalación](#instalación)
- [Componentes del Sistema](#componentes-del-sistema)
  - [Menú Principal](#menú-principal)
  - [Administrador de Usuarios](#administrador-de-usuarios)
  - [Sistema de Búsqueda con Caché](#sistema-de-búsqueda-con-caché)
  - [Juego Battleship Multijugador](#juego-battleship-multijugador)
  - [Multiplicación de Matrices](#multiplicación-de-matrices)
  - [Índice Invertido](#índice-invertido)
- [Variables de Entorno](#variables-de-entorno)
- [Estructura del Proyecto](#estructura-del-proyecto)

---

##  Descripción

Sistema operativo que integra múltiples funcionalidades:
- Gestión de usuarios y perfiles
- Sistema de búsqueda en libros con caché LRU
- Juego multijugador Battleship
- Procesamiento paralelo de índices invertidos
- Multiplicación de matrices
- Generación de estadísticas y gráficos

---

##  Requisitos

- **Compilador**: `g++` con soporte para C++11 o superior
- **Build Tool**: `make`
- **Sistema Operativo**: Linux/Unix o Windows con WSL
- **Python**: Para generación de gráficos (opcional)
- 

---

##  Instalación
```bash
sudo apt install python3-dotenv
sudo apt install python3-matplotlib
```
### Compilación
```bash
cd src
make
```

Esto generará los siguientes ejecutables:
- `menu` - Menú principal del sistema
- `user_admin` - Administrador de usuarios
- `buscador_sistOpe` - Cliente del sistema de búsqueda
- `cache` - Servidor caché
- `motor_busqueda` - Motor de búsqueda
- `battleship_server` - Servidor del juego
- `battleship_client` - Cliente del juego
- `matmul` - Multiplicador de matrices
- `indice_invertido` - Generador de índice invertido
- `indice_paralelo` - Generador de índice invertido paralelo
- `rendimiento` - Prueba de rendimiento para índice paralelo

---

##  Componentes del Sistema

### Menú Principal
Punto de entrada principal del sistema que integra todas las funcionalidades.

```bash
./menu -u <username> -p <password> -f <ruta_de_archivo_contador_palabras>
```

### Administrador de Usuarios
Gestión de usuarios y perfiles del sistema.

```bash
./user_admin
```

**Funcionalidades:**
- Crear nuevos usuarios
- Listar usuarios existentes
- Eliminar usuarios

### Sistema de Búsqueda con Caché
Sistema distribuido de búsqueda en libros con middleware de caché LRU.

**Arquitectura:**
```
Buscador (Cliente) -> Cache (Servidor:8081) -> Motor (Servidor:8082)
```

**Ejecución (requiere 3 terminales):**

1. **Terminal 1 - Motor de Búsqueda:**
   ```bash
   ./motor_busqueda <direccion_archivo_indice_invertido> <direccion_mapa_libros>
   ```

2. **Terminal 2 - Servidor Caché:**
   ```bash
   ./cache
   ```

3. **Terminal 3 - Cliente Buscador:**
   ```bash
   ./buscador_sistOpe
   ```
   O ejecutarlo desde el menú principal.

**Características:**
- Caché LRU (Least Recently Used) con tamaño configurable
- Búsqueda de palabras y frases en colección de libros
- Resultados TOPK por frecuencia
- Métricas de tiempo (caché, motor, total)
- Comunicación por sockets con JSON
- Búsquedas continuas (escribe `-1` para terminar)

### Juego Battleship Multijugador
Implementación del clásico juego de batalla naval con soporte multijugador.

**Ejecución:**

1. **Iniciar servidor:**
   ```bash
   ./battleship_server
   ```
   O iniciarlo desde el menú principal.

2. **Conectar jugadores (en terminales separadas):**
   ```bash
   ./battleship_client
   ```

**Características:**
- Partidas multijugador
- Estadísticas de juego
- Generación de gráficos de rendimiento

### Multiplicación de Matrices
Herramienta para multiplicación de matrices.

```bash
./matmul
```

### Índice Invertido
Generación de índice invertido para búsqueda de libros.

**Versión secuencial:**
```bash
./indice_invertido
```

**Versión paralela:**
```bash
./indice_paralelo
```

---

##  Variables de Entorno

El archivo `.env` en la raíz del proyecto contiene las siguientes configuraciones:

### Sistema de Usuarios
- **`USER_FILE`**: Ruta al archivo `USUARIOS.txt` con datos de usuarios
- **`PERFIL_FILE`**: Ruta al archivo `PERFILES.txt` con perfiles disponibles
- **`ADMIN_SYS`**: Ruta al ejecutable `user_admin`

### Sistema de Búsqueda
- **`LIBROS_DIR`**: Carpeta donde se almacenan los archivos de libros
- **`CREATE_INDEX`**: Ruta al ejecutable `indice_invertido`
- **`INDICE_INVERT_PARALELO`**: Ruta al ejecutable `indice_paralelo`
- **`CACHE_SIZE`**: Tamaño de la caché LRU (por defecto: 5)
- **`TOPK`**: Número de resultados a mostrar (por defecto: 3)
- **`CACHE_PORT`**: Puerto del servidor caché (por defecto: 8081)
- **`MOTOR_PORT`**: Puerto del motor de búsqueda (por defecto: 8082)
- **`SEARCH_APP`**: Ruta aplicación motor de busqueda

### Multiplicación de Matrices
- **`MATRIZ1_FILE`**: Ruta a `matriz1.txt`
- **`MATRIZ2_FILE`**: Ruta a `matriz2.txt`
- **`MUTLI_M`**: Ruta al ejecutable `matmul`

### Juego Battleship
- **`GAME_APP`**: Ruta al ejecutable `battleship_server`
- **`GRAFICADOR_ESTADISTICAS`**: Script Python para gráficos de estadísticas
- **`GRAFICOS_TIP`**: Carpeta para gráficos de tipo de partida
- **`GRAFICOS_TIM`**: Carpeta para gráficos de tiempo de partidas
- **`GRAFICOS_PRE`**: Carpeta para gráficos de precisión
- **`GRAFICOS_TUR`**: Carpeta para gráficos de turnos

### Pruebas de Rendimiento
- **`PERFORMANCE_TEST`**: Ruta al ejecutable de pruebas de rendimiento
- **`SIZE_ARRAY_THREADS`**: Número de hilos para pruebas paralelas
- **`NAME_PTRIAL`**: Nombre del archivo de salida de pruebas
- **`GRAFICADOR_RENDIMIENTO_PATH`**: Script Python para gráficos de rendimiento
- **`GRAFICOS_REN`**: Carpeta para gráficos de rendimiento

---

##  Estructura del Proyecto

```
sistemas_operativos/
├── .env                          # Variables de entorno
├── README.md                     # Este archivo
├── src/
│   ├── makefile                  # Archivo de compilación
│   ├── menu_principal.cpp        # Menú principal
│   ├── user_admin.cpp            # Administrador de usuarios
│   ├── buscador_sistOpe.cpp      # Cliente del buscador
│   ├── cache.cpp                 # Servidor caché
│   ├── motor_busqueda.cpp        # Motor de búsqueda
│   ├── indice_invertido.cpp      # Generador de índice
│   ├── indice_paralelo.cpp       # Generador paralelo
│   ├── matmul.cpp                # Multiplicador de matrices
│   └── battleship/               # Código del juego
│       ├── battleship.h
│       ├── server.cpp
│       ├── client.cpp
│       └── ...
└── data/
    ├── USUARIOS.txt              # Base de datos de usuarios
    ├── PERFILES.txt              # Perfiles del sistema
    ├── indice.idx                # Índice invertido
    ├── MAPA-LIBROS.map           # Mapeo ID → nombre de libro
    ├── libros/                   # Colección de libros
    │   ├── el_principito.txt
    │   ├── don_quijote.txt
    │   └── ...
    └── matrices/                 # Archivos de matrices
        ├── matriz1.txt
        └── matriz2.txt
```






