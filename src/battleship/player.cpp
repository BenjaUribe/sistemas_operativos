#include "battleship.h"

// === IMPLEMENTACIÓN DE LA CLASE PLAYER ===

// Constructores
Player::Player() : id(0), name(""), is_turn(false), is_ready(false) {
    own_board = Board();
    opponent_board = Board();
}

// nombre  = username ??
Player::Player(int player_id, const string& player_name) 
    : id(player_id), name(player_name), is_turn(false), is_ready(false) {
    own_board = Board();
    opponent_board = Board();
}

// Contar barcos restantes (no hundidos)
int Player::ships_remaining() const {
    int count = 0;
    for (const auto& ship : ships) {
        if (!ship.sunk()) {
            count++;
        }
    }
    return count;
}

// Verificar si todos los barcos están colocados
bool Player::all_ships_placed() const {
    return ships.size() == NUM_SHIPS && own_board.ships_placed;
}

// Inicializar jugador
void initializePlayer(Player& player, int id, const string& name) {
    player.id = id;
    player.name = name;
    player.is_turn = false;
    player.is_ready = false;
    
    initializeBoard(player.own_board);
    initializeBoard(player.opponent_board);
    
    player.ships.clear();
    // No crear barcos por defecto aquí.
    // Los barcos serán añadidos cuando el cliente los coloque y el servidor reciba
    // los mensajes MSG_PLACE_SHIP. Evita duplicados en players[].ships.
}

// Realizar un disparo (versión sin estadísticas - para compatibilidad)
bool makeShot(Player& attacker, Player& defender, int x, int y) {
    return makeShot(attacker, defender, x, y, nullptr);
}

// Realizar un disparo (versión con estadísticas)
bool makeShot(Player& attacker, Player& defender, int x, int y, GameStats* stats) {
    // Verificar posición válida
    if (!isValidPosition(x, y, defender.own_board.size)) {
        return false;
    }
    
    // Verificar que no se haya disparado antes a esta posición
    if (attacker.opponent_board.grid[x][y] == HIT || 
        attacker.opponent_board.grid[x][y] == MISS) {
        return false;
    }
    
    bool is_hit = false;
    
    // Verificar si hay un barco en esa posición
    if (defender.own_board.grid[x][y] == SHIP) {
        // ¡Impacto!
        is_hit = true;
        updateCell(defender.own_board, x, y, HIT);
        updateCell(attacker.opponent_board, x, y, HIT);
        
        // Encontrar qué barco fue impactado y actualizar sus hits
        for (auto& ship : defender.ships) {
            if (checkShipHit(ship, x, y)) {
                cout << "¡Impacto en " << defender.name << "!";
                if (ship.sunk()) {
                    cout << " ¡Barco hundido!";
                }
                cout << endl;
                break;
            }
        }
    } else {
        // Agua
        is_hit = false;
        updateCell(defender.own_board, x, y, MISS);
        updateCell(attacker.opponent_board, x, y, MISS);
        cout << "Agua..." << endl;
    }
    
    // Actualizar estadísticas si se proporcionó
    if (stats != nullptr) {
        updateStats(*stats, attacker.id, is_hit);
    }
    
    return true;
}

// Verificar si el jugador ha ganado
bool hasWon(const Player& player) {
    return player.ships_remaining() == 0;
}

// Marcar jugador como listo
void markPlayerReady(Player& player) {
    if (player.all_ships_placed()) {
        player.is_ready = true;
        player.own_board.ships_placed = true;
    }
}