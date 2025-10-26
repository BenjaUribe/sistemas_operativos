#include "battleship.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

// Constantes del servidor
const int PORT = 8080;

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

// Crear mensaje de tamaño del tablero
GameMessage createBoardSizeMessage(int board_size) {
    GameMessage msg(MSG_BOARD_SIZE);
    msg.data1 = board_size;
    string text = "Tablero " + to_string(board_size) + "x" + to_string(board_size);
    strncpy(msg.text, text.c_str(), sizeof(msg.text) - 1);
    return msg;
}

// Crear mensaje de espera
GameMessage createWaitMessage(const string& reason) {
    GameMessage msg(MSG_WAIT);
    strncpy(msg.text, reason.c_str(), sizeof(msg.text) - 1);
    return msg;
}

// Enviar estado del tablero al cliente (para modo 2vs2)
void sendBoardState(int client_socket, const Board& board, const string& description) {
    GameMessage board_msg(MSG_BOARD_STATE);
    strncpy(board_msg.text, description.c_str(), sizeof(board_msg.text) - 1);
    
    // Enviar el mensaje de estado del tablero
    sendGameMessage(client_socket, board_msg);
    
    // Enviar el estado completo del tablero en múltiples mensajes
    for (int i = 0; i < board.size; i++) {
        for (int j = 0; j < board.size; j++) {
            GameMessage cell_msg(MSG_BOARD_STATE);
            cell_msg.x = i;
            cell_msg.y = j;
            cell_msg.data1 = board.grid[i][j]; // Estado de la celda
            sendGameMessage(client_socket, cell_msg);
        }
    }
}

