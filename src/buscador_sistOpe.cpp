#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <filesystem>
#include <sstream>
#include <vector>
#include <chrono>
#include <map>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close


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
            value.erase(value.find_last_not_of(" \n\r\t") + 1);
            env_vars[key] = value;
        }
    }
    return env_vars;
}

// Función para realizar la consulta a la cache
string query_cache(const string& query, int topk, int cache_port) {
    
    // Crear socket cliente
    SOCKET cache_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (cache_socket == INVALID_SOCKET) {
        return "{\"error\": true, \"mensaje\": \"Error al crear socket\"}";
    }
    
    // Conectar a la cache
    struct sockaddr_in direccion_cache;
    direccion_cache.sin_family = AF_INET;
    direccion_cache.sin_port = htons(cache_port);

    inet_pton(AF_INET, "127.0.0.1", &direccion_cache.sin_addr);

    
    if (connect(cache_socket, (struct sockaddr*)&direccion_cache, sizeof(direccion_cache)) == SOCKET_ERROR) {
        closesocket(cache_socket);
        return "{\"error\": true, \"mensaje\": \"No se pudo conectar a la Cache. Asegúrese de que esté ejecutándose.\"}";
    }
    
    // Construir JSON de consulta
    ostringstream json_query;
    json_query << "{\"query\": \"" << query << "\", \"topk\": " << topk << "}";
    string mensaje = json_query.str();
    
    cout << "\n Enviando consulta a Cache..." << endl;
    
    // Enviar consulta
    send(cache_socket, mensaje.c_str(), mensaje.length(), 0);
    
    // Recibir respuesta
    char buffer[8192] = {0};
    int bytes_recibidos = recv(cache_socket, buffer, sizeof(buffer) - 1, 0);
    
    closesocket(cache_socket);

    
    if (bytes_recibidos > 0) {
        buffer[bytes_recibidos] = '\0';
        return string(buffer);
    }
    
    return "{\"error\": true, \"mensaje\": \"Sin respuesta de la Cache\"}";
}

