#include "battleship.h"
#include <fstream>
#include <iomanip>
#include <sys/stat.h>

// === IMPLEMENTACIONES DE TEAM ===

// Constructor de Team
Team::Team() : id(0), ships_placed_count(0), current_placing_player(0) {
    player_ids[0] = -1;
    player_ids[1] = -1;
    shared_board = Board();
    team_ships.clear();
}

// Constructor con tamaño específico
Team::Team(int board_size) : id(0), ships_placed_count(0), current_placing_player(0) {
    player_ids[0] = -1;
    player_ids[1] = -1;
    shared_board = Board(board_size);
    team_ships.clear();
}

// Contar barcos restantes del equipo
int Team::ships_remaining() const {
    int count = 0;
    for (const auto& ship : team_ships) {
        if (!ship.sunk()) {
            count++;
        }
    }
    return count;
}

// Verificar si todos los barcos del equipo están colocados
bool Team::all_ships_placed() const {
    return ships_placed_count >= (NUM_SHIPS * 2); // 3 barcos por jugador * 2 jugadores = 6
}

// Agregar jugador al equipo
void Team::add_player(int player_id) {
    if (player_ids[0] == -1) {
        player_ids[0] = player_id;
    } else if (player_ids[1] == -1) {
        player_ids[1] = player_id;
    }
}

// === LÓGICA PRINCIPAL DEL JUEGO ===

// Constructor del juego
Game::Game() : state(WAITING_FOR_PLAYERS), mode(MODE_1VS1), current_turn(0), winner(-1), active_players(2) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i] = Player();
    }
}

// Verificar si el juego ha terminado
bool Game::is_game_over() const {
    return state == FINISHED;
}

// Cambiar turno
void Game::switch_turn() {
    if (state == IN_PROGRESS) {
        players[current_turn].is_turn = false;
        current_turn = (current_turn + 1) % active_players;
        players[current_turn].is_turn = true;
        
        // Incrementar contador de turnos solo cuando vuelve al jugador 0
        // (así un "turno" cuenta cuando todos han jugado)
        if (current_turn == 0) {
            turn_count++;
        }
    }
}

// Configurar modo de juego
void Game::set_mode(GameMode game_mode, int num_players) {
    mode = game_mode;
    active_players = num_players;
    
    cout << "🎮 Modo configurado: ";
    if (mode == MODE_1VS1) {
        cout << "1vs1 (" << active_players << " jugadores)" << endl;
    } else if (mode == MODE_2VS2) {
        cout << "2vs2 (" << active_players << " jugadores)" << endl;
        setup_teams(game_mode); // Configurar equipos automáticamente con modo correcto
    }
}

// Configurar equipos para modo 2vs2
void Game::setup_teams(GameMode mode) {
    // Equipo 1: Jugadores 0 y 1
    teams[0].id = 0;
    teams[0].add_player(0);
    teams[0].add_player(1);
    
    // Equipo 2: Jugadores 2 y 3
    teams[1].id = 1;
    teams[1].add_player(2);
    teams[1].add_player(3);
    
    int board_size = getBoardSizeForMode(mode);
    cout << "- Equipo 1: Jugadores 0 y 1 (tablero " << board_size << "x" << board_size << ")" << endl;
    cout << "- Equipo 2: Jugadores 2 y 3 (tablero " << board_size << "x" << board_size << ")" << endl;
}

// Inicializar juego
void initializeGame(Game& game, GameMode mode) {
    game.state = WAITING_FOR_PLAYERS;
    game.current_turn = 0;
    game.winner = -1;
    game.mode = mode;
    
    // Configurar tableros con tamaño apropiado según modo
    int board_size = getBoardSizeForMode(mode);
    
    for (int i = 0; i < MAX_PLAYERS; i++) {
        game.players[i] = Player();
        // Inicializar tableros con tamaño dinámico
        game.players[i].own_board = Board(board_size);
        initializeBoard(game.players[i].own_board);
    }
    
    // Para modo 2vs2, también inicializar tableros de equipos
    if (mode == MODE_2VS2) {
        game.teams[0].shared_board = Board(board_size);
        game.teams[1].shared_board = Board(board_size);
        initializeBoard(game.teams[0].shared_board);
        initializeBoard(game.teams[1].shared_board);
    }
}

