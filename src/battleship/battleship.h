#ifndef BATTLESHIP_H
#define BATTLESHIP_H

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <ctime> // Para el tiempo

using namespace std;

// Constantes del juego
const int BOARD_SIZE_1VS1 = 6;   // Tablero 8x8 para modo 1vs1
const int BOARD_SIZE_2VS2 = 8;  // Tablero 10x10 para modo 2vs2 (más barcos)
const int NUM_SHIPS = 3;         // 3 barcos por jugador en ambos modos
const int MAX_PLAYERS = 4;       // Máximo para modo 2vs2

// === MODOS DE JUEGO ===
enum GameMode {
    MODE_1VS1 = 1,
    MODE_2VS2 = 2
};

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
const int MSG_BOARD_STATE = 16; // Estado del tablero (para modo 2vs2)
const int MSG_BOARD_SIZE = 17;  // Tamaño del tablero

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
    int size;  // Tamaño dinámico del tablero
    
    // Constructores
    Board();                    // Constructor por defecto (8x8)
    Board(int board_size);      // Constructor con tamaño específico
};

// === FUNCIONES UTILITARIAS ===
int getBoardSizeForMode(GameMode mode);

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
    int hits;           // Aciertos
    int total_shots;    // Disparos totales
    
    // Constructor
    Player();
    Player(int player_id, const string& player_name);
    
    // Métodos
    int ships_remaining() const;
    bool all_ships_placed() const;
};

// Estructura de equipo para modo 2vs2
struct Team {
    int id;                         // ID del equipo (0 o 1)
    int player_ids[2];             // IDs de los jugadores en el equipo
    Board shared_board;            // Tablero compartido del equipo
    vector<Ship> team_ships;       // Todos los barcos del equipo (6 total)
    int ships_placed_count;        // Barcos ya colocados
    int current_placing_player;    // Jugador que está colocando ahora
    
    // Constructor
    Team();
    Team(int board_size);          // Constructor con tamaño de tablero específico
    
    // Métodos
    int ships_remaining() const;
    bool all_ships_placed() const;
    void add_player(int player_id);
};

// Estructura del juego
struct Game {
    Player players[MAX_PLAYERS];    // Hasta 4 jugadores
    Team teams[2];                  // 2 equipos para modo 2vs2
    GameState state;
    GameMode mode;                  // Modo de juego seleccionado
    int current_turn;
    int winner;
    int active_players;             // Número real de jugadores (2 o 4)
    int turn_count;         // Contador de turnos
    time_t start_time;      // Tiempo de inicio
    
    // Constructor
    Game();
    
    // Métodos
    bool is_game_over() const;
    void switch_turn();
    void set_mode(GameMode game_mode, int num_players);
    void setup_teams(GameMode mode);   // Configurar equipos para modo 2vs2 con tamaño correcto
};

// === DECLARACIONES DE FUNCIONES ===

// Funciones del tablero (board.cpp)
void initializeBoard(Board& board);
void displayBoard(const Board& board, bool hide_ships = false);
void displayBothBoards(const Player& player);
bool isValidPosition(int x, int y, int board_size);
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
void initializeGame(Game& game, GameMode mode);
bool addPlayer(Game& game, const string& name);
void startGame(Game& game);
bool processMove(Game& game, int x, int y);
string getGameStatus(const Game& game);

// Funciones para modo 2vs2
int getNextPlacingPlayer(const Game& game, int ship_number);
int getPlayerTeam(int player_id);
bool isTeamMate(int player1, int player2);

// Funciones para guardar estadísticas
void guardarEstadisticasPartida1v1(int jugador1Aciertos, int jugador1Fallos,
                                    int jugador2Aciertos, int jugador2Fallos,
                                    int turnos, time_t tiempoInicio, time_t tiempoFin);

void guardarEstadisticasPartida2v2(int jugador1Aciertos, int jugador1Fallos,
                                    int jugador2Aciertos, int jugador2Fallos,
                                    int jugador3Aciertos, int jugador3Fallos,
                                    int jugador4Aciertos, int jugador4Fallos,
                                    int turnos, time_t tiempoInicio, time_t tiempoFin);

// === FUNCIONES DE RED ===

// Enviar/recibir mensajes estructurados
bool sendGameMessage(int socket, const GameMessage& msg);
bool receiveGameMessage(int socket, GameMessage& msg);

// Funciones helper para crear mensajes comunes
GameMessage createWelcomeMessage(int player_id);
GameMessage createWaitMessage(const string& reason);

#endif // BATTLESHIP_H