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

// --- Nuevos encabezados para concurrencia ---
#include <thread>               // Para std::thread
#include <mutex>                // Para std::mutex y std::lock_guard
#include <condition_variable> // Para std::condition_variable (reemplaza a semaphore)
#include <queue>                // Para std::queue (la cola de trabajo)
#include <atomic>               // Para std::atomic<bool> (bandera de finalización)
#include <functional>           // Para std::ref (pasar referencias a hilos)

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

using namespace std;

// --- Constantes ---
const int NUM_HILOS = 5;

// --- Función para crear un archivo vacío ---
// (Sin cambios)
int crear_archivo(const string& nombre_indice) {
    ofstream indice(nombre_indice);
    if (!indice.is_open()) {
        cerr << "Error al crear el archivo: " << nombre_indice << endl;
        return -1; // Error al crear archivo
    }
    indice.close();
    return 0; // Archivo creado exitosamente
}

// --- Función para procesar un archivo y actualizar el índice invertido ---
// (Sin cambios. Ahora se usará para actualizar un índice LOCAL de cada hilo)
void procesar_archivo(const string& archivo, unordered_map<string, map<string, int>>& indice_invertido) {
    
    ifstream inFile(archivo);
    if (!inFile.is_open()) {
        cerr << "Error al abrir archivo: " << archivo << endl;
        return;
    }
    string linea;
    regex caracteres_validos("[A-Za-zÝÉÝÓÚáéíóúÑñÜü]+", regex_constants::ECMAScript);

    while (getline(inFile, linea)) {
        for (sregex_iterator it(linea.begin(), linea.end(), caracteres_validos), end_it; it != end_it; ++it) {
            string palabra = it->str();
            transform(palabra.begin(), palabra.end(), palabra.begin(), ::tolower);
            // Filtrar palabras que solo contengan letras válidas
            const string letras_validas = "abcdefghijklmnopqrstuvwxyzáéíóúñü";
            bool solo_letras = true;
            for (char c : palabra) {
                if (letras_validas.find(c) == string::npos) {
                    solo_letras = false;
                    break;
                }
            }
            if (solo_letras && !palabra.empty()) {
                indice_invertido[palabra][archivo]++;
            }
        }
    }
}

