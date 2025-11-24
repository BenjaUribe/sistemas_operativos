#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <map>
#include <vector>
#include <chrono>
#include <sstream>
#include <algorithm>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close

using namespace std;

// Estructura para entrada de cache
struct CacheEntry {
    string query;
    string resultados_json;
    chrono::time_point<chrono::high_resolution_clock> timestamp;
};

// Variables globales
int CACHE_SIZE = 5;
int CACHE_PORT = 8081;
int MOTOR_PORT = 8082;
vector<CacheEntry> cache_memory;

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

// Función para convertir a minúsculas
string a_minusculas(const string& texto) {
    string resultado = texto;
    transform(resultado.begin(), resultado.end(), resultado.begin(), ::tolower);
    return resultado;
}

// Buscar en cache (Cache Hit)
bool buscar_en_cache(const string& query, string& resultado) {
    string query_lower = a_minusculas(query);
    
    for (auto& entry : cache_memory) {
        if (a_minusculas(entry.query) == query_lower) {
            cout << "  \nCACHE HIT para: \"" << query << "\"" << endl;
            resultado = entry.resultados_json;
            entry.timestamp = chrono::high_resolution_clock::now(); // Actualizar LRU
            return true;
        }
    }
    
    cout << "  \nCACHE MISS para: \"" << query << "\"" << endl;
    return false;
}

// Consultar al motor de búsqueda (Cache Miss)
string consultar_motor(const string& query, int topk) {
    cout << "  → Consultando Motor de Búsqueda..." << endl;
    
    
    // Crear socket cliente
    SOCKET motor_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (motor_socket == INVALID_SOCKET) {
        return "{\"error\": true, \"mensaje\": \"Error al conectar con motor\"}";
    }
    
    // Conectar al motor
    struct sockaddr_in direccion_motor;
    direccion_motor.sin_family = AF_INET;
    direccion_motor.sin_port = htons(MOTOR_PORT);
    
    inet_pton(AF_INET, "127.0.0.1", &direccion_motor.sin_addr);

    
    if (connect(motor_socket, (struct sockaddr*)&direccion_motor, sizeof(direccion_motor)) == SOCKET_ERROR) {
        closesocket(motor_socket);
        return "{\"error\": true, \"mensaje\": \"Motor de búsqueda no disponible\"}";
    }
    
    // Construir JSON de consulta
    ostringstream json_query;
    json_query << "{\"query\": \"" << query << "\", \"topk\": " << topk << "}";
    string mensaje = json_query.str();
    
    // Enviar consulta
    cout << "  \nEnviando al motor: " << mensaje << endl;
    send(motor_socket, mensaje.c_str(), mensaje.length(), 0);
    
    // Recibir respuesta
    char buffer[8192] = {0};
    int bytes_recibidos = recv(motor_socket, buffer, sizeof(buffer) - 1, 0);
    
    closesocket(motor_socket);

    if (bytes_recibidos > 0) {
        buffer[bytes_recibidos] = '\0';
        string respuesta_motor(buffer);
        cout << "  \nJSON recibido del motor:" << endl;
        cout << "     " << respuesta_motor << endl;
        return respuesta_motor;
    }
    
    return "{\"error\": true, \"mensaje\": \"Sin respuesta del motor\"}";
}

// Función para limpiar JSON de todos los campos de tiempo
string limpiar_json_tiempos(const string& json) {
    string resultado = json;
    
    // Eliminar tiempo_motor_ms
    size_t pos = resultado.find(", \"tiempo_motor_ms\":");
    if (pos != string::npos) {
        size_t fin = resultado.find_first_of(",}", pos + 1);
        if (fin != string::npos) {
            resultado.erase(pos, fin - pos);
        }
    }
    
    // Eliminar tiempo_cache_ms
    pos = resultado.find(", \"tiempo_cache_ms\":");
    if (pos != string::npos) {
        size_t fin = resultado.find_first_of(",}", pos + 1);
        if (fin != string::npos) {
            resultado.erase(pos, fin - pos);
        }
    }
    
    // Eliminar tiempo_total_ms
    pos = resultado.find(", \"tiempo_total_ms\":");
    if (pos != string::npos) {
        size_t fin = resultado.find_first_of(",}", pos + 1);
        if (fin != string::npos) {
            resultado.erase(pos, fin - pos);
        }
    }
    
    return resultado;
}

