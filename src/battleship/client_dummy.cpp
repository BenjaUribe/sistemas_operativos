#include "battleship.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>

// Constantes del cliente dummy
const string DEFAULT_SERVER_IP = "127.0.0.1";
const int DEFAULT_SERVER_PORT = 8080;

// === IMPLEMENTACIÓN DE FUNCIONES DE RED ===

bool sendGameMessage(int socket, const GameMessage& msg) {
    int total_sent = 0;
    int message_size = sizeof(GameMessage);
    const char* buffer = (const char*)&msg;
    
    // Enviar datos hasta completar el mensaje
    while (total_sent < message_size) {
        int bytes_sent = send(socket, buffer + total_sent, message_size - total_sent, 0);
        
        if (bytes_sent > 0) {
            total_sent += bytes_sent;
        } else {
            cout << "❌ Error enviando mensaje" << endl;
            return false;
        }
    }
    
    return true;
}

bool receiveGameMessage(int socket, GameMessage& msg) {
    int total_received = 0;
    int message_size = sizeof(GameMessage);
    char* buffer = (char*)&msg;
    
    // Recibir datos hasta completar el mensaje
    while (total_received < message_size) {
        int bytes_received = recv(socket, buffer + total_received, message_size - total_received, 0);
        
        if (bytes_received > 0) {
            total_received += bytes_received;
        } else if (bytes_received == 0) {
            cout << "❌ Servidor cerró la conexión" << endl;
            return false;
        } else {
            cout << "❌ Error recibiendo mensaje" << endl;
            return false;
        }
    }
    
    return true;
}

int main(int argc, char* argv[]) {
    // Inicializar semilla para números aleatorios
    srand(time(nullptr));
    
    string player_name = "DummyBot";
    if (argc > 1) {
        player_name = argv[1]; // Permitir nombre personalizado
    }
    
    cout << "🤖 CLIENTE DUMMY: " << player_name << endl;
    cout << "Conectando al servidor..." << endl;
    
    // Crear socket y conectar
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        cout << "❌ Error creando socket" << endl;
        return 1;
    }
    
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(DEFAULT_SERVER_PORT);
    inet_pton(AF_INET, DEFAULT_SERVER_IP.c_str(), &server_address.sin_addr);
    
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        cout << "❌ Error conectando al servidor" << endl;
        close(client_socket);
        return 1;
    }
    
    cout << "✅ Conectado al servidor" << endl;
    
    // Recibir mensaje de bienvenida
    GameMessage welcome_msg;
    if (receiveGameMessage(client_socket, welcome_msg) && welcome_msg.type == MSG_WELCOME) {
        cout << "📩 " << welcome_msg.text << " (ID: " << welcome_msg.player_id << ")" << endl;
    }
    
    // Enviar nombre
    GameMessage name_msg(MSG_SET_NAME);
    strncpy(name_msg.text, player_name.c_str(), sizeof(name_msg.text) - 1);
    sendGameMessage(client_socket, name_msg);
    cout << "📝 Nombre enviado: " << player_name << endl;
    
    // === LÓGICA DEL JUEGO DUMMY ===
    
    // Posiciones predefinidas para los barcos (bien separadas)
    int ship_positions[NUM_SHIPS][3] = {
        {0, 0, HORIZONTAL},  // Barco 1: (0,0) horizontal - ocupa (0,0)(0,1)
        {2, 0, HORIZONTAL},  // Barco 2: (2,0) horizontal - ocupa (2,0)(2,1)  
        {4, 0, HORIZONTAL}   // Barco 3: (4,0) horizontal - ocupa (4,0)(4,1)
    };
    
    int ships_placed = 0;
    bool game_active = true;
    
    while (game_active) {
        GameMessage server_msg;
        
        if (receiveGameMessage(client_socket, server_msg)) {
            switch (server_msg.type) {
                case MSG_WAIT: {
                    cout << "⏳ " << server_msg.text << endl;
                    
                    // Si nos piden colocar barcos O recibimos confirmación, colocar siguiente barco
                    if ((string(server_msg.text).find("Coloca") != string::npos || 
                         string(server_msg.text).find("correctamente") != string::npos) && 
                        ships_placed < NUM_SHIPS) {
                        
                        cout << "🚢 Colocando barco " << (ships_placed + 1) << "..." << endl;
                        
                        GameMessage ship_msg(MSG_PLACE_SHIP);
                        ship_msg.x = ship_positions[ships_placed][0];
                        ship_msg.y = ship_positions[ships_placed][1];
                        ship_msg.data1 = DESTROYER; // Tipo de barco
                        ship_msg.data2 = ship_positions[ships_placed][2]; // Orientación
                        
                        sendGameMessage(client_socket, ship_msg);
                        cout << "  ✅ Barco enviado a (" << ship_msg.x << "," << ship_msg.y << ")" << endl;
                        ships_placed++;
                    }
                    break;
                }
                    
                case MSG_YOUR_TURN: {
                    cout << "🎯 ¡Mi turno! Disparando..." << endl;
                    
                    // Generar coordenadas aleatorias para disparar
                    GameMessage shoot_msg(MSG_SHOOT);
                    shoot_msg.x = rand() % BOARD_SIZE;
                    shoot_msg.y = rand() % BOARD_SIZE;
                    
                    sendGameMessage(client_socket, shoot_msg);
                    cout << "  💥 Disparando a (" << shoot_msg.x << "," << shoot_msg.y << ")" << endl;
                    break;
                }
                    
                case MSG_SHOT_RESULT: {
                    cout << "📊 Resultado: " << server_msg.text << " en (" << server_msg.x << "," << server_msg.y << ")" << endl;
                    break;
                }
                    
                case MSG_GAME_OVER: {
                    cout << "🏁 " << server_msg.text << endl;
                    if (server_msg.data1 == 1) {
                        cout << "🎉 ¡He ganado!" << endl;
                    } else {
                        cout << "😔 He perdido..." << endl;
                    }
                    game_active = false;
                    break;
                }
                    
                case MSG_ERROR: {
                    cout << "❌ Error: " << server_msg.text << endl;
                    
                    // Si el error es de colocación de barco, reintentar con nueva posición
                    if (string(server_msg.text).find("inválida") != string::npos && ships_placed < NUM_SHIPS) {
                        cout << "🔄 Reintentando colocar barco " << (ships_placed + 1) << " en nueva posición..." << endl;
                        
                        // Generar nueva posición aleatoria
                        GameMessage ship_msg(MSG_PLACE_SHIP);
                        ship_msg.x = rand() % (BOARD_SIZE - 1);  // Dejar espacio para barco horizontal
                        ship_msg.y = rand() % (BOARD_SIZE - 1);
                        ship_msg.data1 = DESTROYER;
                        ship_msg.data2 = HORIZONTAL;
                        
                        sendGameMessage(client_socket, ship_msg);
                        cout << "  🚢 Nuevo intento: barco enviado a (" << ship_msg.x << "," << ship_msg.y << ")" << endl;
                    }
                    break;
                }
                    
                default: {
                    cout << "❓ Mensaje desconocido: " << server_msg.type << endl;
                    break;
                }
            }
        } else {
            cout << "❌ Error recibiendo mensaje del servidor" << endl;
            game_active = false;
        }
    }
    
    cout << "👋 Desconectando..." << endl;
    close(client_socket);
    return 0;
}