// Función para mostrar los resultados parseando el JSON
void mostrar_resultados(const string& json_respuesta) {
    cout << "\n" << string(60, '=') << endl;
    cout << "           RESULTADOS DE LA BÚSQUEDA" << endl;
    cout << string(60, '=') << endl;
    
    // Verificar si hay error
    if (json_respuesta.find("\"error\": true") != string::npos) {
        size_t pos_mensaje = json_respuesta.find("\"mensaje\":");
        if (pos_mensaje != string::npos) {
            size_t inicio = json_respuesta.find('"', pos_mensaje + 10);
            size_t fin = json_respuesta.find('"', inicio + 1);
            if (inicio != string::npos && fin != string::npos) {
                string mensaje_error = json_respuesta.substr(inicio + 1, fin - inicio - 1);
                cout << "\n❌ ERROR: " << mensaje_error << endl;
            }
        }
        cout << string(60, '=') << "\n" << endl;
        return;
    }
    
    // Extraer query
    size_t pos_query = json_respuesta.find("\"query\":");
    if (pos_query != string::npos) {
        size_t inicio = json_respuesta.find('"', pos_query + 8);
        size_t fin = json_respuesta.find('"', inicio + 1);
        if (inicio != string::npos && fin != string::npos) {
            string query = json_respuesta.substr(inicio + 1, fin - inicio - 1);
            cout << "\n🔍 Query: \"" << query << "\"" << endl;
        }
    }
    
    // Extraer origen
    size_t pos_origen = json_respuesta.find("\"origen_respuesta\":");
    if (pos_origen != string::npos) {
        size_t inicio = json_respuesta.find('"', pos_origen + 19);
        size_t fin = json_respuesta.find('"', inicio + 1);
        if (inicio != string::npos && fin != string::npos) {
            string origen = json_respuesta.substr(inicio + 1, fin - inicio - 1);
            if (origen == "cache") {
                cout << "Origen: CACHE (Hit)" << endl;
            } else {
                cout << "Origen: MOTOR DE BÚSQUEDA (Miss)" << endl;
            }
        }
    }
    
    // Extraer resultados
    cout << "\nLibros encontrados:" << endl;
    cout << string(60, '-') << endl;
    
    size_t pos_resultados = json_respuesta.find("\"resultados\":");
    if (pos_resultados != string::npos) {
        size_t pos_bracket = json_respuesta.find('[', pos_resultados);
        size_t pos_bracket_cierre = json_respuesta.find(']', pos_bracket);
        
        if (pos_bracket != string::npos && pos_bracket_cierre != string::npos) {
            string resultados_str = json_respuesta.substr(pos_bracket + 1, pos_bracket_cierre - pos_bracket - 1);
            
            if (resultados_str.find('{') == string::npos) {
                cout << "   (No se encontraron resultados)" << endl;
            } else {
                int contador = 1;
                size_t pos = 0;
                
                while ((pos = resultados_str.find('{', pos)) != string::npos) {
                    size_t pos_cierre = resultados_str.find('}', pos);
                    if (pos_cierre == string::npos) break;
                    
                    string resultado = resultados_str.substr(pos, pos_cierre - pos + 1);
                    
                    // Extraer libro
                    size_t pos_libro = resultado.find("\"libro\":");
                    string libro = "";
                    if (pos_libro != string::npos) {
                        size_t inicio = resultado.find('"', pos_libro + 8);
                        size_t fin = resultado.find('"', inicio + 1);
                        if (inicio != string::npos && fin != string::npos) {
                            libro = resultado.substr(inicio + 1, fin - inicio - 1);
                        }
                    }
                    
                    // Extraer frecuencia
                    size_t pos_freq = resultado.find("\"frecuencia\":");
                    int frecuencia = 0;
                    if (pos_freq != string::npos) {
                        size_t inicio = resultado.find_first_of("0123456789", pos_freq);
                        size_t fin = resultado.find_first_of(",}", inicio);
                        if (inicio != string::npos && fin != string::npos) {
                            frecuencia = stoi(resultado.substr(inicio, fin - inicio));
                        }
                    }
                    
                    cout << "   " << contador << ". " << libro << " (frecuencia: " << frecuencia << ")" << endl;
                    
                    pos = pos_cierre + 1;
                    contador++;
                }
            }
        }
    }
    
    // Extraer tiempos
    cout << "\n  Tiempos de ejecución:" << endl;
    cout << string(60, '-') << endl;
    
    // Tiempo cache
    size_t pos_tiempo_cache = json_respuesta.find("\"tiempo_cache_ms\":");
    if (pos_tiempo_cache != string::npos) {
        size_t inicio = json_respuesta.find_first_of("0123456789.", pos_tiempo_cache);
        size_t fin = json_respuesta.find_first_of(",}", inicio);
        if (inicio != string::npos && fin != string::npos) {
            string tiempo = json_respuesta.substr(inicio, fin - inicio);
            cout << "   Cache: " << tiempo << " ms" << endl;
        }
    }
    
    // Tiempo motor
    size_t pos_tiempo_motor = json_respuesta.find("\"tiempo_motor_ms\":");
    if (pos_tiempo_motor != string::npos) {
        size_t inicio = json_respuesta.find_first_of("0123456789.", pos_tiempo_motor);
        size_t fin = json_respuesta.find_first_of(",}", inicio);
        if (inicio != string::npos && fin != string::npos) {
            string tiempo = json_respuesta.substr(inicio, fin - inicio);
            cout << "   Motor: " << tiempo << " ms" << endl;
        }
    }
    
    // Tiempo total
    size_t pos_tiempo_total = json_respuesta.find("\"tiempo_total_ms\":");
    if (pos_tiempo_total != string::npos) {
        size_t inicio = json_respuesta.find_first_of("0123456789.", pos_tiempo_total);
        size_t fin = json_respuesta.find_first_of(",}", inicio);
        if (inicio != string::npos && fin != string::npos) {
            string tiempo = json_respuesta.substr(inicio, fin - inicio);
            cout << "   TOTAL: " << tiempo << " ms" << endl;
        }
    }
    
    cout << string(60, '=') << "\n" << endl;
}

int main (int argc, char* argv[]) {
    cout << ":::::::::: Buscador de Libros con Caché LRU ::::::::::" << endl;
    #ifdef _WIN32
        printf("[PID: %d]\n", GetCurrentProcessId());
    #else
        printf("[PID: %d]\n", getpid());
    #endif

    // Leer configuración del .env
    map<string, string> env_vars = leer_env("../.env");
    
    int cache_port = stoi(env_vars["CACHE_PORT"]);
    int topk = stoi(env_vars["TOPK"]);
    
    cout << "Cache Puerto: " << cache_port << endl;
    cout << "TOPK: " << topk << endl;

    // Bucle principal para realizar múltiples búsquedas
    while (true) {
        cout  << "\n" << string(60, '=') << endl;
        cout << "Ingrese la palabra/frase a buscar (o '-1' para terminar): ";
        string busqueda;
        getline(cin, busqueda);
        
        // Verificar si el usuario quiere salir
        if (busqueda == "-1") {
            cout << "\nSaliendo del buscador..." << endl;
            break;
        }
        
        if (busqueda.empty()) {
            cout << "Error: Debe ingresar una búsqueda válida" << endl;
            continue;
        }
        
        // Realizar consulta a la cache
        auto inicio = chrono::high_resolution_clock::now();
        string respuesta = query_cache(busqueda, topk, cache_port);
        auto fin = chrono::high_resolution_clock::now();
        
        double tiempo_total = chrono::duration<double, milli>(fin - inicio).count();
        
        // Mostrar resultados
        mostrar_resultados(respuesta);
        
        cout << "Tiempo total del cliente: " << tiempo_total << " ms" << endl;
    }

    return 0;
}