// Actualizar cache con nueva entrada (política LRU)
void actualizar_cache(const string& query, const string& resultados_json) {
    // Si cache está llena, eliminar el más antiguo (LRU)
    if (cache_memory.size() >= (size_t)CACHE_SIZE) {
        auto oldest = min_element(cache_memory.begin(), cache_memory.end(),
            [](const CacheEntry& a, const CacheEntry& b) {
                return a.timestamp < b.timestamp;
            });
        
        cout << "  Cache llena. Eliminando entrada: \"" << oldest->query << "\"" << endl;
        cache_memory.erase(oldest);
    }
    
    // Agregar nueva entrada (guardar JSON LIMPIO, sin tiempos)
    CacheEntry nueva_entrada;
    nueva_entrada.query = query;
    nueva_entrada.resultados_json = limpiar_json_tiempos(resultados_json);
    nueva_entrada.timestamp = chrono::high_resolution_clock::now();
    
    cout << "  JSON limpio guardado: " << nueva_entrada.resultados_json << endl;
    
    cache_memory.push_back(nueva_entrada);
    cout << "  Entrada guardada en cache (" << cache_memory.size() << "/" << CACHE_SIZE << ")" << endl;
    
    // Mostrar contenido actual de la cache
    cout << "\n  Estado actual de la Cache:" << endl;
    for (size_t i = 0; i < cache_memory.size(); i++) {
        cout << "     " << (i + 1) << ". Query: \"" << cache_memory[i].query << "\"" << endl;
    }
}

// Agregar tiempos al JSON base (que viene limpio de cache)
string agregar_tiempo_cache(const string& json_base, double tiempo_cache_ms, bool es_hit, double tiempo_motor_ms = 0.0) {
    // El json_base siempre está limpio (sin tiempos)
    size_t pos_cierre = json_base.rfind('}');
    if (pos_cierre == string::npos) return json_base;
    
    ostringstream resultado;
    resultado << json_base.substr(0, pos_cierre);
    
    // Agregar tiempos
    resultado << ", \"tiempo_cache_ms\": " << tiempo_cache_ms;
    
    if (es_hit) {
        // Cache Hit: tiempo motor = 0, solo cache
        resultado << ", \"tiempo_motor_ms\": 0";
        resultado << ", \"tiempo_total_ms\": " << tiempo_cache_ms;
    } else {
        // Cache Miss: sumar cache + motor
        resultado << ", \"tiempo_motor_ms\": " << tiempo_motor_ms;
        resultado << ", \"tiempo_total_ms\": " << (tiempo_cache_ms + tiempo_motor_ms);
    }
    
    resultado << "}";
    return resultado.str();
}

