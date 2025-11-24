#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <regex>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <functional>
#include <chrono>
#include <cstdlib>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

using namespace std;
namespace fs = filesystem;

// --- Estructuras y Alias de Tipo ---

// Un mapa parcial de [id_libro] -> [conteo]
using IndiceParcial = std::map<int, int>;
// El índice global [palabra] -> IndiceParcial
using IndiceGlobal = std::unordered_map<std::string, IndiceParcial>;

// Estructura para el trabajo en la cola
struct TrabajoLibro {
    int id_libro;
    string path_archivo;
};

// Estructura para mantener toda la configuración de la app
struct AppConfig {
    string path_indice;
    string path_mapa;
    string path_log;
    string path_carpeta;
    string path_log_tiempos; 
    int N_THREADS;
    int N_LOTE;
    bool valida = false;
};


long long obtener_timestamp_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

int crear_archivo(const string& nombre_archivo) {
    ofstream archivo(nombre_archivo); // Sobrescribe si existe
    if (!archivo.is_open()) {
        cerr << "Error al crear el archivo: " << nombre_archivo << endl;
        return -1;
    }
    archivo.close();
    return 0;
}

// --- Lógica de Procesamiento de Archivos (sin cambios) ---

void procesar_archivo(
    int libro_id,
    const string& archivo_path,
    IndiceGlobal& indice_invertido, // Usando el alias de tipo
    int& contador_palabras_total
) {
    contador_palabras_total = 0;
    ifstream inFile(archivo_path);
    if (!inFile.is_open()) {
        cerr << "Error al abrir archivo: " << archivo_path << endl;
        return;
    }
    string linea;
    regex caracteres_validos("[A-Za-zÝÉÝÓÚáéíóúÑñÜü]+", regex_constants::ECMAScript);

    while (getline(inFile, linea)) {
        for (sregex_iterator it(linea.begin(), linea.end(), caracteres_validos), end_it; it != end_it; ++it) {
            string palabra = it->str();
            transform(palabra.begin(), palabra.end(), palabra.begin(), ::tolower);
            
            const string letras_validas = "abcdefghijklmnopqrstuvwxyzáéíóúñü";
            bool solo_letras = true;
            for (char c : palabra) {
                if (letras_validas.find(c) == string::npos) {
                    solo_letras = false;
                    break;
                }
            }
            if (solo_letras && !palabra.empty()) {
                indice_invertido[palabra][libro_id]++;
                contador_palabras_total++;
            }
        }
    }
}

// --- Lógica de Escritura de Archivos (sin cambios) ---