// Agregar jugador al juego
bool addPlayer(Game& game, const string& name) {
    if (game.state != WAITING_FOR_PLAYERS) {
        return false;
    }
    
    // Buscar slot libre
    for (int i = 0; i < 2; i++) {
        if (game.players[i].name.empty()) {
            initializePlayer(game.players[i], i, name);
            cout << "Jugador " << name << " se ha unido como Player " << (i + 1) << endl;
            
            // Si ya hay 2 jugadores, cambiar a fase de colocación
            if (i == 1) {
                game.state = PLACING_SHIPS;
                cout << "¡Ambos jugadores conectados! Fase de colocación de barcos." << endl;
            }
            return true;
        }
    }
    
    return false; // Juego lleno
}

// Iniciar el juego (después de colocar barcos)
void startGame(Game& game) {
    if (game.state != PLACING_SHIPS) {
        return;
    }
    
    if (game.mode == MODE_1VS1) {
        // Verificar que ambos jugadores estén listos
        bool both_ready = true;
        for (int i = 0; i < 2; i++) {
            if (!game.players[i].is_ready) {
                both_ready = false;
                break;
            }
        }
        
        if (both_ready) {
            game.state = IN_PROGRESS;
            game.current_turn = 0;
            game.players[0].is_turn = true;
            game.players[1].is_turn = false;
            
            // Inicializar contadores de estadísticas
            game.turn_count = 0;
            game.start_time = time(nullptr);
            for (int i = 0; i < 2; i++) {
                game.players[i].hits = 0;
                game.players[i].total_shots = 0;
            }
            
            cout << "\n¡Juego iniciado! " << game.players[0].name << " comienza." << endl;
        }
    } else {
        // Modo 2vs2: Verificar que ambos equipos tengan todos sus barcos
        cout << "\n Verificando estado de equipos para iniciar combate..." << endl;
        cout << "- Equipo 1: " << game.teams[0].ships_placed_count << "/6 barcos" << endl;
        cout << "- Equipo 2: " << game.teams[1].ships_placed_count << "/6 barcos" << endl;
        
        bool teams_ready = (game.teams[0].ships_placed_count == 6 && 
                           game.teams[1].ships_placed_count == 6);
        
        if (teams_ready) {
            cout << "\n Ambos equipos listos - Cambiando estado a IN_PROGRESS" << endl;
            game.state = IN_PROGRESS;
            game.current_turn = 0; // Empezar con jugador 0 (Equipo 1)
            
            // Configurar turnos
            for (int i = 0; i < game.active_players; i++) {
                game.players[i].is_turn = (i == 0);
            }
            
            // Inicializar contadores de cada jugador
            game.turn_count = 0;
            game.start_time = time(nullptr);
            for (int i = 0; i < game.active_players; i++) {
                game.players[i].hits = 0;
                game.players[i].total_shots = 0;
            }
            
            cout << " Juego 2vs2 iniciado! " << game.players[0].name << " (Equipo 1) comienza." << endl;
            cout << " Estado del juego: " << (game.state == IN_PROGRESS ? "IN_PROGRESS" : "OTRO") << endl;
        } else {
            cout << "\n Error: Equipos no están listos para el combate" << endl;
            cout << " Equipo 1: " << game.teams[0].ships_placed_count << "/6 barcos" << endl;
            cout << " Equipo 2: " << game.teams[1].ships_placed_count << "/6 barcos" << endl;
        }
    }
}