// Parsear JSON para extraer query y topk
bool parsear_json_buscador(const string& json, string& query, int& topk) {
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

int main (int argc, char* argv[]) {

    // Leer variables del archivo .env
    map<string, string> env_vars = leer_env("../.env");
    
    string cache_size_str = env_vars["CACHE_SIZE"];
    cache_size_str.erase(cache_size_str.find_last_not_of(" \n\r\t") + 1);
    CACHE_SIZE = stoi(cache_size_str);
    
    string cache_port_str = env_vars["CACHE_PORT"];
    cache_port_str.erase(cache_port_str.find_last_not_of(" \n\r\t") + 1);
    CACHE_PORT = stoi(cache_port_str);
    
    string motor_port_str = env_vars["MOTOR_PORT"];
    motor_port_str.erase(motor_port_str.find_last_not_of(" \n\r\t") + 1);
    MOTOR_PORT = stoi(motor_port_str);
    
    cout << "=== CACHE MIDDLEWARE ===" << endl;
    cout << "Tamaño de caché: " << CACHE_SIZE << endl;
    cout << "Puerto Cache: " << CACHE_PORT << endl;
    cout << "Puerto Motor: " << MOTOR_PORT << endl;
    
    #ifdef _WIN32
        printf("[PID: %d]\n", GetCurrentProcessId());
    #else
        printf("[PID: %d]\n", getpid());
    #endif


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
    direccion_servidor.sin_port = htons(CACHE_PORT);

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

    cout << "\nCache escuchando en puerto " << CACHE_PORT << "..." << endl;
    cout << "Esperando consultas del Buscador...\n" << endl;

    // Aceptar conexiones
    while (true) {
        struct sockaddr_in direccion_cliente;
        int tam_direccion = sizeof(direccion_cliente);

        SOCKET cliente_socket = accept(servidor_socket, (struct sockaddr*)&direccion_cliente, (socklen_t*)&tam_direccion);

        
        if (cliente_socket == INVALID_SOCKET) {
            cerr << "Error al aceptar conexión" << endl;
            continue;
        }

        cout << "\n=== Consulta recibida del Buscador ===" << endl;
        auto inicio_cache = chrono::high_resolution_clock::now();

        // Recibir mensaje
        char buffer[4096] = {0};
        int bytes_recibidos = recv(cliente_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_recibidos > 0) {
            buffer[bytes_recibidos] = '\0';
            string mensaje_recibido(buffer);
            
            cout << "JSON recibido: " << mensaje_recibido << endl;
            
            // Parsear JSON
            string query;
            int topk;
            
            if (parsear_json_buscador(mensaje_recibido, query, topk)) {
                string respuesta;
                
                // Intentar buscar en cache
                if (buscar_en_cache(query, respuesta)) {
                    // CACHE HIT - respuesta ya está limpia
                    auto fin_cache = chrono::high_resolution_clock::now();
                    double tiempo_cache = chrono::duration<double, milli>(fin_cache - inicio_cache).count();
                    
                    cout << "  JSON base de cache: " << respuesta << endl;
                    respuesta = agregar_tiempo_cache(respuesta, tiempo_cache, true, 0.0);
                    
                    cout << " \nJSON final " << endl;
                    cout << "     " << respuesta << endl;
                } else {
                    // CACHE MISS - consultar motor
                    string respuesta_motor = consultar_motor(query, topk);
                    
                    // Extraer tiempo del motor del JSON
                    double tiempo_motor = 0.0;
                    size_t pos_tiempo = respuesta_motor.find("\"tiempo_motor_ms\":");
                    if (pos_tiempo != string::npos) {
                        size_t inicio = respuesta_motor.find_first_of("0123456789.", pos_tiempo);
                        size_t fin = respuesta_motor.find_first_of(",}", inicio);
                        if (inicio != string::npos && fin != string::npos) {
                            tiempo_motor = stod(respuesta_motor.substr(inicio, fin - inicio));
                        }
                    }
                    
                    auto fin_cache = chrono::high_resolution_clock::now();
                    double tiempo_cache = chrono::duration<double, milli>(fin_cache - inicio_cache).count();
                    
                    // Guardar en cache (se limpiará automáticamente)
                    cout << "  Guardando en cache..." << endl;
                    actualizar_cache(query, respuesta_motor);
                    
                    // Limpiar JSON y agregar tiempos correctos
                    string json_limpio = limpiar_json_tiempos(respuesta_motor);
                    cout << "  JSON limpio: " << json_limpio << endl;
                    respuesta = agregar_tiempo_cache(json_limpio, tiempo_cache, false, tiempo_motor);
                    
                    cout << " \nJSON final " << endl;
                    cout << "     " << respuesta << endl;
                }
                
                cout << " Enviando respuesta al Buscador" << endl;
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
