#include "battleship.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

// Constantes del cliente
const string DEFAULT_SERVER_IP = "127.0.0.1";
const int DEFAULT_SERVER_PORT = 8080;

// === FUNCIONES AUXILIARES ===
// Determinar si dos jugadores están en el mismo equipo (solo para modo 2vs2)
bool sameTeam(int player1, int player2, int board_size) {
    // Determinar modo basado en el tamaño del tablero
    bool is_2vs2_mode = (board_size == BOARD_SIZE_2VS2);
    
    if (is_2vs2_mode) {
        // Modo 2vs2: Equipo 1: jugadores 0 y 1, Equipo 2: jugadores 2 y 3
        return (player1 / 2) == (player2 / 2);
    } else {
        // Modo 1vs1: jugadores 0 y 1 son siempre oponentes
        return false;
    }
}


// === FUNCIONES DE RED ===
bool sendGameMessage(int socket, const GameMessage& msg) {
    int total_sent = 0;
    int message_size = sizeof(GameMessage);
    const char* buffer = (const char*)&msg;
    
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

// Recibir mensaje estructurado
bool receiveGameMessage(int socket, GameMessage& msg) {
    int total_received = 0;
    int message_size = sizeof(GameMessage);
    char* buffer = (char*)&msg;
    
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

// Funciones para interfaz interactiva
void printBoard(const Board& board, bool show_ships = true) {
    cout << "\n   ";
    for (int j = 0; j < board.size; j++) {
        cout << " " << j << " ";
    }
    cout << endl;
    
    for (int i = 0; i < board.size; i++) {
        cout << " " << i << " ";
        for (int j = 0; j < board.size; j++) {
            char cell = ' ';
            if (board.grid[i][j] == SHIP && show_ships) {
                cell = 'S';  // Barco
            } else if (board.grid[i][j] == HIT) {
                cell = 'X';  // Impacto
            } else if (board.grid[i][j] == MISS) {
                cell = 'O';  // Fallo
            } else {
                cell = '~';  // Agua
            }
            cout << "[" << cell << "]";
        }
        cout << endl;
    }
    cout << endl;
}

// Mostrar ambos tableros en formato vertical
void printBothBoards(const Board& myBoard, const Board& enemyBoard) {
    cout << "\n======================================" << endl;
    cout << "===        TUS BARCOS             ===" << endl;
    cout << "======================================" << endl;
    cout << "S=barco, X=barco dañado, O=disparo enemigo fallido, ~=agua" << endl;
    
    // Encabezado para mi tablero
    cout << "   ";
    for (int j = 0; j < myBoard.size; j++) {
        cout << " " << j << " ";
    }
    cout << endl;
    
    // Mi tablero
    for (int i = 0; i < myBoard.size; i++) {
        cout << " " << i << " ";
        for (int j = 0; j < myBoard.size; j++) {
            char cell = '~';
            if (myBoard.grid[i][j] == SHIP) {
                cell = 'S';  // Mis barcos
            } else if (myBoard.grid[i][j] == HIT) {
                cell = 'X';  // Mis barcos dañados
            } else if (myBoard.grid[i][j] == MISS) {
                cell = 'O';  // Disparos enemigos fallidos
            }
            cout << "[" << cell << "]";
        }
        cout << endl;
    }
    
    cout << "\n======================================" << endl;
    cout << "===      TABLERO ENEMIGO          ===" << endl;
    cout << "======================================" << endl;
    cout << "X=tus impactos, O=tus fallos, ~=desconocido" << endl;
    
    // Encabezado para tablero enemigo
    cout << "   ";
    for (int j = 0; j < enemyBoard.size; j++) {
        cout << " " << j << " ";
    }
    cout << endl;
    
    // Tablero enemigo
    for (int i = 0; i < enemyBoard.size; i++) {
        cout << " " << i << " ";
        for (int j = 0; j < enemyBoard.size; j++) {
            char cell = '~';
            if (enemyBoard.grid[i][j] == HIT) {
                cell = 'X';  // Impactos míos
            } else if (enemyBoard.grid[i][j] == MISS) {
                cell = 'O';  // Fallos míos
            }
            cout << "[" << cell << "]";
        }
        cout << endl;
    }
    cout << "======================================\n" << endl;
}

bool getCoordinates(int& x, int& y, const string& prompt, int board_size) {
    cout << prompt;
    string input;
    getline(cin, input);
    
    // Formato esperado: "x,y" o "x y"
    size_t comma = input.find(',');
    size_t space = input.find(' ');
    
    if (comma != string::npos) {
        try {
            x = stoi(input.substr(0, comma));
            y = stoi(input.substr(comma + 1));
            return (x >= 0 && x < board_size && y >= 0 && y < board_size);
        } catch (...) {
            return false;
        }
    } else if (space != string::npos) {
        try {
            x = stoi(input.substr(0, space));
            y = stoi(input.substr(space + 1));
            return (x >= 0 && x < board_size && y >= 0 && y < board_size);
        } catch (...) {
            return false;
        }
    }
    return false;
}

// Coloca los barcos en el tablero pasado por referencia
bool handleShipPlacement(int socket, Board& myBoard) {
    cout << "\n🚢 FASE DE COLOCACIÓN DE BARCOS" << endl;
    cout << "Tienes que colocar " << NUM_SHIPS << " barcos:" << endl;
    cout << "  1. Destructor (tamaño 2)" << endl;
    cout << "  2. Acorazado (tamaño 3)" << endl; 
    cout << "  3. Portaaviones (tamaño 4)" << endl;
    cout << "Formato: fila,columna,orientación (ejemplo: 2,3,H o 2,3,V)" << endl;
    cout << "Orientación: H=horizontal, V=vertical" << endl;
    cout << "Rango válido: 0-" << (myBoard.size-1) << endl;
    
    // myBoard debe venir inicializado por el llamador
    
    // Definir los tipos de barcos en orden
    ShipType shipTypes[NUM_SHIPS] = {DESTROYER, BATTLESHIP, AIRCRAFT_CARRIER};
    string shipNames[NUM_SHIPS] = {"Destructor", "Acorazado", "Portaaviones"};
    
    for (int ship = 0; ship < NUM_SHIPS; ship++) {
        bool placed = false;
        ShipType currentShipType = shipTypes[ship];
        int shipSize = static_cast<int>(currentShipType);
        
        while (!placed) {
            cout << "\n🔹 " << shipNames[ship] << " (tamaño " << shipSize << ") - " << (ship+1) << "/" << NUM_SHIPS << endl;
            cout << "Tu tablero actual:" << endl;
            printBoard(myBoard, true);
            
            // Pedir coordenadas y orientación
            cout << "Ingresa: fila,columna,orientación (ej: 1,2,H o 1,2,V): ";
            string input;
            getline(cin, input);
            
            // Parsear entrada
            size_t comma1 = input.find(',');
            size_t comma2 = input.find(',', comma1 + 1);
            
            if (comma1 != string::npos && comma2 != string::npos) {
                try {
                    int x = stoi(input.substr(0, comma1));
                    int y = stoi(input.substr(comma1 + 1, comma2 - comma1 - 1));
                    char orientChar = toupper(input.substr(comma2 + 1)[0]);
                    
                    if (x < 0 || x >= myBoard.size || y < 0 || y >= myBoard.size) {
                        cout << "❌ Coordenadas fuera de rango." << endl;
                        continue;
                    }
                    
                    if (orientChar != 'H' && orientChar != 'V') {
                        cout << "❌ Orientación inválida. Use H o V." << endl;
                        continue;
                    }
                    
                    Orientation orientation = (orientChar == 'H') ? HORIZONTAL : VERTICAL;
                    
                    // Crear mensaje de colocación
                    GameMessage place_msg(MSG_PLACE_SHIP);
                    place_msg.x = x;
                    place_msg.y = y;
                    place_msg.data1 = static_cast<int>(currentShipType);  // Tipo/tamaño del barco
                    place_msg.data2 = static_cast<int>(orientation);      // Orientación
                    
                    if (sendGameMessage(socket, place_msg)) {
                    cout << "📤 Enviando barco a posición [" << x << "," << y << "]..." << endl;
                    
                    // Esperar respuesta del servidor
                    GameMessage response;
                    if (receiveGameMessage(socket, response)) {
                        // El servidor confirma colocación mediante MSG_WAIT
                        if (response.type == MSG_WAIT) {
                            string msg_text(response.text);
                            // Aceptar cualquier mensaje MSG_WAIT que no sea error
                            if (msg_text.find("Error") == string::npos && 
                                msg_text.find("error") == string::npos) {
                                cout << "✅ " << response.text << " en [" << x << "," << y << "]" << endl;
                                // Actualizar el tablero local - colocar todas las celdas del barco
                                for (int i = 0; i < shipSize; i++) {
                                    if (orientation == HORIZONTAL) {
                                        myBoard.grid[x][y + i] = SHIP;
                                    } else {
                                        myBoard.grid[x + i][y] = SHIP;
                                    }
                                }
                                placed = true;
                                
                                // Mostrar tablero actualizado
                                cout << "\n📋 Tablero actualizado:" << endl;
                                printBoard(myBoard, true);
                            } else {
                                cout << "❌ " << response.text << endl;
                            }
                        } else if (response.type == MSG_ERROR) {
                            cout << "❌ " << response.text << endl;
                        } else {
                            cout << "⚠️ Respuesta inesperada del servidor" << endl;
                        }
                    } else {
                        cout << "❌ Error recibiendo respuesta del servidor" << endl;
                        break;
                    }
                    } else {
                        cout << "❌ Error enviando posición del barco" << endl;
                        break;
                    }
                } catch (...) {
                    cout << "❌ Error en formato de entrada. Use: fila,columna,orientación (ej: 1,2,H)" << endl;
                }
            } else {
                cout << "❌ Formato inválido. Use: fila,columna,orientación (ej: 1,2,H)" << endl;
            }
        }
        
        if (!placed) {
            cout << "❌ Error colocando barco. Terminando..." << endl;
            return false;
        }
    }
    
    cout << "\n🎉 ¡Todos los barcos colocados exitosamente!" << endl;
    cout << "\n📋 Tu tablero final:" << endl;
    printBoard(myBoard, true);
    
    // Enviar mensaje de listo automáticamente
    cout << "\n📤 Enviando señal de listo al servidor..." << endl;
    GameMessage ready_msg(MSG_READY);
    if (sendGameMessage(socket, ready_msg)) {
        cout << "✅ Señal de listo enviada. Esperando al oponente..." << endl;
        return true;
    } else {
        cout << "❌ Error enviando señal de listo" << endl;
        return false;
    }
}

// Realiza un disparo y marca awaiting_result=true si se envió con éxito
bool handleShooting(int socket, Board& myBoard, Board& enemyBoard, bool &awaiting_result) {
    cout << "\n🎯 TU TURNO - Elige dónde disparar" << endl;
    printBothBoards(myBoard, enemyBoard);
    
    int x, y;
    bool validShot = false;
    
    while (!validShot) {
        if (getCoordinates(x, y, "Dispara a (fila,columna): ", enemyBoard.size)) {
            if (enemyBoard.grid[x][y] == WATER) {  // Solo disparar a posiciones desconocidas
                GameMessage shoot_msg(MSG_SHOOT);
                shoot_msg.x = x;
                shoot_msg.y = y;
                
                if (sendGameMessage(socket, shoot_msg)) {
                    cout << "💥 Disparo enviado a [" << x << "," << y << "]" << endl;
                    validShot = true;
                    // Esperamos el resultado del servidor
                    awaiting_result = true;
                    return true;
                } else {
                    cout << "❌ Error enviando disparo" << endl;
                    return false;
                }
            } else {
                cout << "❌ Ya disparaste a esa posición. Elige otra." << endl;
            }
        } else {
            cout << "❌ Coordenadas inválidas. Usa formato: fila,columna (ejemplo: 2,3)" << endl;
        }
    }
    return false;
}

void updateEnemyBoard(Board& enemyBoard, int x, int y, bool hit) {
    if (hit) {
        enemyBoard.grid[x][y] = HIT;
    } else {
        enemyBoard.grid[x][y] = MISS;
    }
}

// Manejar colocación de un solo barco (modo 2vs2)
bool handleSingleShipPlacement(int socket) {
    cout << "\n🚢 Coloca tu barco en el tablero del equipo" << endl;
    cout << "Ingresa: fila,columna,orientación (ej: 1,2,H o 1,2,V): ";
    
    string input;
    getline(cin, input);
    
    // Parsear entrada
    size_t comma1 = input.find(',');
    size_t comma2 = input.find(',', comma1 + 1);
    
    if (comma1 != string::npos && comma2 != string::npos) {
        try {
            int x = stoi(input.substr(0, comma1));
            int y = stoi(input.substr(comma1 + 1, comma2 - comma1 - 1));
            char orientChar = toupper(input.substr(comma2 + 1)[0]);
            
            // Para esta función, necesitamos conocer el tamaño del tablero
            // Por ahora usamos el tamaño por defecto, pero esto debería ser un parámetro
            int board_size = 8; // Temporal - debería ser parámetro
            if (x < 0 || x >= board_size || y < 0 || y >= board_size) {
                cout << "❌ Coordenadas fuera de rango." << endl;
                return false;
            }
            
            if (orientChar != 'H' && orientChar != 'V') {
                cout << "❌ Orientación inválida. Use H o V." << endl;
                return false;
            }
            
            // Determinar tipo de barco (rotativo: DESTROYER, BATTLESHIP, AIRCRAFT_CARRIER)
            static int ship_type_index = 0;
            ShipType shipTypes[3] = {DESTROYER, BATTLESHIP, AIRCRAFT_CARRIER};
            ShipType currentShipType = shipTypes[ship_type_index % 3];
            ship_type_index++;
            
            Orientation orientation = (orientChar == 'H') ? HORIZONTAL : VERTICAL;
            
            // Crear mensaje de colocación
            GameMessage place_msg(MSG_PLACE_SHIP);
            place_msg.x = x;
            place_msg.y = y;
            place_msg.data1 = static_cast<int>(currentShipType);
            place_msg.data2 = static_cast<int>(orientation);
            
            if (sendGameMessage(socket, place_msg)) {
                cout << "📤 Barco enviado a posición [" << x << "," << y << "]..." << endl;
                return true;
            } else {
                cout << "❌ Error enviando barco" << endl;
                return false;
            }
            
        } catch (...) {
            cout << "❌ Error en formato de entrada." << endl;
        }
    } else {
        cout << "❌ Formato inválido. Use: fila,columna,orientación" << endl;
    }
    
    return false;
}

int main() {
    cout << "=== CLIENTE BATTLESHIP ===" << endl;
    cout << "Conectando al servidor " << DEFAULT_SERVER_IP << ":" << DEFAULT_SERVER_PORT << "..." << endl;
    
    // Paso 1: Crear socket del cliente
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        cout << "Error: No se pudo crear el socket del cliente" << endl;
        return 1;
    }
    cout << "✓ Socket del cliente creado" << endl;
    
    // Paso 2: Configurar dirección del servidor
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(DEFAULT_SERVER_PORT);
    
    // Convertir IP de texto a formato binario
    if (inet_pton(AF_INET, DEFAULT_SERVER_IP.c_str(), &server_address.sin_addr) <= 0) {
        cout << "Error: Dirección IP inválida" << endl;
        close(client_socket);
        return 1;
    }
    cout << "✓ Dirección del servidor configurada" << endl;
    
    // Paso 3: Conectarse al servidor
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        cout << "Error: No se pudo conectar al servidor" << endl;
        cout << "¿Está el servidor ejecutándose?" << endl;
        close(client_socket);
        return 1;
    }
    cout << "✓ Conectado al servidor exitosamente!" << endl;
    
    // Paso 4: Recibir mensaje de bienvenida del servidor
    GameMessage welcome_msg;
    int my_player_id = -1;
    if (receiveGameMessage(client_socket, welcome_msg)) {
        if (welcome_msg.type == MSG_WELCOME) {
            my_player_id = welcome_msg.player_id;
            cout << "📩 Servidor dice: " << welcome_msg.text << endl;
            cout << "🆔 Tu ID de jugador: " << my_player_id << endl;
        } else {
            cout << "⚠️ Mensaje inesperado del servidor (tipo " << welcome_msg.type << ")" << endl;
        }
    } else {
        cout << "❌ Error recibiendo mensaje de bienvenida" << endl;
        close(client_socket);
        return 1;
    }
    
    // Paso 4.5: Recibir tamaño del tablero del servidor
    int board_size = BOARD_SIZE_1VS1; // Tamaño por defecto
    GameMessage board_size_msg;
    if (receiveGameMessage(client_socket, board_size_msg)) {
        if (board_size_msg.type == MSG_BOARD_SIZE) {
            board_size = board_size_msg.data1;
            cout << "📐 " << board_size_msg.text << endl;
            cout << "🎯 Configurando tableros con tamaño " << board_size << "x" << board_size << endl;
        } else {
            cout << "⚠️ Mensaje inesperado del servidor (esperaba tamaño de tablero, tipo " << board_size_msg.type << ")" << endl;
        }
    } else {
        cout << "❌ Error recibiendo tamaño de tablero" << endl;
        close(client_socket);
        return 1;
    }
    
    // Inicializar tableros con el tamaño correcto
    Board myBoard(board_size);
    Board enemyBoard(board_size);
    initializeBoard(myBoard);
    initializeBoard(enemyBoard);
    
    // Paso 5: Enviar nombre al servidor
    string player_name;
    cout << "\n👤 Ingresa tu nombre: ";
    getline(cin, player_name);
    
    GameMessage name_msg(MSG_SET_NAME);
    strncpy(name_msg.text, player_name.c_str(), sizeof(name_msg.text) - 1);
    
    if (sendGameMessage(client_socket, name_msg)) {
        cout << "✓ Nombre enviado al servidor" << endl;
    } else {
        cout << "❌ Error enviando nombre" << endl;
        close(client_socket);
        return 1;
    }
    
    cout << "\n⏳ Esperando respuesta del servidor..." << endl;
    
    // Variables para el juego
    bool shipPlacementPhase = true;
    bool awaiting_result = false; // true cuando enviamos un disparo y esperamos resultado
    
    // Paso 6: Loop principal - escuchar mensajes del servidor
    bool game_running = true;
    while (game_running) {
        GameMessage server_msg;
        if (receiveGameMessage(client_socket, server_msg)) {
            switch (server_msg.type) {
                case MSG_WAIT:
                    cout << "⏳ " << server_msg.text << endl;
                    // Detectar fin de la fase de colocación
                    if (string(server_msg.text).find("combate ha comenzado") != string::npos || 
                        string(server_msg.text).find("El combate ha comenzado") != string::npos) {
                        shipPlacementPhase = false;
                        cout << "🎯 Cambiando a fase de combate" << endl;
                    }
                    // Detectar si es tiempo de colocar barcos (modo 1vs1)
                    else if (shipPlacementPhase && (string(server_msg.text).find("Coloca tus") != string::npos)) {
                        handleShipPlacement(client_socket, myBoard);
                        shipPlacementPhase = false;
                    }
                    // En modo 2vs2, cuando es nuestro turno individual, pedimos coordenadas directamente
                    else if (shipPlacementPhase && string(server_msg.text).find("Tu turno: coloca barco") != string::npos) {
                        // Esto es modo 2vs2, pedimos solo las coordenadas del barco actual
                        // Repetir hasta que la colocación sea exitosa
                        bool placement_successful = false;
                        while (!placement_successful) {
                            placement_successful = handleSingleShipPlacement(client_socket);
                        }
                    }
                    break;
                    
                case MSG_YOUR_TURN:
                    if (!shipPlacementPhase) {
                        // Solicitar disparo al usuario (handleShooting marcará awaiting_result)
                        if (!handleShooting(client_socket, myBoard, enemyBoard, awaiting_result)) {
                            cout << "❌ Error al enviar disparo" << endl;
                        }
                    }
                    break;
                    
                case MSG_SHOT_RESULT: {
                    // El servidor usa data1=1 para hit, data2=1 para sunk, x,y con coordenadas
                    // player_id indica quién disparó
                    bool hit = (server_msg.data1 == 1);
                    bool sunk = (server_msg.data2 == 1);
                    int rx = server_msg.x;
                    int ry = server_msg.y;
                    int shooter_id = server_msg.player_id;

                    cout << "\n🔍 DEBUG: Shot result - shooter=" << shooter_id << ", my_id=" << my_player_id 
                         << ", hit=" << hit << ", pos=[" << rx << "," << ry << "]" << endl;
                    cout << "   sameTeam(" << shooter_id << ", " << my_player_id << ") = " 
                         << (sameTeam(shooter_id, my_player_id, myBoard.size) ? "true" : "false") << endl;

                    if (shooter_id == my_player_id) {
                        // Yo disparé - actualizar tablero enemigo
                        cout << "   -> Yo disparé, actualizando enemyBoard" << endl;
                        updateEnemyBoard(enemyBoard, rx, ry, hit);
                        awaiting_result = false;

                        if (hit) {
                            cout << "🎯 ¡IMPACTO en [" << rx << "," << ry << "]!" << endl;
                            if (sunk) cout << "💥 ¡Hundiste un barco!" << endl;
                        } else {
                            cout << "🌊 Agua en [" << rx << "," << ry << "]" << endl;
                        }
                    } else if (sameTeam(shooter_id, my_player_id, myBoard.size)) {
                        // Mi compañero disparó - actualizar tablero enemigo
                        cout << "   -> Mi compañero disparó, actualizando enemyBoard" << endl;
                        cout << "   -> ANTES: enemyBoard[" << rx << "][" << ry << "] = " << enemyBoard.grid[rx][ry] << endl;
                        updateEnemyBoard(enemyBoard, rx, ry, hit);
                        cout << "   -> DESPUÉS: enemyBoard[" << rx << "][" << ry << "] = " << enemyBoard.grid[rx][ry] << endl;
                        
                        if (hit) {
                            cout << "🤝 Tu compañero impactó en [" << rx << "," << ry << "]!" << endl;
                            if (sunk) cout << "💥 Tu compañero hundió un barco!" << endl;
                        } else {
                            cout << "🌊 Tu compañero falló en [" << rx << "," << ry << "] - marcado en tablero ENEMIGO" << endl;
                        }
                    } else {
                        // El equipo enemigo disparó - actualizar mi tablero
                        cout << "   -> Enemigo disparó, actualizando myBoard" << endl;
                        cout << "   -> ANTES: myBoard[" << rx << "][" << ry << "] = " << myBoard.grid[rx][ry] << endl;
                        if (hit) {
                            myBoard.grid[rx][ry] = HIT;
                            cout << "⚠️ Te han impactado en [" << rx << "," << ry << "]!" << endl;
                            if (sunk) cout << "💥 Un barco tuyo se hundió!" << endl;
                        } else {
                            myBoard.grid[rx][ry] = MISS;
                            cout << "✅ El oponente falló en [" << rx << "," << ry << "] - marcado en MI tablero" << endl;
                        }
                        cout << "   -> DESPUÉS: myBoard[" << rx << "][" << ry << "] = " << myBoard.grid[rx][ry] << endl;
                    }
                    
                    // Mostrar estado actualizado de ambos tableros
                    printBothBoards(myBoard, enemyBoard);
                    break;
                }
                    
                case MSG_GAME_OVER: {
                    cout << "\n🏁 " << server_msg.text << endl;
                    // data1 = 1 si este cliente ganó, 0 si perdió (según server.cpp)
                    if (server_msg.data1 == 1) {
                        cout << "🎉 ¡Has ganado!" << endl;
                    } else {
                        cout << "😞 Has perdido. Suerte la próxima vez." << endl;
                    }
                    cout << "\nTu tablero final:" << endl;
                    printBoard(myBoard, true);
                    cout << "\nTablero enemigo final:" << endl;
                    printBoard(enemyBoard, false);
                    game_running = false;
                    break;
                }
                    
                case MSG_ERROR:
                    cout << "❌ Error del servidor: " << server_msg.text << endl;
                    break;
                    
                case MSG_BOARD_STATE: {
                    // Recibir estado del tablero del equipo
                    if (strlen(server_msg.text) > 0) {
                        // Primer mensaje con descripción
                        cout << "\n📋 " << server_msg.text << endl;
                        
                        // Crear tablero temporal para recibir el estado
                        Board teamBoard(myBoard.size); // Usar el mismo tamaño que mi tablero
                        initializeBoard(teamBoard);
                        
                        // Recibir estado de todas las celdas
                        for (int i = 0; i < teamBoard.size * teamBoard.size; i++) {
                            GameMessage cell_msg;
                            if (receiveGameMessage(client_socket, cell_msg)) {
                                if (cell_msg.type == MSG_BOARD_STATE) {
                                    teamBoard.grid[cell_msg.x][cell_msg.y] = cell_msg.data1;
                                }
                            }
                        }
                        
                        // Mostrar el tablero del equipo
                        cout << "Estado actual del tablero de tu equipo:" << endl;
                        printBoard(teamBoard, true);
                        
                        // Si ya terminó la fase de colocación, actualizar myBoard con el estado del equipo
                        if (!shipPlacementPhase) {
                            cout << "🔄 Actualizando tu tablero con los barcos del equipo..." << endl;
                            myBoard = teamBoard;
                        }
                    }
                    break;
                }
                    
                default:
                    cout << "❓ Mensaje desconocido del servidor (tipo " << server_msg.type << ")" << endl;
                    break;
            }
        } else {
            cout << "❌ Error recibiendo mensaje del servidor o conexión cerrada" << endl;
            game_running = false;
        }
    }
    
    cout << "\n👋 Desconectando del servidor..." << endl;
    close(client_socket);
    return 0;
}