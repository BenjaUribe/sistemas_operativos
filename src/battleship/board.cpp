#include "battleship.h"

// Constructor del tablero
Board::Board() : ships_placed(false) {
    grid = vector<vector<int>>(BOARD_SIZE, vector<int>(BOARD_SIZE, WATER));
}

// Inicializar tablero
void initializeBoard(Board& board) {
    board.grid = vector<vector<int>>(BOARD_SIZE, vector<int>(BOARD_SIZE, WATER));
    board.ships_placed = false;
}

// Mostrar tablero
void displayBoard(const Board& board, bool hide_ships) {
    cout << "\n  ";
    for (int i = 0; i < BOARD_SIZE; i++) {
        cout << " " << i + 1 << " ";
    }
    cout << "\n";
    
    for (int i = 0; i < BOARD_SIZE; i++) {
        cout << char('A' + i) << " ";
        for (int j = 0; j < BOARD_SIZE; j++) {
            char symbol;
            switch (board.grid[i][j]) {
                case WATER:
                    symbol = '~';
                    break;
                case SHIP:
                    symbol = hide_ships ? '~' : 'S';
                    break;
                case HIT:
                    symbol = 'X';
                    break;
                case MISS:
                    symbol = 'O';
                    break;
                default:
                    symbol = '?';
            }
            cout << " " << symbol << " ";
        }
        cout << "\n";
    }
    cout << "\n";
}

// Mostrar ambos tableros del jugador
void displayBothBoards(const Player& player) {
    cout << "\n=== TU TABLERO ===";
    displayBoard(player.own_board, false);
    
    cout << "=== TABLERO ENEMIGO ===";
    displayBoard(player.opponent_board, true);
}

// Validar posición
bool isValidPosition(int x, int y) {
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

// Actualizar celda del tablero
void updateCell(Board& board, int x, int y, CellState state) {
    if (isValidPosition(x, y)) {
        board.grid[x][y] = state;
    }
}