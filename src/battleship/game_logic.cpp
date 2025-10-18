#include "battleship.h"

// === LÓGICA PRINCIPAL DEL JUEGO ===

// Constructor del juego
Game::Game() : state(WAITING_FOR_PLAYERS), current_turn(0), winner(-1) {
    players[0] = Player();
    players[1] = Player();
}

// Verificar si el juego ha terminado
bool Game::is_game_over() const {
    return state == FINISHED;
}

// Cambiar turno
void Game::switch_turn() {
    if (state == IN_PROGRESS) {
        players[current_turn].is_turn = false;
        current_turn = (current_turn + 1) % 2;
        players[current_turn].is_turn = true;
    }
}

// Inicializar juego
void initializeGame(Game& game) {
    game.state = WAITING_FOR_PLAYERS;
    game.current_turn = 0;
    game.winner = -1;
    
    for (int i = 0; i < 2; i++) {
        game.players[i] = Player();
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