// --- Función para escribir el índice invertido en un archivo ---
// (Sin cambios. Se llamará una sola vez al final)
void escribir_indice(const string& nombre_indice, const unordered_map<string, map<string, int>>& indice_invertido) {
    ofstream outFile(nombre_indice, ios::app);
    if (!outFile.is_open()) {
        cerr << "Error al abrir el archivo para escribir: " << nombre_indice << endl;
        return;
    }

    // Copiar el mapa a un vector para ordenarlo por palabra (opcional, pero da orden)
    vector<pair<string, map<string, int>>> indice_vec(indice_invertido.begin(), indice_invertido.end());
    sort(indice_vec.begin(), indice_vec.end(), 
        [](const auto& a, const auto& b) { return a.first < b.first; });


    for (const auto& [palabra, archivos] : indice_vec) {
        outFile << palabra;

        // Copiar y ordenar los archivos por cantidad descendente
        vector<pair<string, int>> archivos_vec(archivos.begin(), archivos.end());
        sort(archivos_vec.begin(), archivos_vec.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        for (const auto& [archivo, count] : archivos_vec) {
            filesystem::path p(archivo);
            outFile << ";(" << p.filename().string() << "," << count << ")";
        }
        outFile << "\n";
    }
    outFile.close();
}


// --- Función del Hilo Trabajador (Consumidor) ---
void funcion_worker(
    queue<string>& cola_archivos,
    unordered_map<string, map<string, int>>& indice_global,
    mutex& mutex_cola,
    mutex& mutex_indice_global,
    condition_variable& cv_cola, // Reemplaza al semáforo
    const atomic<bool>& productor_terminado
) {
    while (true) {
        string archivo_a_procesar;
        bool hay_trabajo = false;

        // 2. Sección Crítica: Obtener trabajo de la cola
        {
            // Usamos unique_lock porque .wait() necesita poder bloquear y desbloquear el mutex
            std::unique_lock<std::mutex> lock(mutex_cola);

            // Espera (se duerme) MIENTRAS la cola esté vacía Y el productor NO haya terminado
            cv_cola.wait(lock, [&cola_archivos, &productor_terminado]() {
                return !cola_archivos.empty() || productor_terminado;
            });

            // Si despertamos, puede ser por dos razones:
            // 1. Hay trabajo
            // 2. El productor terminó

            if (!cola_archivos.empty()) {
                archivo_a_procesar = cola_archivos.front();
                cola_archivos.pop();
                hay_trabajo = true;
            } else if (productor_terminado) {
                // La cola está vacía y el productor terminó, salimos del bucle
                break; // Salir del bucle while(true)
            }
        } // El unique_lock (mutex_cola) se libera aquí

        // 4. Procesar el archivo (fuera de cualquier lock)
        if (hay_trabajo) {
            // Se procesa en un índice local para no bloquear el índice global
            unordered_map<string, map<string, int>> indice_local;
            procesar_archivo(archivo_a_procesar, indice_local);

            // 5. Sección Crítica: Fusionar el índice local con el global
            if (!indice_local.empty()) {
                lock_guard<mutex> lock(mutex_indice_global);
                for (const auto& [palabra, archivos] : indice_local) {
                    for (const auto& [archivo, count] : archivos) {
                        indice_global[palabra][archivo] += count;
                    }
                }
            } // El mutex_indice_global se libera aquí
        }
    }
}


// --- Función principal para crear el índice invertido (Modificada) ---
void crear_indice(const string& nombre_indice, const string& path_carpeta) {
    
    // --- Variables compartidas ---
    unordered_map<string, map<string, int>> indice_invertido_global;
    queue<string> cola_archivos;
    mutex mutex_cola;
    mutex mutex_indice_global;
    condition_variable cv_cola; // El reemplazo del semáforo
    atomic<bool> productor_terminado(false); // Bandera para indicar fin

    // --- Crear el pool de hilos ---
    vector<thread> hilos;
    for (int i = 0; i < NUM_HILOS; ++i) {
        hilos.emplace_back(
            funcion_worker,
            ref(cola_archivos),          // Referencia a la cola
            ref(indice_invertido_global), // Referencia al índice global
            ref(mutex_cola),             // Referencia al mutex de la cola
            ref(mutex_indice_global),    // Referencia al mutex del índice
            ref(cv_cola),                // Referencia a la condition variable
            ref(productor_terminado)     // Referencia a la bandera
        );
    }

    // --- Hilo Principal (Productor) ---
    // Recorre el directorio y añade archivos a la cola
    namespace fs = filesystem;
    int archivos_encontrados = 0;
    cout << "Productor: Buscando archivos en " << path_carpeta << "..." << endl;

    for (const auto& entry : fs::directory_iterator(path_carpeta)) {
        if (entry.is_regular_file()) {
            {
                lock_guard<mutex> lock(mutex_cola); // Proteger la cola
                cola_archivos.push(entry.path().string());
            }
            // Señalar que hay una nueva unidad de trabajo (despierta a UN hilo)
            cv_cola.notify_one();
            archivos_encontrados++;
        }
    }
    cout << "Productor: " << archivos_encontrados << " archivos agregados a la cola." << endl;

    // --- Señal de finalización ---
    // Indicar a los hilos que el productor ha terminado
    productor_terminado = true;

    // Despertar a TODOS los hilos (notify_all)
    // Esto asegura que si algún hilo está dormido (cv_cola.wait),
    // se despierte, vea la bandera "productor_terminado" y termine.
    cv_cola.notify_all();

    // --- Esperar (join) a que todos los hilos terminen ---
    cout << "Productor: Esperando a que terminen los " << NUM_HILOS << " hilos..." << endl;
    for (auto& t : hilos) {
        t.join();
    }
    cout << "Productor: Todos los hilos han terminado." << endl;

    // --- Escritura Final ---
    // Ahora que todos los hilos han terminado, el índice global está completo
    // y podemos escribirlo en el archivo de forma segura, sin mutex.
    cout << "Escribiendo índice final en el archivo..." << endl;
    escribir_indice(nombre_indice, indice_invertido_global);
}

// --- Función principal ---
// (Sin cambios)
int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Uso correcto: " << argv[0] << " <nombre_indice.idx> <ruta_carpetas>" << endl;
        return 1;
    }

    string nombre_indice = "data/" + string(argv[1]);
    string path_carpeta = argv[2];

    cout << "\nCreando índice invertido..." << endl;
    #ifdef _WIN32
        printf("[PID: %d]\n", (int)GetCurrentProcessId());
    #else
        printf("[PID: %d]\n", (int)getpid());
    #endif

    // Crear el archivo del índice
    if (crear_archivo(nombre_indice) != 0) {
        return 1; // Error al crear el archivo
    }

    // Crear el índice invertido (ahora en paralelo)
    auto inicio = chrono::high_resolution_clock::now();
    crear_indice(nombre_indice, path_carpeta);
    auto fin = chrono::high_resolution_clock::now();
    chrono::duration<double> duracion = fin - inicio;

    cout << "\n=== Índice invertido creado exitosamente en: " << nombre_indice << " ===" << endl;
    cout << "Tiempo de procesamiento: " << duracion.count() << " segundos" << endl;
    return 0;
}

