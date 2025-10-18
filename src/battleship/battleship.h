#ifndef BATTLESHIP_H
#define BATTLESHIP_H

#include <iostream>
#include <vector>
#include <string>
#include <cstring>

using namespace std;

// Constantes del juego
const int BOARD_SIZE = 5;
const int NUM_SHIPS = 3;

// === PROTOCOLO DE RED ===

// Tipos de mensajes - Cliente → Servidor
const int MSG_SET_NAME = 2;     // Enviar nombre
const int MSG_PLACE_SHIP = 3;   // Colocar barco
const int MSG_READY = 4;        // Listo para jugar
const int MSG_SHOOT = 5;        // Disparar

// Tipos de mensajes - Servidor → Cliente
const int MSG_WELCOME = 10;     // Bienvenida + ID de jugador
const int MSG_WAIT = 11;        // Esperar (otro jugador, etc.)
const int MSG_YOUR_TURN = 12;   // Es tu turno
const int MSG_SHOT_RESULT = 13; // Resultado de disparo
const int MSG_GAME_OVER = 14;   // Juego terminado
const int MSG_ERROR = 15;       // Error del servidor

// Estructura del mensaje de red
struct GameMessage {
    int type;           // Tipo de mensaje (constantes de arriba)
    int player_id;      // ID del jugador (0 o 1)
    int x, y;           // Coordenadas (para disparos/barcos)
    int data1;          // Dato extra (ShipType, resultado, etc.)
    int data2;          // Dato extra (Orientation, etc.)
    char text[64];      // Texto (nombres, mensajes)
    
    // Constructor por defecto
    GameMessage() {
        type = 0;
        player_id = 0;
        x = y = 0;
        data1 = data2 = 0;
        memset(text, 0, sizeof(text));
    }
    
    // Constructor con tipo
    GameMessage(int msg_type) : GameMessage() {
        type = msg_type;
    }
};

// Estados de las celdas
enum CellState {
    WATER = 0,      // Agua vacía
    SHIP = 1,       // Parte de un barco
    HIT = 2,        // Barco impactado
    MISS = 3        // Disparo al agua
};

// Tipos de barcos (tamaños)
enum ShipType {
    DESTROYER = 2,
    BATTLESHIP = 3,
    AIRCRAFT_CARRIER = 4
};

// Orientación del barco
enum Orientation {
    HORIZONTAL,
    VERTICAL
};

// Estados del juego
enum GameState {
    WAITING_FOR_PLAYERS,
    PLACING_SHIPS,
    IN_PROGRESS,
    FINISHED
};

// Estructura del tablero
struct Board {
    vector<vector<int>> grid;
    bool ships_placed;
    
    // Constructor
    Board();
};

// Estructura de un barco
struct Ship {
    int id;
    ShipType type;
    Orientation orientation;
    int x, y;
    int hits;
    
    // Constructor
    Ship();
    Ship(int ship_id, ShipType ship_type, int pos_x, int pos_y, Orientation orient);
    
    // Métodos
    bool sunk() const;
    int get_size() const;
};

// Estructura del jugador
struct Player {
    int id;
    string name;
    Board own_board;
    Board opponent_board;
    vector<Ship> ships;
    bool is_turn;
    bool is_ready;
    
    // Constructor
    Player();
    Player(int player_id, const string& player_name);
    
    // Métodos
    int ships_remaining() const;
    bool all_ships_placed() const;
};

// Estructura del juego
struct Game {
    Player players[2];
    GameState state;
    int current_turn;
    int winner;
    
    // Constructor
    Game();
    
    // Métodos
    bool is_game_over() const;
    void switch_turn();
};

// === DECLARACIONES DE FUNCIONES ===

// Funciones del tablero (board.cpp)
void initializeBoard(Board& board);
void displayBoard(const Board& board, bool hide_ships = false);
void displayBothBoards(const Player& player);
bool isValidPosition(int x, int y);
void updateCell(Board& board, int x, int y, CellState state);

// Funciones de barcos (ship.cpp)
bool canPlaceShip(const Board& board, const Ship& ship);
bool placeShip(Board& board, Ship& ship);
bool removeShip(Board& board, const Ship& ship);
vector<pair<int, int>> getShipCoordinates(const Ship& ship);
bool checkShipHit(Ship& ship, int x, int y);

// Funciones del jugador (player.cpp)
void initializePlayer(Player& player, int id, const string& name);
bool makeShot(Player& attacker, Player& defender, int x, int y);
bool hasWon(const Player& player);
void markPlayerReady(Player& player);

// Funciones del juego (game_logic.cpp)
void initializeGame(Game& game);
bool addPlayer(Game& game, const string& name);
void startGame(Game& game);
bool processMove(Game& game, int x, int y);
string getGameStatus(const Game& game);

// === FUNCIONES DE RED ===

// Enviar/recibir mensajes estructurados
bool sendGameMessage(int socket, const GameMessage& msg);
bool receiveGameMessage(int socket, GameMessage& msg);

// Funciones helper para crear mensajes comunes
GameMessage createWelcomeMessage(int player_id);
GameMessage createWaitMessage(const string& reason);

#endif // BATTLESHIP_H