// Procesar un movimiento (disparo)
bool processMove(Game& game, int x, int y) {
    if (game.state != IN_PROGRESS) {
        return false;
    }
    
    int attacker_idx = game.current_turn;
    int defender_idx = (game.current_turn + 1) % 2;
    
    Player& attacker = game.players[attacker_idx];
    Player& defender = game.players[defender_idx];
    
    // Realizar disparo
    if (!makeShot(attacker, defender, x, y)) {
        return false;
    }
    
    // Verificar condición de victoria
    if (hasWon(defender)) {
        game.state = FINISHED;
        game.winner = attacker_idx;
        cout << "\n¡" << attacker.name << " ha ganado el juego!" << endl;
        
        // GUARDAR ESTADÍSTICAS AL FINALIZAR LA PARTIDA
        time_t tiempo_fin = time(nullptr);
        
        if (game.mode == MODE_2VS2) {
            guardarEstadisticasPartida2v2(game.players[0].hits, game.players[0].total_shots,
                                          game.players[1].hits, game.players[1].total_shots,
                                          game.players[2].hits, game.players[2].total_shots,
                                          game.players[3].hits, game.players[3].total_shots,
                                          game.turn_count, game.start_time, tiempo_fin);
        } else {
            guardarEstadisticasPartida1v1(game.players[0].hits, game.players[0].total_shots,
                                          game.players[1].hits, game.players[1].total_shots,
                                          game.turn_count, game.start_time, tiempo_fin);
        }
        
        return true;
    }
    
    // Cambiar turno
    game.switch_turn();
    cout << "Turno de " << game.players[game.current_turn].name << endl;
    
    return true;
}

// Obtener estado del juego como string
string getGameStatus(const Game& game) {
    switch (game.state) {
        case WAITING_FOR_PLAYERS:
            return "Esperando jugadores...";
        case PLACING_SHIPS:
            return "Colocando barcos...";
        case IN_PROGRESS:
            return "Juego en progreso - Turno de " + game.players[game.current_turn].name;
        case FINISHED:
            if (game.winner >= 0 && game.winner < 2) {
                return "Juego terminado - Ganador: " + game.players[game.winner].name;
            }
            return "Juego terminado";
        default:
            return "Estado desconocido";
    }
}

// === FUNCIONES PARA MODO 2VS2 ===

// Calcular el siguiente jugador que debe colocar un barco (colocación alternada)
int getNextPlacingPlayer(const Game& game, int ship_number) {
    // Patrón de colocación alternada:
    // Barco 1: Jugador 0 (Equipo A)
    // Barco 2: Jugador 2 (Equipo B)
    // Barco 3: Jugador 1 (Equipo A)
    // Barco 4: Jugador 3 (Equipo B)
    // Barco 5: Jugador 0 (Equipo A)
    // Barco 6: Jugador 2 (Equipo B)
    // ... y así sucesivamente
    
    int round = ship_number / 2;        // Cada 2 barcos cambia el jugador del equipo
    int team = ship_number % 2;         // Alterna entre equipos (0 = equipo A, 1 = equipo B)
    int player_in_team = round % 2;     // Alterna jugadores dentro del equipo
    
    if (team == 0) {
        // Equipo A: jugadores 0 y 1
        return (player_in_team == 0) ? 0 : 1;
    } else {
        // Equipo B: jugadores 2 y 3
        return (player_in_team == 0) ? 2 : 3;
    }
}

// Obtener el equipo de un jugador
int getPlayerTeam(int player_id) {
    return (player_id < 2) ? 0 : 1;  // Jugadores 0,1 = Equipo 0; Jugadores 2,3 = Equipo 1
}

// Verificar si dos jugadores son compañeros de equipo
bool isTeamMate(int player1, int player2) {
    return getPlayerTeam(player1) == getPlayerTeam(player2);
}

// === FUNCIONES PARA GUARDAR ESTADÍSTICAS ===

// Función para verificar si un directorio existe
bool directorioExiste(const std::string& ruta) {
    struct stat info;
    return (stat(ruta.c_str(), &info) == 0 && (info.st_mode & S_IFDIR));
}

// Función para crear directorio si no existe
void crearDirectorioSiNoExiste(const std::string& ruta) {
    if (!directorioExiste(ruta)) {
        #ifdef _WIN32
            _mkdir(ruta.c_str());
        #else
            mkdir(ruta.c_str(), 0777);
        #endif
    }
}

