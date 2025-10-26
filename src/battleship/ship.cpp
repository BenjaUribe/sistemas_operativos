#include "battleship.h"

// === IMPLEMENTACIÓN DE LA CLASE SHIP ===

// Constructores
Ship::Ship() : id(0), type(DESTROYER), orientation(HORIZONTAL), x(0), y(0), hits(0) {}

Ship::Ship(int ship_id, ShipType ship_type, int pos_x, int pos_y, Orientation orient) 
    : id(ship_id), type(ship_type), orientation(orient), x(pos_x), y(pos_y), hits(0) {}

// Verificar si el barco está hundido
bool Ship::sunk() const {
    return hits >= get_size();
}

// Obtener tamaño del barco
int Ship::get_size() const {
    return static_cast<int>(type);
}

// Verificar si se puede colocar un barco
bool canPlaceShip(const Board& board, const Ship& ship) {
    int ship_size = ship.get_size();
    
    cout << "\n🔍 DEBUG: Validando barco en posición (" << ship.x << "," << ship.y << ")" << endl;
    cout << "   - Tipo: " << ship.type << ", Tamaño: " << ship_size << endl;
    cout << "   - Orientación: " << (ship.orientation == HORIZONTAL ? "HORIZONTAL" : "VERTICAL") << endl;
    
    // Verificar límites del tablero
    if (ship.orientation == HORIZONTAL) {
        cout << "   - Verificando límites horizontales: y + size = " << ship.y << " + " << ship_size << " = " << (ship.y + ship_size) << " <= " << board.size << endl;
        if (ship.x < 0 || ship.x >= board.size || 
            ship.y < 0 || ship.y + ship_size > board.size) {
            cout << "   ❌ Fuera de límites del tablero" << endl;
            return false;
        }
    } else { // VERTICAL
        cout << "   - Verificando límites verticales: x + size = " << ship.x << " + " << ship_size << " = " << (ship.x + ship_size) << " <= " << board.size << endl;
        if (ship.x < 0 || ship.x + ship_size > board.size || 
            ship.y < 0 || ship.y >= board.size) {
            cout << "   ❌ Fuera de límites del tablero" << endl;
            return false;
        }
    }
    
    cout << "   ✓ Dentro de límites del tablero" << endl;
    
    // Verificar que no haya barcos en esas posiciones
   
    vector<pair<int, int>> coordinates = getShipCoordinates(ship);
    cout << "   - Coordenadas a ocupar: ";
    for (const auto& coord : coordinates) {
        cout << "(" << coord.first << "," << coord.second << ") ";
    }
    cout << endl;
    
    for (const auto& coord : coordinates) {
        if (board.grid[coord.first][coord.second] != WATER) {
            cout << "   ❌ Ya hay barco en (" << coord.first << "," << coord.second << ")" << endl;
            return false;
        }
    }
    
    cout << "   ✓ No hay conflictos con otros barcos" << endl;
    
    // Verificar que no haya barcos adyacentes
    for (const auto& coord : coordinates) {
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int nx = coord.first + dx;
                int ny = coord.second + dy;
                if (isValidPosition(nx, ny, board.size) && 
                    board.grid[nx][ny] == SHIP) {
                    cout << "   ❌ Barco adyacente en (" << nx << "," << ny << ")" << endl;
                    return false;
                }
            }
        }
    }
    
    cout << "   ✓ No hay barcos adyacentes - BARCO VÁLIDO" << endl;
    return true;
}

// Colocar barco en el tablero
bool placeShip(Board& board, Ship& ship) {
    if (!canPlaceShip(board, ship)) {
        return false;
    }
    
    vector<pair<int, int>> coordinates = getShipCoordinates(ship);
    for (const auto& coord : coordinates) {
        board.grid[coord.first][coord.second] = SHIP;
    }
    
    return true;
}

// Remover barco del tablero
bool removeShip(Board& board, const Ship& ship) {
    vector<pair<int, int>> coordinates = getShipCoordinates(ship);
    for (const auto& coord : coordinates) {
        if (isValidPosition(coord.first, coord.second, board.size)) {
            board.grid[coord.first][coord.second] = WATER;
        }
    }
    return true;
}

// Obtener coordenadas que ocupa un barco
// ex: Barco en (1,2) horizontal de tamaño 3 → [(1,2), (1,3), (1,4)]
vector<pair<int, int>> getShipCoordinates(const Ship& ship) {
    vector<pair<int, int>> coordinates;
    int ship_size = ship.get_size();
    
    for (int i = 0; i < ship_size; i++) {
        if (ship.orientation == HORIZONTAL) {
            coordinates.push_back({ship.x, ship.y + i});
        } else { // VERTICAL
            coordinates.push_back({ship.x + i, ship.y});
        }
    }
    
    return coordinates;
}

// Verificar si un disparo impacta un barco específico
bool checkShipHit(Ship& ship, int x, int y) {
    vector<pair<int, int>> coordinates = getShipCoordinates(ship);
    
    for (const auto& coord : coordinates) {
        if (coord.first == x && coord.second == y) {
            ship.hits++;
            return true;
        }
    }
    
    return false;
}