#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close


using namespace std;

// Estructura para almacenar el índice invertido (con IDs numéricos)
map<string, vector<pair<int, int>>> indice_invertido; // palabra -> [(ID, frecuencia)]
map<int, string> mapa_libros; // ID -> nombre_libro.txt

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
            // Limpiar espacios y saltos de línea
            value.erase(value.find_last_not_of(" \n\r\t") + 1);
            env_vars[key] = value;
        }
    }
    return env_vars;
}

// Función para cargar el mapa de libros (ID -> nombre)
bool cargar_mapa_libros(const string& mapa_path) {
    ifstream archivo(mapa_path);
    if (!archivo.is_open()) {
        cerr << "Error: No se pudo abrir el archivo de mapa: " << mapa_path << endl;
        return false;
    }

    string linea;
    int libros_cargados = 0;
    
    while (getline(archivo, linea)) {
        if (linea.empty()) continue;
        
        // Formato: ID;nombre_libro.txt
        size_t pos_semicolon = linea.find(';');
        if (pos_semicolon == string::npos) continue;
        
        int id = stoi(linea.substr(0, pos_semicolon));
        string nombre = linea.substr(pos_semicolon + 1);
        
        mapa_libros[id] = nombre;
        libros_cargados++;
    }
    
    archivo.close();
    cout << "Mapa de libros cargado: " << libros_cargados << " libros" << endl;
    return true;
}

// Función para cargar el índice invertido desde archivo
bool cargar_indice(const string& indice_path) {
    ifstream archivo(indice_path);
    if (!archivo.is_open()) {
        cerr << "Error: No se pudo abrir el archivo de índice: " << indice_path << endl;
        return false;
    }

    string linea;
    int lineas_cargadas = 0;
    
    while (getline(archivo, linea)) {
        if (linea.empty()) continue;
        
        // Formato: palabra;(ID,freq);(ID,freq);...
        size_t pos_semicolon = linea.find(';');
        if (pos_semicolon == string::npos) continue;
        
        string palabra = linea.substr(0, pos_semicolon);
        string resto = linea.substr(pos_semicolon + 1);
        
        vector<pair<int, int>> libros; // (ID, frecuencia)
        
        // Parsear cada (ID,frecuencia)
        size_t pos = 0;
        while ((pos = resto.find('(')) != string::npos) {
            size_t pos_cierre = resto.find(')', pos);
            if (pos_cierre == string::npos) break;
            
            string contenido = resto.substr(pos + 1, pos_cierre - pos - 1);
            size_t pos_coma = contenido.find(',');
            if (pos_coma != string::npos) {
                int id = stoi(contenido.substr(0, pos_coma));
                int frecuencia = stoi(contenido.substr(pos_coma + 1));
                libros.push_back({id, frecuencia});
            }
            
            resto = resto.substr(pos_cierre + 1);
        }
        
        if (!libros.empty()) {
            indice_invertido[palabra] = libros;
            lineas_cargadas++;
        }
    }
    
    archivo.close();
    cout << "Índice cargado: " << lineas_cargadas << " palabras únicas" << endl;
    return true;
}

// Función para convertir texto a minúsculas
string a_minusculas(const string& texto) {
    string resultado = texto;
    transform(resultado.begin(), resultado.end(), resultado.begin(), ::tolower);
    return resultado;
}

// Función para buscar con TOPK
string buscar_topk(const string& query, int topk) {
    auto inicio = chrono::high_resolution_clock::now();
    
    // Convertir query a minúsculas y tokenizar
    string query_lower = a_minusculas(query);
    istringstream iss(query_lower);
    vector<string> palabras;
    string palabra;
    
    while (iss >> palabra) {
        palabras.push_back(palabra);
    }
    
    // Acumular frecuencias por ID de libro
    map<int, int> frecuencias_por_id;
    
    for (const string& palabra_buscar : palabras) {
        auto it = indice_invertido.find(palabra_buscar);
        if (it != indice_invertido.end()) {
            for (const auto& par : it->second) {
                int id_libro = par.first;
                int frecuencia = par.second;
                frecuencias_por_id[id_libro] += frecuencia;
            }
        }
    }
    
    // Ordenar por frecuencia descendente
    vector<pair<int, int>> resultados_ids(frecuencias_por_id.begin(), frecuencias_por_id.end());
    sort(resultados_ids.begin(), resultados_ids.end(), 
         [](const pair<int, int>& a, const pair<int, int>& b) {
             return a.second > b.second;
         });
    
    // Tomar solo TOPK
    if (resultados_ids.size() > (size_t)topk) {
        resultados_ids.resize(topk);
    }
    
    auto fin = chrono::high_resolution_clock::now();
    double tiempo_ms = chrono::duration<double, milli>(fin - inicio).count();
    
    // Construir JSON de respuesta con MAPEO INVERSO (ID -> Nombre)
    ostringstream json;
    json << "{\"query\": \"" << query << "\", \"resultados\": [";
    
    for (size_t i = 0; i < resultados_ids.size(); i++) {
        if (i > 0) json << ", ";
        
        int id_libro = resultados_ids[i].first;
        int frecuencia = resultados_ids[i].second;
        
        // MAPEO INVERSO: Buscar nombre del libro por ID
        string nombre_libro = "ID_" + to_string(id_libro); // Por defecto si no se encuentra
        auto it = mapa_libros.find(id_libro);
        if (it != mapa_libros.end()) {
            nombre_libro = it->second;
        }
        
        json << "{\"libro\": \"" << nombre_libro 
             << "\", \"frecuencia\": " << frecuencia << "}";
    }
    
    json << "], \"origen_respuesta\": \"motor\", \"tiempo_motor_ms\": " 
         << tiempo_ms << "}";
    
    return json.str();
}