void guardarEstadisticasPartida1v1(int jugador1Aciertos, int jugador1Fallos,
                                    int jugador2Aciertos, int jugador2Fallos,
                                    int turnos, time_t tiempoInicio, time_t tiempoFin) {
    std::cout << "\n=== GUARDANDO ESTADÍSTICAS 1v1 ===" << std::endl;
    std::cout << "J1: " << jugador1Aciertos << " aciertos, " << jugador1Fallos << " fallos" << std::endl;
    std::cout << "J2: " << jugador2Aciertos << " aciertos, " << jugador2Fallos << " fallos" << std::endl;
    std::cout << "Turnos: " << turnos << std::endl;
    
    // Crear directorio data si no existe
    crearDirectorioSiNoExiste("data");
    
    std::ofstream archivo("data/estadisticas_partidas.txt", std::ios::app);
    
    if (archivo.is_open()) {
        int duracion = static_cast<int>(difftime(tiempoFin, tiempoInicio));
        
        std::cout << "Escribiendo: (" << jugador1Aciertos << "," << jugador1Fallos << "),(" 
                  << jugador2Aciertos << "," << jugador2Fallos << "),(-1,-1),(-1,-1),1v1," 
                  << duracion << "," << turnos << std::endl;
        
        archivo << "(" << jugador1Aciertos << "," << jugador1Fallos << ")," 
                << "(" << jugador2Aciertos << "," << jugador2Fallos << ")," 
                << "(-1,-1)," 
                << "(-1,-1)," 
                << "1v1," 
                << duracion << "," 
                << turnos << std::endl;
        
        archivo.flush();
        archivo.close();
        std::cout << "✓ Estadísticas guardadas en data/estadisticas_partidas.txt" << std::endl;
    } else {
        std::cerr << "✗ Error: No se pudo abrir el archivo de estadísticas." << std::endl;
    }
}

void guardarEstadisticasPartida2v2(int jugador1Aciertos, int jugador1Fallos,
                                    int jugador2Aciertos, int jugador2Fallos,
                                    int jugador3Aciertos, int jugador3Fallos,
                                    int jugador4Aciertos, int jugador4Fallos,
                                    int turnos, time_t tiempoInicio, time_t tiempoFin) {
    std::cout << "\n=== GUARDANDO ESTADÍSTICAS 2v2 ===" << std::endl;
    std::cout << "J1: " << jugador1Aciertos << " aciertos, " << jugador1Fallos << " fallos" << std::endl;
    std::cout << "J2: " << jugador2Aciertos << " aciertos, " << jugador2Fallos << " fallos" << std::endl;
    std::cout << "J3: " << jugador3Aciertos << " aciertos, " << jugador3Fallos << " fallos" << std::endl;
    std::cout << "J4: " << jugador4Aciertos << " aciertos, " << jugador4Fallos << " fallos" << std::endl;
    std::cout << "Turnos: " << turnos << std::endl;
    
    // Crear directorio data si no existe
    crearDirectorioSiNoExiste("data");
    
    std::ofstream archivo("data/estadisticas_partidas.txt", std::ios::app);
    
    if (archivo.is_open()) {
        int duracion = static_cast<int>(difftime(tiempoFin, tiempoInicio));
        
        std::cout << "Escribiendo: (" << jugador1Aciertos << "," << jugador1Fallos << "),(" 
                  << jugador2Aciertos << "," << jugador2Fallos << "),(" 
                  << jugador3Aciertos << "," << jugador3Fallos << "),(" 
                  << jugador4Aciertos << "," << jugador4Fallos << "),2v2," 
                  << duracion << "," << turnos << std::endl;
        
        archivo << "(" << jugador1Aciertos << "," << jugador1Fallos << ")," 
                << "(" << jugador2Aciertos << "," << jugador2Fallos << ")," 
                << "(" << jugador3Aciertos << "," << jugador3Fallos << ")," 
                << "(" << jugador4Aciertos << "," << jugador4Fallos << ")," 
                << "2v2," 
                << duracion << "," 
                << turnos << std::endl;
        
        archivo.flush();
        archivo.close();
        std::cout << "✓ Estadísticas guardadas en data/estadisticas_partidas.txt" << std::endl;
    } else {
        std::cerr << "✗ Error: No se pudo abrir el archivo de estadísticas." << std::endl;
    }
}