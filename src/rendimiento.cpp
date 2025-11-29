#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <map>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

using namespace std;

// Función para leer variables del .env
map<string, string> leer_env(const string& env_path) {
    map<string, string> env_vars;
    ifstream env(env_path);
    string line;
    while (getline(env, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq != string::npos) {
            string key = line.substr(0, eq);
            string value = line.substr(eq + 1);
            env_vars[key] = value;
        }
    }
    return env_vars;
}

int main(int argc, char* argv[]) {

    #ifdef _WIN32
        printf("[PID: %d]\n", (int)GetCurrentProcessId());
    #else
        printf("[PID: %d]\n", (int)getpid());
    #endif

    cout << "Iniciando prueba de rendimiento..." << endl;
    
    // Verificar argumentos
    if (argc != 2) {
        cerr << "Uso: " << argv[0] << " <n_lote>" << endl;
        return 1;
    }
    
    int n_lote = atoi(argv[1]); // ← Solo 1 argumento (n_lote)

    // Leer variables del .env
    map<string, string> env_vars = leer_env("../.env");

    
    
    // Limpiar espacios en blanco de CANT_THREADS
    env_vars["SIZE_ARRAY_THREADS"].erase(0, env_vars["SIZE_ARRAY_THREADS"].find_first_not_of(" \n\r\t"));
    env_vars["SIZE_ARRAY_THREADS"].erase(env_vars["SIZE_ARRAY_THREADS"].find_last_not_of(" \n\r\t") + 1);

    // Limpiar espacios en blanco de INDICE_INVERT_PARALELO
    env_vars["INDICE_INVERT_PARALELO"].erase(0, env_vars["INDICE_INVERT_PARALELO"].find_first_not_of(" \n\r\t"));
    env_vars["INDICE_INVERT_PARALELO"].erase(env_vars["INDICE_INVERT_PARALELO"].find_last_not_of(" \n\r\t") + 1);
    
    // Limpiar espacios en blanco de NAME_PTRIAL
    env_vars["NAME_PTRIAL"].erase(0, env_vars["NAME_PTRIAL"].find_first_not_of(" \n\r\t"));
    env_vars["NAME_PTRIAL"].erase(env_vars["NAME_PTRIAL"].find_last_not_of(" \n\r\t") + 1);

    env_vars["LIBROS_DIR"].erase(0, env_vars["LIBROS_DIR"].find_first_not_of(" \n\r\t"));
    env_vars["LIBROS_DIR"].erase(env_vars["LIBROS_DIR"].find_last_not_of(" \n\r\t") + 1);
    
    int size_array_threads = stoi(env_vars["SIZE_ARRAY_THREADS"]);
    string index_path = env_vars["INDICE_INVERT_PARALELO"];
    string nombre_indice = env_vars["NAME_PTRIAL"];
    string path_carpeta = env_vars["LIBROS_DIR"];
    
    // Pedir al usuario que ingrese los números de threads
    vector<int> CANT_THREADS;
    cout << "Ingrese " << size_array_threads << " numeros de threads:" << endl;
    
    for (int i = 0; i < size_array_threads; i++) {
        int num_thread;
        cout << "Thread #" << (i + 1) << ": ";
        cin >> num_thread;
        
        if (num_thread <= 0) {
            cerr << "Error: El numero de threads debe ser positivo. Intente nuevamente." << endl;
            i--; // Repetir esta iteración
            continue;
        }
        
        CANT_THREADS.push_back(num_thread);
    }
    
    cout << "\nThreads a probar: ";
    for (int t : CANT_THREADS) cout << t << " ";
    cout << endl;
    cout << "iniciando" << endl;

    for (int num_threads : CANT_THREADS){ // ← Más limpio

        string comando = index_path + " " + nombre_indice + " " + path_carpeta + " " + to_string(num_threads) + " " + to_string(n_lote);
        cout << "Ejecutando: " << comando << endl; 
        system(comando.c_str());

    }
    cout << "Prueba de rendimiento finalizada." << endl;
    
    // Llamar al script de Python para generar el gráfico
    cout << "\nGenerando grafico de rendimiento..." << endl;
    
    string python_script = env_vars["GRAFICADOR_RENDIMIENTO_PATH"];
    
    // Limpiar espacios
    python_script.erase(0, python_script.find_first_not_of(" \n\r\t"));
    python_script.erase(python_script.find_last_not_of(" \n\r\t") + 1);
    
    string comando_python = "python3 \"" + python_script + "\"";
    
    int result = system(comando_python.c_str());
    
    if (result == 0) {
        cout << "Grafico generado exitosamente." << endl;
    } else {
        cerr << "Error al ejecutar el script de Python (codigo: " << result << ")" << endl;
        cerr << "Verifica que Python este instalado y en el PATH" << endl;
    }
    
    return 0;
}
