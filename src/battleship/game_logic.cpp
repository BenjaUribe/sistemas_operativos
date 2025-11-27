#include "battleship.h"
#include <fstream>
#include <ctime>
#include <iomanip>

// === FUNCIONES DE ESTADÍSTICAS ===

void initGameStats(GameStats& stats, GameMode mode, int num_players) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        stats.player_hits[i] = 0;
        stats.player_misses[i] = 0;
    }
    stats.total_turns = 0;
    stats.winner_name = "";
    stats.game_start = time(nullptr);
    stats.game_end = 0;
    stats.mode = mode;
    stats.num_players = num_players;
}

void updateStats(GameStats& stats, int player_id, bool is_hit) {
    if (player_id >= 0 && player_id < MAX_PLAYERS) {
        if (is_hit) {
            stats.player_hits[player_id]++;
            cout << "[STATS] Jugador " << player_id << " acierto. Total: " << stats.player_hits[player_id] << endl;
        } else {
            stats.player_misses[player_id]++;
            cout << "[STATS] Jugador " << player_id << " fallo. Total: " << stats.player_misses[player_id] << endl;
        }
    } else {
        cout << "[STATS ERROR] player_id inválido: " << player_id << endl;
    }
}

void saveGameStats(const GameStats& stats, const Player players[], const string player_names[], int num_players) {
    // Debug: Mostrar estadísticas antes de guardar
    cout << "\n[DEBUG] Guardando estadísticas:" << endl;
    for (int i = 0; i < num_players; i++) {
        cout << "  Jugador " << i << " (" << player_names[i] << "): " 
             << stats.player_hits[i] << " aciertos, " 
             << stats.player_misses[i] << " fallos" << endl;
    }
    cout << "  Total turnos: " << stats.total_turns << endl;
    
    // Crear directorio data si no existe (compatible con Windows)
    #ifdef _WIN32
        system("if not exist \"data\" mkdir \"data\"");
    #else
        system("mkdir -p data");
    #endif
    
    ofstream file("data/battleship_stats.txt", ios::app);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo de estadísticas" << endl;
        return;
    }
    
    // Formatear fecha
    char buffer[80];
    struct tm* timeinfo = localtime(&stats.game_start);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    // Calcular duración
    int duration = static_cast<int>(difftime(stats.game_end, stats.game_start));
    int minutes = duration / 60;
    int seconds = duration % 60;
    
    // Escribir encabezado
    file << "========================================\n";
    file << "PARTIDA - " << buffer << "\n";
    file << "Modo: " << (stats.mode == MODE_1VS1 ? "1vs1" : "2vs2") << "\n";
    file << "========================================\n";
    
    if (stats.mode == MODE_1VS1) {
        // Modo 1vs1: Mostrar 2 jugadores
        for (int i = 0; i < 2; i++) {
            int total_shots = stats.player_hits[i] + stats.player_misses[i];
            double precision = total_shots > 0 
                ? (stats.player_hits[i] * 100.0) / total_shots 
                : 0.0;
            
            file << "Jugador " << (i + 1) << ": " << player_names[i] << "\n";
            file << "  - Aciertos: " << stats.player_hits[i] << "\n";
            file << "  - Fallos: " << stats.player_misses[i] << "\n";
            file << "  - Precisión: " << fixed << setprecision(2) << precision << "%\n";
            file << "\n";
        }
    } else {
        // Modo 2vs2: Mostrar por equipos
        file << "EQUIPO 1:\n";
        int team1_hits = 0, team1_misses = 0;
        for (int i = 0; i < 2; i++) {
            int total_shots = stats.player_hits[i] + stats.player_misses[i];
            double precision = total_shots > 0 
                ? (stats.player_hits[i] * 100.0) / total_shots 
                : 0.0;
            
            file << "  " << player_names[i] << ":\n";
            file << "    - Aciertos: " << stats.player_hits[i] << "\n";
            file << "    - Fallos: " << stats.player_misses[i] << "\n";
            file << "    - Precisión: " << fixed << setprecision(2) << precision << "%\n";
            
            team1_hits += stats.player_hits[i];
            team1_misses += stats.player_misses[i];
        }
        int team1_total = team1_hits + team1_misses;
        double team1_precision = team1_total > 0 ? (team1_hits * 100.0) / team1_total : 0.0;
        file << "  Total Equipo 1: " << team1_hits << " aciertos, " << team1_misses << " fallos";
        file << " (" << fixed << setprecision(2) << team1_precision << "%)\n\n";
        
        file << "EQUIPO 2:\n";
        int team2_hits = 0, team2_misses = 0;
        for (int i = 2; i < 4; i++) {
            int total_shots = stats.player_hits[i] + stats.player_misses[i];
            double precision = total_shots > 0 
                ? (stats.player_hits[i] * 100.0) / total_shots 
                : 0.0;
            
            file << "  " << player_names[i] << ":\n";
            file << "    - Aciertos: " << stats.player_hits[i] << "\n";
            file << "    - Fallos: " << stats.player_misses[i] << "\n";
            file << "    - Precisión: " << fixed << setprecision(2) << precision << "%\n";
            
            team2_hits += stats.player_hits[i];
            team2_misses += stats.player_misses[i];
        }
        int team2_total = team2_hits + team2_misses;
        double team2_precision = team2_total > 0 ? (team2_hits * 100.0) / team2_total : 0.0;
        file << "  Total Equipo 2: " << team2_hits << " aciertos, " << team2_misses << " fallos";
        file << " (" << fixed << setprecision(2) << team2_precision << "%)\n\n";
    }
    
    file << "Total de turnos: " << stats.total_turns << "\n";
    file << "Duración: " << minutes << "m " << seconds << "s\n";
    file << "Ganador: " << stats.winner_name << "\n";
    file << "========================================\n\n";
    
    file.close();
    cout << "Estadísticas guardadas en data/battleship_stats.txt" << endl;
}

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