void escribir_indice(const string& nombre_indice, const IndiceGlobal& indice_invertido) { // Usando el alias
    ofstream outFile(nombre_indice, ios::app);
    if (!outFile.is_open()) {
        cerr << "Error al abrir el archivo para escribir: " << nombre_indice << endl;
        return;
    }

    vector<pair<string, IndiceParcial>> indice_vec(indice_invertido.begin(), indice_invertido.end());
    sort(indice_vec.begin(), indice_vec.end(), 
        [](const auto& a, const auto& b) { return a.first < b.first; });

    for (const auto& [palabra, libros] : indice_vec) {
        outFile << palabra;

        vector<pair<int, int>> libros_vec(libros.begin(), libros.end());
        sort(libros_vec.begin(), libros_vec.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        for (const auto& [libro_id, count] : libros_vec) {
            outFile << ";(" << libro_id << "," << count << ")";
        }
        outFile << "\n";
    }
    outFile.close();
}


// -----------------------------------------------------------------
// --- NUEVA CLASE WORKER (Inspirada en tu ejemplo 'thread_obj') ---
// -----------------------------------------------------------------
/**
 * @brief Un objeto "callable" que encapsula la lógica del hilo consumidor.
 */
class Worker {
public:
    // El constructor recibe referencias a todos los recursos compartidos
    Worker(
        queue<TrabajoLibro>& cola_trabajo,
        IndiceGlobal& indice_global,
        mutex& mutex_cola,
        mutex& mutex_indice_global,
        condition_variable& cv_cola,
        const atomic<bool>& productor_terminado,
        const int N_LOTE,
        ofstream& log_file_out,
        mutex& mutex_log
    ) : // Los inicializa como miembros de la clase
        cola_trabajo_(cola_trabajo),
        indice_global_(indice_global),
        mutex_cola_(mutex_cola),
        mutex_indice_global_(mutex_indice_global),
        cv_cola_(cv_cola),
        productor_terminado_(productor_terminado),
        N_LOTE_(N_LOTE),
        log_file_out_(log_file_out),
        mutex_log_(mutex_log)
    {}

    // El operador () es lo que ejecutará el std::thread
    void operator()() {
        std::thread::id thread_id = std::this_thread::get_id();

        while (true) {
            vector<TrabajoLibro> lote_trabajo;
            bool hay_trabajo = false;

            // 1. Sección Crítica: Obtener un lote de trabajo
            {
                std::unique_lock<std::mutex> lock(mutex_cola_); // Usa el mutex miembro

                // Usa la CV y la cola miembro
                cv_cola_.wait(lock, [&]() {
                    return !cola_trabajo_.empty() || productor_terminado_;
                });

                while (!cola_trabajo_.empty() && lote_trabajo.size() < (size_t)N_LOTE_) {
                    lote_trabajo.push_back(cola_trabajo_.front());
                    cola_trabajo_.pop();
                }

                if (!lote_trabajo.empty()) {
                    hay_trabajo = true;
                } else if (productor_terminado_) {
                    break; // Salir
                }
            } // Se libera el unique_lock

            // 2. Procesar el lote de trabajo
            if (hay_trabajo) {
                IndiceGlobal indice_local; // Indice local del hilo
                
                for (const auto& job : lote_trabajo) {
                    int contador_palabras = 0;
                    long long t_inicio = obtener_timestamp_ms();

                    procesar_archivo(job.id_libro, job.path_archivo, indice_local, contador_palabras);
                    
                    long long t_termino = obtener_timestamp_ms();

                    // 3. Escribir en el log (Sección Crítica)
                    {
                        lock_guard<mutex> lock_log(mutex_log_); // Usa el mutex de log
                        log_file_out_ << thread_id << ", " << job.id_libro << ", " << contador_palabras << ", " << t_inicio << ", " << t_termino << "\n";
                    }
                }

                // 4. Fusionar con el índice global (Sección Crítica)
                if (!indice_local.empty()) {
                    lock_guard<mutex> lock(mutex_indice_global_); // Usa el mutex global
                    for (const auto& [palabra, libros] : indice_local) {
                        for (const auto& [libro_id, count] : libros) {
                            indice_global_[palabra][libro_id] += count;
                        }
                    }
                }
            }
        }
    }

private:
    // Referencias a todos los recursos compartidos
    queue<TrabajoLibro>& cola_trabajo_;
    IndiceGlobal& indice_global_;
    mutex& mutex_cola_;
    mutex& mutex_indice_global_;
    condition_variable& cv_cola_;
    const atomic<bool>& productor_terminado_;
    const int N_LOTE_;
    ofstream& log_file_out_;
    mutex& mutex_log_;
};


// --- Función principal para crear el índice invertido (Modificada) ---
// Ahora recibe la configuración
void crear_indice(const AppConfig& config) {
    
    // --- Variables compartidas ---
    IndiceGlobal indice_invertido_global;
    queue<TrabajoLibro> cola_trabajo;
    
    mutex mutex_cola;
    mutex mutex_indice_global;
    condition_variable cv_cola;
    atomic<bool> productor_terminado(false);

    // --- Recursos para el Log ---
    ofstream log_file_out(config.path_log, ios::app);
    if (!log_file_out.is_open()) {
        cerr << "Error fatal: No se pudo abrir el archivo de log: " << config.path_log << endl;
        return;
    }
    log_file_out << "thread_id, libro_id, cantidad_palabras, timestamp_inicio_ms, timestamp_termino_ms\n";
    mutex mutex_log;

    // --- Crear el pool de hilos (AHORA MÁS LIMPIO) ---
    cout << "Productor: Lanzando " << config.N_THREADS << " hilos..." << endl;
    vector<thread> hilos;
    for (int i = 0; i < config.N_THREADS; ++i) {
        // En lugar de 9 parámetros, solo creamos el objeto Worker
        hilos.emplace_back(
            Worker(
                cola_trabajo,
                indice_invertido_global,
                mutex_cola,
                mutex_indice_global,
                cv_cola,
                productor_terminado,
                config.N_LOTE,
                log_file_out,
                mutex_log
            )
        );
    }

    // --- Hilo Principal (Productor) ---
    cout << "Productor: Buscando archivos en " << config.path_carpeta << "..." << endl;
    ofstream mapa_file_out(config.path_mapa, ios::app);
    if (!mapa_file_out.is_open()) {
        cerr << "Error fatal: No se pudo abrir el archivo de mapa: " << config.path_mapa << endl;
        productor_terminado = true;
        cv_cola.notify_all();
        for (auto& t : hilos) t.join();
        return;
    }

    int contador_id_libros = 0;
    try {
        for (const auto& entry : fs::directory_iterator(config.path_carpeta)) {
            if (entry.is_regular_file()) {
                int libro_id = ++contador_id_libros;
                string path_libro = entry.path().string();
                string nombre_libro = entry.path().filename().string();

                mapa_file_out << libro_id << ";" << nombre_libro << "\n";

                {
                    lock_guard<mutex> lock(mutex_cola);
                    cola_trabajo.push({libro_id, path_libro});
                }
                cv_cola.notify_one();
            }
        }
    } catch (const fs::filesystem_error& e) {
        cerr << "Error al leer el directorio: " << e.what() << endl;
        productor_terminado = true;
    }
    
    mapa_file_out.close();
    cout << "Productor: " << contador_id_libros << " archivos agregados a la cola." << endl;

    // --- Señal de finalización ---
    productor_terminado = true;
    cv_cola.notify_all();

    // --- Esperar (join) a que todos los hilos terminen ---
    cout << "Productor: Esperando a que terminen los " << config.N_THREADS << " hilos..." << endl;
    for (auto& t : hilos) {
        t.join();
    }
    cout << "Productor: Todos los hilos han terminado." << endl;

    log_file_out.close();

    // --- Escritura Final ---
    cout << "Escribiendo índice final en el archivo: " << config.path_indice << endl;
    escribir_indice(config.path_indice, indice_invertido_global);
}

// --- Función para preparar la configuración ---
AppConfig configurar_aplicacion(int argc, char* argv[]) {
    AppConfig config;

    if (argc != 5) {
        cout << "Uso correcto: " << argv[0] << " <nombre_indice.idx> <ruta_carpetas> <n_threads> <n_lote>" << endl;
        cout << "Ejemplo: " << argv[0] << " mi_indice.idx libros 5 10" << endl;
        cout << "Esto generará: data/mi_indice.idx, data/MAPA-LIBROS.map, data/mi_indice.log" << endl;
        return config;
    }

    // Derivar los paths del argumento base
    string nombre_completo = string(argv[1]);
    config.path_carpeta = argv[2];

    // Parsear N_THREADS y N_LOTE
    int n_threads = stoi(string(argv[3]));
    int n_lote = stoi(string(argv[4]));

    config.N_THREADS = n_threads;
    config.N_LOTE = n_lote;
    
    // Extraer solo el nombre del archivo sin la ruta
    size_t ultima_barra = nombre_completo.find_last_of("/\\");
    string nombre_archivo = (ultima_barra != string::npos) 
        ? nombre_completo.substr(ultima_barra + 1) 
        : nombre_completo;
    
    // Quitar la extensión .idx para derivar el nombre base
    string nombre_base = nombre_archivo;
    if (nombre_archivo.size() >= 4 && nombre_archivo.substr(nombre_archivo.size() - 4) == ".idx") {
        nombre_base = nombre_archivo.substr(0, nombre_archivo.size() - 4);
    }
    
    // Todos los archivos se crean en la carpeta data/
    config.path_indice = "data/" + nombre_archivo;
    config.path_mapa = "data/MAPA-LIBROS.map";
    config.path_log = "data/" + nombre_base + ".log";
    config.path_log_tiempos = "data/logs_tiempos.txt"; 

    cout << "\n--- Configuración ---" << endl;
    cout << "Path Indice: " << config.path_indice << endl;
    cout << "Path Mapa  : " << config.path_mapa << endl;
    cout << "Path Log   : " << config.path_log << endl;
    cout << "Path Log Tiempos: " << config.path_log_tiempos << endl; 
    cout << "Path Libros: " << config.path_carpeta << endl;
    cout << "N_THREADS  : " << config.N_THREADS << endl;
    cout << "N_LOTE     : " << config.N_LOTE << endl;
    cout << "----------------------" << endl;


    #ifdef _WIN32
        printf("[PID: %d]\n", (int)GetCurrentProcessId());
    #else
        printf("[PID: %d]\n", (int)getpid());
    #endif

    config.valida = true;
    return config;
}

// --- Función principal (AHORA MÁS LIMPIA) ---
int main(int argc, char* argv[]) {
    
    // 1. Configurar la aplicación
    AppConfig config = configurar_aplicacion(argc, argv);
    if (!config.valida) {
        return 1;
    }

    // 2. Crear/Limpiar los archivos de salida
    if (crear_archivo(config.path_indice) != 0) return 1;
    if (crear_archivo(config.path_mapa) != 0) return 1;
    if (crear_archivo(config.path_log) != 0) return 1;

    // 3. Ejecutar el proceso y medir tiempo
    auto inicio = chrono::high_resolution_clock::now();
    
    crear_indice(config);
    
    auto fin = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duracion = fin - inicio; // ← Cambiar a milliseconds

    cout << "\n=== Índice invertido creado exitosamente ===" << endl;
    cout << "Tiempo total: " << duracion.count() << " ms" << endl;

    // 4. Escribir en logs_tiempos.txt
    ofstream log_tiempos(config.path_log_tiempos, ios::app); // ← Modo append
    if (log_tiempos.is_open()) {
        log_tiempos << "(" << config.N_THREADS << ", " << duracion.count() << ")" << endl;
        log_tiempos.close();
        cout << "Registro de tiempo guardado en: " << config.path_log_tiempos << endl;
    } else {
        cerr << "Error: No se pudo abrir el archivo de log de tiempos" << endl;
    }

    return 0;
}