#include "battleship.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

// Constantes del servidor
const int PORT = 8080;
const int MAX_CLIENTS = 2;

// === FUNCIONES DE RED ===

// Enviar mensaje estructurado
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
            cout << "Error enviando mensaje" << endl;
            return false;
        }
    }
    
    return true;
}

// Recibir mensaje estructurado
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
            cout << "Cliente se desconectó" << endl;
            return false;
        } else {
            cout << "Error recibiendo mensaje" << endl;
            return false;
        }
    }
    
    return true;
}

// Crear mensaje de bienvenida
GameMessage createWelcomeMessage(int player_id) {
    GameMessage msg(MSG_WELCOME);
    msg.player_id = player_id;
    string text = "Eres el jugador " + to_string(player_id + 1);
    strncpy(msg.text, text.c_str(), sizeof(msg.text) - 1);
    return msg;
}

// Crear mensaje de espera
GameMessage createWaitMessage(const string& reason) {
    GameMessage msg(MSG_WAIT);
    strncpy(msg.text, reason.c_str(), sizeof(msg.text) - 1);
    return msg;
}

int main() {
    cout << "Servidor Battleship iniciando en puerto " << PORT << "..." << endl;
    
    // Paso 1: Crear socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        cout << "Error: No se pudo crear el socket" << endl;
        return 1;
    }
    cout << "✓ Socket creado exitosamente" << endl;
    
    // Paso 2: Configurar dirección del servidor
    struct sockaddr_in address;
    address.sin_family = AF_INET;        // IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Escuchar en todas las interfaces
    address.sin_port = htons(PORT);      // Puerto convertido a formato de red
    
    cout << "✓ Dirección configurada (0.0.0.0:" << PORT << ")" << endl;
    
    // Paso 3: bind() - Reservar el puerto
    int bind_result = bind(server_socket, (struct sockaddr*)&address, sizeof(address));
    if (bind_result == -1) {
        cout << "Error: No se pudo hacer bind al puerto " << PORT << endl;
        cout << "¿Está el puerto ocupado por otro programa?" << endl;
        close(server_socket);
        return 1;
    }
    cout << "✓ Puerto " << PORT << " reservado exitosamente" << endl;
    
    // Paso 4: listen() - Empezar a escuchar conexiones
    int listen_result = listen(server_socket, MAX_CLIENTS);
    if (listen_result == -1) {
        cout << "Error: No se pudo poner el socket en modo escucha" << endl;
        close(server_socket);
        return 1;
    }
    cout << "✓ Servidor escuchando conexiones..." << endl;
    cout << "Esperando " << MAX_CLIENTS << " jugadores..." << endl;
    
    // Array para guardar los sockets de los clientes
    int client_sockets[MAX_CLIENTS];
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    
    // Paso 5: accept() - Aceptar conexiones de clientes
    for (int i = 0; i < MAX_CLIENTS; i++) {
        cout << "\nEsperando jugador " << (i + 1) << "..." << endl;
        
        client_sockets[i] = accept(server_socket, (struct sockaddr*)&client_address, &client_len);
        if (client_sockets[i] == -1) {
            cout << "Error: No se pudo aceptar la conexión del jugador " << (i + 1) << endl;
            // Cerrar conexiones previas
            for (int j = 0; j < i; j++) {
                close(client_sockets[j]);
            }
            close(server_socket);
            return 1;
        }
        
        // Obtener IP del cliente conectado
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
        cout << "✓ Jugador " << (i + 1) << " conectado desde " << client_ip << endl;
        
        // Enviar mensaje de bienvenida estructurado
        GameMessage welcome = createWelcomeMessage(i);
        if (sendGameMessage(client_sockets[i], welcome)) {
            cout << "✓ Mensaje de bienvenida enviado al jugador " << (i + 1) << endl;
        } else {
            cout << "Error: No se pudo enviar bienvenida al jugador " << (i + 1) << endl;
        }
    }
    
    cout << "\n🎮 ¡Ambos jugadores conectados! Listo para jugar." << endl;
    
    // Recibir nombres usando protocolo estructurado
    string player_names[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        // Esperar mensaje MSG_SET_NAME del cliente
        GameMessage name_msg;
        if (receiveGameMessage(client_sockets[i], name_msg)) {
            if (name_msg.type == MSG_SET_NAME) {
                player_names[i] = string(name_msg.text);
                cout << "✓ Jugador " << (i + 1) << " se llama: " << player_names[i] << endl;
            } else {
                cout << "⚠️ Jugador " << (i + 1) << " envió mensaje inesperado (tipo " << name_msg.type << ")" << endl;
                player_names[i] = "Jugador" + to_string(i + 1);
            }
        } else {
            cout << "✗ Error recibiendo nombre del jugador " << (i + 1) << endl;
            player_names[i] = "Jugador" + to_string(i + 1);
        }
    }
    
    // Enviar información del oponente usando protocolo estructurado
    for (int i = 0; i < MAX_CLIENTS; i++) {
        int opponent = (i + 1) % 2;  // 0->1, 1->0
        GameMessage opponent_msg(MSG_WAIT);
        string opponent_info = "Tu oponente es: " + player_names[opponent];
        strncpy(opponent_msg.text, opponent_info.c_str(), sizeof(opponent_msg.text) - 1);
        sendGameMessage(client_sockets[i], opponent_msg);
    }
    
    cout << "\n🎮 Partida: " << player_names[0] << " vs " << player_names[1] << endl;
    
    // === INICIALIZAR EL JUEGO ===
    Game battleship_game;
    initializeGame(battleship_game);
    
    // Configurar jugadores con nombres reales
    for (int i = 0; i < MAX_CLIENTS; i++) {
        initializePlayer(battleship_game.players[i], i, player_names[i]);
        cout << "✓ Jugador " << (i + 1) << " (" << player_names[i] << ") inicializado" << endl;
    }
    
    cout << "\n📋 Iniciando fase de colocación de barcos..." << endl;
    
    // Dar tiempo para que los clientes procesen los mensajes
    cout << "⏳ Esperando que los clientes estén listos..." << endl;
    usleep(1000000); // Esperar 1 segundo
    
    // === FASE DE COLOCACIÓN DE BARCOS ===
    for (int player_idx = 0; player_idx < MAX_CLIENTS; player_idx++) {
        cout << "\n🚢 Jugador " << (player_idx + 1) << " (" << player_names[player_idx] << ") colocando barcos..." << endl;
        
        // Notificar al jugador que debe colocar barcos
        GameMessage place_ships_msg(MSG_WAIT);
        string msg_text = "Coloca tus " + to_string(NUM_SHIPS) + " barcos";
        strncpy(place_ships_msg.text, msg_text.c_str(), sizeof(place_ships_msg.text) - 1);
        sendGameMessage(client_sockets[player_idx], place_ships_msg);
        
        // Recibir colocación de cada barco
        for (int ship_idx = 0; ship_idx < NUM_SHIPS; ship_idx++) {
            bool ship_placed = false;
            while (!ship_placed) {
                GameMessage ship_msg;
                if (receiveGameMessage(client_sockets[player_idx], ship_msg)) {
                    if (ship_msg.type == MSG_PLACE_SHIP) {
                        // Crear barco con los datos recibidos
                        Ship new_ship(ship_idx, 
                                    static_cast<ShipType>(ship_msg.data1),
                                    ship_msg.x, 
                                    ship_msg.y, 
                                    static_cast<Orientation>(ship_msg.data2));
                        
                        // Intentar colocar barco
                        if (placeShip(battleship_game.players[player_idx].own_board, new_ship)) {
                            battleship_game.players[player_idx].ships.push_back(new_ship);
                            cout << "  ✓ Barco " << (ship_idx + 1) << " colocado en (" << ship_msg.x << "," << ship_msg.y << ")" << endl;
                            
                            // Confirmar al cliente
                            GameMessage confirm(MSG_WAIT);
                            strncpy(confirm.text, "Barco colocado correctamente", sizeof(confirm.text) - 1);
                            sendGameMessage(client_sockets[player_idx], confirm);
                            ship_placed = true;
                        } else {
                            cout << "  ❌ Posición inválida para barco " << (ship_idx + 1) << endl;
                            
                            // Enviar error al cliente
                            GameMessage error(MSG_ERROR);
                            strncpy(error.text, "Posición inválida, intenta otra vez", sizeof(error.text) - 1);
                            sendGameMessage(client_sockets[player_idx], error);
                        }
                    } else {
                        cout << "  ⚠️ Mensaje inesperado del jugador (tipo " << ship_msg.type << ")" << endl;
                    }
                } else {
                    cout << "  ❌ Error recibiendo datos del jugador " << (player_idx + 1) << endl;
                    break;
                }
            }
        }
        
        // Marcar jugador como listo
        markPlayerReady(battleship_game.players[player_idx]);
        cout << "✅ Jugador " << (player_idx + 1) << " terminó de colocar barcos" << endl;
    }
    
    // Iniciar el juego
    startGame(battleship_game);
    cout << "\n⚔️ ¡Iniciando combate!" << endl;
    
    // Notificar a ambos jugadores que el juego comenzó
    for (int i = 0; i < MAX_CLIENTS; i++) {
        GameMessage game_start(MSG_WAIT);
        strncpy(game_start.text, "¡El combate ha comenzado!", sizeof(game_start.text) - 1);
        sendGameMessage(client_sockets[i], game_start);
    }
    
    // === FASE DE COMBATE ===
    bool game_over = false;
    while (!game_over) {
        cout << "\n🎯 Turno de " << player_names[battleship_game.current_turn] << endl;
        
        // Notificar al jugador activo que es su turno
        GameMessage your_turn(MSG_YOUR_TURN);
        sendGameMessage(client_sockets[battleship_game.current_turn], your_turn);
        
        // Notificar al oponente que debe esperar
        int opponent = (battleship_game.current_turn + 1) % 2;
        GameMessage wait_turn(MSG_WAIT);
        string wait_msg = "Turno de " + player_names[battleship_game.current_turn];
        strncpy(wait_turn.text, wait_msg.c_str(), sizeof(wait_turn.text) - 1);
        sendGameMessage(client_sockets[opponent], wait_turn);
        
        // Recibir disparo del jugador activo
        bool valid_shot = false;
        while (!valid_shot) {
            GameMessage shot_msg;
            if (receiveGameMessage(client_sockets[battleship_game.current_turn], shot_msg)) {
                if (shot_msg.type == MSG_SHOOT) {
                    cout << "  💥 " << player_names[battleship_game.current_turn] << " dispara a (" << shot_msg.x << "," << shot_msg.y << ")" << endl;
                    
                    // Procesar el disparo
                    int opponent = (battleship_game.current_turn + 1) % 2;
                    
                    if (makeShot(battleship_game.players[battleship_game.current_turn], 
                                battleship_game.players[opponent], 
                                shot_msg.x, shot_msg.y)) {
                        
                        // Determinar resultado del disparo
                        bool hit = (battleship_game.players[opponent].own_board.grid[shot_msg.x][shot_msg.y] == HIT);
                        bool sunk = false;
                        
                        // Verificar si algún barco se hundió
                        if (hit) {
                            for (const auto& ship : battleship_game.players[opponent].ships) {
                                if (ship.sunk()) {
                                    sunk = true;
                                    break;
                                }
                            }
                        }
                        
                        // Verificar si el juego terminó
                        game_over = (battleship_game.players[opponent].ships_remaining() == 0);
                        
                        // Crear mensaje de resultado
                        GameMessage result(MSG_SHOT_RESULT);
                        result.data1 = hit ? 1 : 0;
                        result.data2 = sunk ? 1 : 0;
                        result.x = shot_msg.x;
                        result.y = shot_msg.y;
                        
                        if (game_over) {
                            strncpy(result.text, "¡Todos los barcos hundidos! Juego terminado", sizeof(result.text) - 1);
                        } else if (sunk) {
                            strncpy(result.text, hit ? "¡Impacto! ¡Barco hundido!" : "Agua...", sizeof(result.text) - 1);
                        } else {
                            strncpy(result.text, hit ? "¡Impacto!" : "Agua...", sizeof(result.text) - 1);
                        }
                        
                        // Enviar resultado a ambos jugadores
                        sendGameMessage(client_sockets[battleship_game.current_turn], result);
                        sendGameMessage(client_sockets[opponent], result);
                        
                        valid_shot = true;
                        
                        // Cambiar turno solo si falló (MISS) y el juego no terminó
                        if (!game_over && !hit) {
                            battleship_game.players[battleship_game.current_turn].is_turn = false;
                            battleship_game.current_turn = (battleship_game.current_turn + 1) % 2;
                            battleship_game.players[battleship_game.current_turn].is_turn = true;
                            cout << "  🔄 Turno cambiado. Ahora juega: " << player_names[battleship_game.current_turn] << endl;
                        } else if (!game_over && hit) {
                            cout << "  🎯 " << player_names[battleship_game.current_turn] << " mantiene el turno por impacto" << endl;
                        }
                        
                    } else {
                        cout << "  ❌ Disparo inválido" << endl;
                        GameMessage error(MSG_ERROR);
                        strncpy(error.text, "Disparo inválido, intenta otra posición", sizeof(error.text) - 1);
                        sendGameMessage(client_sockets[battleship_game.current_turn], error);
                    }
                } else {
                    cout << "  ⚠️ Mensaje inesperado del jugador (esperaba disparo)" << endl;
                }
            } else {
                cout << "  ❌ Error recibiendo disparo del jugador" << endl;
                game_over = true; // Terminar juego si hay problemas de comunicación
                break;
            }
        }
    }
    
    // === FIN DEL JUEGO ===
    if (game_over) {
        int winner = battleship_game.current_turn;
        int loser = (winner + 1) % 2;
        
        cout << "\n🏆 ¡" << player_names[winner] << " ha ganado la partida!" << endl;
        cout << "📊 Barcos restantes:" << endl;
        cout << "   " << player_names[0] << ": " << battleship_game.players[0].ships_remaining() << endl;
        cout << "   " << player_names[1] << ": " << battleship_game.players[1].ships_remaining() << endl;
        
        // Enviar mensaje de victoria al ganador
        GameMessage win_msg(MSG_GAME_OVER);
        string win_text = "¡Felicidades! ¡Has ganado la partida!";
        strncpy(win_msg.text, win_text.c_str(), sizeof(win_msg.text) - 1);
        win_msg.data1 = 1; // 1 = ganador
        sendGameMessage(client_sockets[winner], win_msg);
        
        // Enviar mensaje de derrota al perdedor
        GameMessage lose_msg(MSG_GAME_OVER);
        string lose_text = "¡Juego terminado! " + player_names[winner] + " ha ganado";
        strncpy(lose_msg.text, lose_text.c_str(), sizeof(lose_msg.text) - 1);
        lose_msg.data1 = 0; // 0 = perdedor
        sendGameMessage(client_sockets[loser], lose_msg);
        
        cout << "\n✅ Partida completada exitosamente" << endl;
    } else {
        cout << "\n⚠️ Juego terminado por error de comunicación" << endl;
    }
    
    // Cerrar todas las conexiones
    for (int i = 0; i < MAX_CLIENTS; i++) {
        close(client_sockets[i]);
    }
    close(server_socket);
    return 0;
}