int main() {
    cout << "=============================================" << endl;
    cout << "      SERVIDOR BATTLESHIP                 " << endl;
    cout << "=============================================" << endl;
    
    // Selección de modo de juego
    cout << "\n Selecciona el modo de juego:" << endl;
    cout << "  1. Modo 1vs1 (2 jugadores)" << endl;
    cout << "  2. Modo 2vs2 (4 jugadores)" << endl;
    cout << "Opción: ";
    
    int mode_choice;
    cin >> mode_choice;
    
    GameMode selected_mode;
    int target_players;
    
    switch (mode_choice) {
        case 1:
            selected_mode = MODE_1VS1;
            target_players = 2;
            cout << " Modo 1vs1 seleccionado (2 jugadores)" << endl;
            break;
        case 2:
            selected_mode = MODE_2VS2;
            target_players = 4;
            cout << " Modo 2vs2 seleccionado (4 jugadores)" << endl;
            break;
        default:
            cout << "  Opción inválida. Usando modo 1vs1 por defecto." << endl;
            selected_mode = MODE_1VS1;
            target_players = 2;
            break;
    }
    
    cout << "\n Servidor iniciando en puerto " << PORT << "..." << endl;
    
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
    int listen_result = listen(server_socket, target_players);
    if (listen_result == -1) {
        cout << "Error: No se pudo poner el socket en modo escucha" << endl;
        close(server_socket);
        return 1;
    }
    cout << "✓ Servidor escuchando conexiones..." << endl;
    cout << " Esperando " << target_players << " jugadores..." << endl;
    
    // Array para guardar los sockets de los clientes
    int client_sockets[MAX_PLAYERS];
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);
    
    // Paso 5: accept() - Aceptar conexiones de clientes
    for (int i = 0; i < target_players; i++) {
        cout << "\n Esperando jugador " << (i + 1) << "/" << target_players << "..." << endl;
        
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
        cout << " Jugador " << (i + 1) << " conectado desde " << client_ip << endl;
        
        // Enviar mensaje de bienvenida estructurado
        GameMessage welcome = createWelcomeMessage(i);
        if (sendGameMessage(client_sockets[i], welcome)) {
            cout << "✓ Mensaje de bienvenida enviado al jugador " << (i + 1) << endl;
        } else {
            cout << "Error: No se pudo enviar bienvenida al jugador " << (i + 1) << endl;
        }
        
        // Enviar tamaño del tablero según el modo
        int board_size = getBoardSizeForMode(selected_mode);
        GameMessage board_size_msg = createBoardSizeMessage(board_size);
        if (sendGameMessage(client_sockets[i], board_size_msg)) {
            cout << "✓ Tamaño de tablero (" << board_size << "x" << board_size << ") enviado al jugador " << (i + 1) << endl;
        } else {
            cout << "Error: No se pudo enviar tamaño de tablero al jugador " << (i + 1) << endl;
        }
    }
    
    string players_text = (target_players == 2) ? "jugadores" : "jugadores";
    cout << "\n ¡Todos los " << target_players << " " << players_text << " conectados! Listo para jugar." << endl;
    
    // Recibir nombres usando protocolo estructurado
    string player_names[MAX_PLAYERS];
    for (int i = 0; i < target_players; i++) {
        // Esperar mensaje MSG_SET_NAME del cliente
        GameMessage name_msg;
        if (receiveGameMessage(client_sockets[i], name_msg)) {
            if (name_msg.type == MSG_SET_NAME) {
                player_names[i] = string(name_msg.text);
                cout << " Jugador " << (i + 1) << " se llama: " << player_names[i] << endl;
            } else {
                cout << " Jugador " << (i + 1) << " envió mensaje inesperado (tipo " << name_msg.type << ")" << endl;
                player_names[i] = "Jugador" + to_string(i + 1);
            }
        } else {
            cout << " Error recibiendo nombre del jugador " << (i + 1) << endl;
            player_names[i] = "Jugador" + to_string(i + 1);
        }
    }
    
    // Enviar información de otros jugadores
    for (int i = 0; i < target_players; i++) {
        GameMessage info_msg(MSG_WAIT);
        string info_text;
        
        if (selected_mode == MODE_1VS1) {
            int opponent = (i + 1) % 2;  // 0->1, 1->0
            info_text = "Tu oponente es: " + player_names[opponent];
        } else {
            // Modo 2vs2: mostrar información de equipo
            if (i < 2) {
                info_text = "Equipo 1: " + player_names[0] + " & " + player_names[1];
            } else {
                info_text = "Equipo 2: " + player_names[2] + " & " + player_names[3];
            }
        }
        
        strncpy(info_msg.text, info_text.c_str(), sizeof(info_msg.text) - 1);
        sendGameMessage(client_sockets[i], info_msg);
    }
    
    // Mostrar información de la partida
    if (selected_mode == MODE_1VS1) {
        cout << "\n  Partida 1vs1: " << player_names[0] << " vs " << player_names[1] << endl;
    } else {
        cout << "\n  Partida 2vs2:" << endl;
        cout << "     Equipo 1: " << player_names[0] << " & " << player_names[1] << endl;
        cout << "     Equipo 2: " << player_names[2] << " & " << player_names[3] << endl;
    }
    
    // === INICIALIZAR EL JUEGO ===
    Game battleship_game;
    initializeGame(battleship_game, selected_mode);
    
    // Configurar modo del juego
    battleship_game.set_mode(selected_mode, target_players);
    
    // Configurar jugadores con nombres reales
    for (int i = 0; i < target_players; i++) {
        initializePlayer(battleship_game.players[i], i, player_names[i]);
        cout << "✓ Jugador " << (i + 1) << " (" << player_names[i] << ") inicializado" << endl;
    }
    
    cout << "\n📋 Iniciando fase de colocación de barcos..." << endl;
    
    // Dar tiempo para que los clientes procesen los mensajes
    cout << "⏳ Esperando que los clientes estén listos..." << endl;
    usleep(1000000); // Esperar 1 segundo
    
    // === FASE DE COLOCACIÓN DE BARCOS ===
    
    if (selected_mode == MODE_1VS1) {
        // Modo 1vs1: Cada jugador coloca en su propio tablero
        for (int player_idx = 0; player_idx < target_players; player_idx++) {
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
                            
                            // Intentar colocar barco en tablero individual
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
        
    } else {
        // Modo 2vs2: Colocación alternada en tableros compartidos por equipo
        int total_ships = NUM_SHIPS * target_players; // 3 barcos * 4 jugadores = 12 barcos total
        
        cout << "\n🔄 Modo 2vs2: Colocación alternada entre equipos" << endl;
        cout << "📋 Orden: Equipo A (J1) → Equipo B (J3) → Equipo A (J2) → Equipo B (J4) → ..." << endl;
        
        for (int ship_number = 0; ship_number < total_ships; ship_number++) {
            int current_player = getNextPlacingPlayer(battleship_game, ship_number);
            int player_team = getPlayerTeam(current_player);
            
            cout << "\n🚢 Turno " << (ship_number + 1) << "/" << total_ships << 
                    ": " << player_names[current_player] << 
                    " (Equipo " << (player_team + 1) << ") colocando barco..." << endl;
            
            // Enviar estado actual del tablero del equipo al jugador
            string board_desc = "Tablero actual del Equipo " + to_string(player_team + 1);
            sendBoardState(client_sockets[current_player], battleship_game.teams[player_team].shared_board, board_desc);
            
            // Notificar al jugador actual
            GameMessage place_msg(MSG_WAIT);
            int ships_in_team = battleship_game.teams[player_team].ships_placed_count + 1;
            string msg_text = "Tu turno: coloca barco " + to_string(ships_in_team) + "/6 para tu equipo";
            strncpy(place_msg.text, msg_text.c_str(), sizeof(place_msg.text) - 1);
            sendGameMessage(client_sockets[current_player], place_msg);
            
            // Notificar a otros jugadores que esperen
            for (int i = 0; i < target_players; i++) {
                if (i != current_player) {
                    GameMessage wait_msg(MSG_WAIT);
                    string wait_text = player_names[current_player] + " está colocando barco para su equipo...";
                    strncpy(wait_msg.text, wait_text.c_str(), sizeof(wait_msg.text) - 1);
                    sendGameMessage(client_sockets[i], wait_msg);
                }
            }
            
            // Recibir colocación del barco
            bool ship_placed = false;
            while (!ship_placed) {
                GameMessage ship_msg;
                if (receiveGameMessage(client_sockets[current_player], ship_msg)) {
                    if (ship_msg.type == MSG_PLACE_SHIP) {
                        // Crear barco con los datos recibidos
                        Ship new_ship(ship_number, 
                                    static_cast<ShipType>(ship_msg.data1),
                                    ship_msg.x, 
                                    ship_msg.y, 
                                    static_cast<Orientation>(ship_msg.data2));
                        
                        // Intentar colocar barco en tablero compartido del equipo
                        if (placeShip(battleship_game.teams[player_team].shared_board, new_ship)) {
                            battleship_game.teams[player_team].team_ships.push_back(new_ship);
                            battleship_game.teams[player_team].ships_placed_count++;
                            
                            cout << "  ✓ Barco colocado en tablero del Equipo " << (player_team + 1) << 
                                    " en (" << ship_msg.x << "," << ship_msg.y << ")" << endl;
                            
                            // Confirmar al cliente
                            GameMessage confirm(MSG_WAIT);
                            strncpy(confirm.text, "Barco colocado en tablero del equipo", sizeof(confirm.text) - 1);
                            sendGameMessage(client_sockets[current_player], confirm);
                            ship_placed = true;
                        } else {
                            cout << "  ❌ Posición inválida para barco en tablero del equipo" << endl;
                            
                            // Enviar error al cliente
                            GameMessage error(MSG_ERROR);
                            strncpy(error.text, "Posición inválida, intenta otra vez", sizeof(error.text) - 1);
                            sendGameMessage(client_sockets[current_player], error);
                        }
                    } else {
                        cout << "  ⚠️ Mensaje inesperado del jugador (tipo " << ship_msg.type << ")" << endl;
                    }
                } else {
                    cout << "  ❌ Error recibiendo datos del jugador " << (current_player + 1) << endl;
                    break;
                }
            }
        }
        
        cout << "\n✅ Todos los barcos colocados en tableros de equipos!" << endl;
        cout << "🔵 Equipo 1: " << battleship_game.teams[0].ships_placed_count << " barcos" << endl;
        cout << "🔴 Equipo 2: " << battleship_game.teams[1].ships_placed_count << " barcos" << endl;
    }
    
    // Iniciar el juego
    startGame(battleship_game);
    cout << "\n⚔️ ¡Iniciando combate!" << endl;
    
    // Notificar a todos los jugadores que el juego comenzó
    for (int i = 0; i < target_players; i++) {
        GameMessage game_start(MSG_WAIT);
        strncpy(game_start.text, "¡El combate ha comenzado!", sizeof(game_start.text) - 1);
        sendGameMessage(client_sockets[i], game_start);
    }
    
    // En modo 2vs2, enviar el estado del tablero del equipo a cada jugador
    if (selected_mode == MODE_2VS2) {
        for (int i = 0; i < target_players; i++) {
            int player_team = getPlayerTeam(i);
            string board_desc = "Estado inicial de tu equipo";
            sendBoardState(client_sockets[i], battleship_game.teams[player_team].shared_board, board_desc);
        }
    }
    
    // === FASE DE COMBATE ===
    bool game_over = false;
    while (!game_over) {
        int current_player = battleship_game.current_turn;
        cout << "\n🎯 Turno de " << player_names[current_player] << endl;
        
        // Notificar al jugador activo que es su turno
        GameMessage your_turn(MSG_YOUR_TURN);
        sendGameMessage(client_sockets[current_player], your_turn);
        
        // Notificar a otros jugadores que deben esperar
        for (int i = 0; i < target_players; i++) {
            if (i != current_player) {
                GameMessage wait_turn(MSG_WAIT);
                string wait_msg = "Turno de " + player_names[current_player];
                strncpy(wait_turn.text, wait_msg.c_str(), sizeof(wait_turn.text) - 1);
                sendGameMessage(client_sockets[i], wait_turn);
            }
        }
        
        // Recibir disparo del jugador activo
        bool valid_shot = false;
        while (!valid_shot) {
            GameMessage shot_msg;
            if (receiveGameMessage(client_sockets[current_player], shot_msg)) {
                if (shot_msg.type == MSG_SHOOT) {
                    cout << "  💥 " << player_names[current_player] << " dispara a (" << shot_msg.x << "," << shot_msg.y << ")" << endl;
                    
                    bool hit = false;
                    bool sunk = false;
                    
                    if (selected_mode == MODE_1VS1) {
                        // Modo 1vs1: Atacar tablero individual del oponente
                        int opponent = (current_player + 1) % 2;
                        
                        if (makeShot(battleship_game.players[current_player], 
                                    battleship_game.players[opponent], 
                                    shot_msg.x, shot_msg.y)) {
                            
                            hit = (battleship_game.players[opponent].own_board.grid[shot_msg.x][shot_msg.y] == HIT);
                            
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
                            valid_shot = true;
                        }
                        
                    } else {
                        // Modo 2vs2: Atacar tablero compartido del equipo enemigo
                        int attacking_team = getPlayerTeam(current_player);
                        int defending_team = (attacking_team + 1) % 2;
                        
                        // Verificar que la posición sea válida y no haya sido atacada antes
                        if (isValidPosition(shot_msg.x, shot_msg.y, battleship_game.teams[defending_team].shared_board.size)) {
                            Board& target_board = battleship_game.teams[defending_team].shared_board;
                            
                            // Verificar que no se haya disparado antes a esta posición
                            if (target_board.grid[shot_msg.x][shot_msg.y] != HIT && 
                                target_board.grid[shot_msg.x][shot_msg.y] != MISS) {
                                
                                // Verificar si hay un barco en esa posición
                                if (target_board.grid[shot_msg.x][shot_msg.y] == SHIP) {
                                    // ¡Impacto!
                                    target_board.grid[shot_msg.x][shot_msg.y] = HIT;
                                    hit = true;
                                    
                                    // Encontrar qué barco fue impactado y actualizar sus hits
                                    for (auto& ship : battleship_game.teams[defending_team].team_ships) {
                                        if (checkShipHit(ship, shot_msg.x, shot_msg.y)) {
                                            cout << "  🎯 ¡Impacto en equipo " << (defending_team + 1) << "!";
                                            if (ship.sunk()) {
                                                cout << " ¡Barco hundido!";
                                                sunk = true;
                                            }
                                            cout << endl;
                                            break;
                                        }
                                    }
                                } else {
                                    // Agua
                                    target_board.grid[shot_msg.x][shot_msg.y] = MISS;
                                    cout << "  🌊 Agua..." << endl;
                                }
                                
                                // Verificar si el equipo defensor perdió todos los barcos
                                game_over = (battleship_game.teams[defending_team].ships_remaining() == 0);
                                valid_shot = true;
                            }
                        }
                    }
                    
                    if (valid_shot) {
                        // Crear mensaje de resultado
                        GameMessage result(MSG_SHOT_RESULT);
                        result.player_id = current_player;  // Incluir quién disparó
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
                        
                        // Enviar resultado a todos los jugadores
                        for (int i = 0; i < target_players; i++) {
                            sendGameMessage(client_sockets[i], result);
                        }
                        
                        // Cambiar turno solo si falló (MISS) y el juego no terminó
                        if (!game_over && !hit) {
                            battleship_game.players[current_player].is_turn = false;

                            if (selected_mode == MODE_1VS1) {
                                // Alternancia simple entre 2 jugadores
                                battleship_game.current_turn = (battleship_game.current_turn + 1) % 2;
                            } else {
                                // Modo 2vs2: Usar un orden de turnos por equipos para no alternar siempre
                                // entre los mismos dos jugadores. Definimos un orden fijo que
                                // alterna equipos y rota jugadores dentro de cada equipo:
                                // Orden: 0 (A1), 2 (B1), 1 (A2), 3 (B2)
                                int turnOrder[] = {0, 2, 1, 3};
                                int orderSize = 4;

                                // Encontrar la posición actual en el arreglo de turnos
                                int pos = 0;
                                for (int p = 0; p < orderSize; p++) {
                                    if (turnOrder[p] == current_player) {
                                        pos = p;
                                        break;
                                    }
                                }

                                int nextPos = (pos + 1) % orderSize;
                                battleship_game.current_turn = turnOrder[nextPos];
                            }
                            
                            battleship_game.players[battleship_game.current_turn].is_turn = true;
                            cout << "  🔄 Turno cambiado. Ahora juega: " << player_names[battleship_game.current_turn] << endl;
                        } else if (!game_over && hit) {
                            cout << "  🎯 " << player_names[current_player] << " mantiene el turno por impacto" << endl;
                        }
                        
                    } else {
                        cout << "  ❌ Disparo inválido" << endl;
                        GameMessage error(MSG_ERROR);
                        strncpy(error.text, "Disparo inválido, intenta otra posición", sizeof(error.text) - 1);
                        sendGameMessage(client_sockets[current_player], error);
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
        if (selected_mode == MODE_1VS1) {
            // Modo 1vs1: El ganador es quien hizo el último disparo
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
            
        } else {
            // Modo 2vs2: El equipo ganador es el del jugador que hizo el último disparo
            int winning_player = battleship_game.current_turn;
            int winning_team = getPlayerTeam(winning_player);
            
            cout << "\n🏆 ¡Equipo " << (winning_team + 1) << " ha ganado la partida!" << endl;
            cout << "🔵 Equipo ganador: ";
            cout << "📊 Barcos restantes por equipo:" << endl;
            cout << "   🔵 Equipo 1: " << battleship_game.teams[0].ships_remaining() << " barcos" << endl;
            cout << "   🔴 Equipo 2: " << battleship_game.teams[1].ships_remaining() << " barcos" << endl;
            
            // Enviar mensajes a todos los jugadores
            for (int i = 0; i < target_players; i++) {
                GameMessage msg(MSG_GAME_OVER);
                int player_team = getPlayerTeam(i);
                
                if (player_team == winning_team) {
                    // Jugador del equipo ganador
                    string win_text = "¡Felicidades! ¡Tu equipo ha ganado!";
                    strncpy(msg.text, win_text.c_str(), sizeof(msg.text) - 1);
                    msg.data1 = 1; // 1 = ganador
                    cout << "     " << player_names[i];
                    if (i == 0 || i == 2) cout << " & ";
                } else {
                    // Jugador del equipo perdedor
                    string lose_text = "¡Juego terminado! Equipo " + to_string(winning_team + 1) + " ha ganado";
                    strncpy(msg.text, lose_text.c_str(), sizeof(msg.text) - 1);
                    msg.data1 = 0; // 0 = perdedor
                }
                
                sendGameMessage(client_sockets[i], msg);
            }
            cout << endl;
        }
        
        cout << "\n✅ Partida completada exitosamente" << endl;
    } else {
        cout << "\n⚠️ Juego terminado por error de comunicación" << endl;
    }
    
    // Cerrar todas las conexiones
    for (int i = 0; i < target_players; i++) {
        close(client_sockets[i]);
    }
    close(server_socket);
    return 0;
}