// Función para parsear JSON simple (extrae query y topk)
bool parsear_json_query(const string& json, string& query, int& topk) {
    size_t pos_query = json.find("\"query\":");
    size_t pos_topk = json.find("\"topk\":");
    
    if (pos_query == string::npos || pos_topk == string::npos) {
        return false;
    }
    
    // Extraer query
    size_t inicio_query = json.find('"', pos_query + 8);
    size_t fin_query = json.find('"', inicio_query + 1);
    if (inicio_query == string::npos || fin_query == string::npos) {
        return false;
    }
    query = json.substr(inicio_query + 1, fin_query - inicio_query - 1);
    
    // Extraer topk
    size_t inicio_topk = json.find_first_of("0123456789", pos_topk + 7);
    size_t fin_topk = json.find_first_of(",}", inicio_topk);
    if (inicio_topk == string::npos || fin_topk == string::npos) {
        return false;
    }
    topk = stoi(json.substr(inicio_topk, fin_topk - inicio_topk));
    
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Uso: ./motor_busqueda <archivo_indice.idx> <mapa_libros.map>" << endl;
        return 1;
    }

    string indice_path = argv[1];
    string mapa_path = argv[2];

    #ifdef _WIN32
        printf("[Motor de Búsqueda - PID: %d]\n", GetCurrentProcessId());
    #else
        printf("[Motor de Búsqueda - PID: %d]\n", getpid());
    #endif

    // Leer variables del .env
    map<string, string> env_vars = leer_env("../.env");
    int puerto = stoi(env_vars["MOTOR_PORT"]);

    // PASO 1: Cargar mapa de libros (ID -> nombre)
    cout << "Cargando mapa de libros..." << endl;
    if (!cargar_mapa_libros(mapa_path)) {
        return 1;
    }

    // PASO 2: Cargar índice invertido
    cout << "Cargando índice invertido..." << endl;
    if (!cargar_indice(indice_path)) {
        return 1;
    }


    // Crear socket servidor
    SOCKET servidor_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor_socket == INVALID_SOCKET) {
        cerr << "Error al crear socket" << endl;
        return 1;
    }

    // Configurar dirección del servidor
    struct sockaddr_in direccion_servidor;
    direccion_servidor.sin_family = AF_INET;
    direccion_servidor.sin_addr.s_addr = INADDR_ANY;
    direccion_servidor.sin_port = htons(puerto);

    // Bind
    if (bind(servidor_socket, (struct sockaddr*)&direccion_servidor, sizeof(direccion_servidor)) == SOCKET_ERROR) {
        cerr << "Error en bind" << endl;
        closesocket(servidor_socket);
        return 1;
    }

    // Listen
    if (listen(servidor_socket, 5) == SOCKET_ERROR) {
        cerr << "Error en listen" << endl;
        closesocket(servidor_socket);
        return 1;
    }

    cout << "Motor de Búsqueda escuchando en puerto " << puerto << "..." << endl;

    // Aceptar conexiones
    while (true) {
        struct sockaddr_in direccion_cliente;
        int tam_direccion = sizeof(direccion_cliente);

        SOCKET cliente_socket = accept(servidor_socket, (struct sockaddr*)&direccion_cliente, (socklen_t*)&tam_direccion);
        
        if (cliente_socket == INVALID_SOCKET) {
            cerr << "Error al aceptar conexión" << endl;
            continue;
        }

        cout << "\n=== Conexión recibida desde Cache ===" << endl;

        // Recibir mensaje
        char buffer[4096] = {0};
        int bytes_recibidos = recv(cliente_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_recibidos > 0) {
            buffer[bytes_recibidos] = '\0';
            string mensaje_recibido(buffer);
            
            cout << "Query recibida: " << mensaje_recibido << endl;
            
            // Parsear JSON
            string query;
            int topk;
            
            if (parsear_json_query(mensaje_recibido, query, topk)) {
                // Realizar búsqueda
                string respuesta = buscar_topk(query, topk);
                
                cout << "Enviando respuesta al Cache..." << endl;
                
                // Enviar respuesta
                send(cliente_socket, respuesta.c_str(), respuesta.length(), 0);
            } else {
                string error = "{\"error\": true, \"mensaje\": \"JSON inválido\"}";
                send(cliente_socket, error.c_str(), error.length(), 0);
            }
        }

        closesocket(cliente_socket);
    }

    closesocket(servidor_socket);

    return 0;
}
