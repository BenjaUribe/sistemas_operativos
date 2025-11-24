#include "battleship.h"

// Constructor del tablero por defecto (8x8)
Board::Board() : ships_placed(false), size(BOARD_SIZE_1VS1) {
    grid = vector<vector<int>>(size, vector<int>(size, WATER));
}

// Constructor del tablero con tamaño específico
Board::Board(int board_size) : ships_placed(false), size(board_size) {
    grid = vector<vector<int>>(size, vector<int>(size, WATER));
}

// Función utilitaria para obtener tamaño según modo
int getBoardSizeForMode(GameMode mode) {
    return (mode == MODE_1VS1) ? BOARD_SIZE_1VS1 : BOARD_SIZE_2VS2;
}

// Inicializar tablero
void initializeBoard(Board& board) {
    board.grid = vector<vector<int>>(board.size, vector<int>(board.size, WATER));
    board.ships_placed = false;
}

// Mostrar tablero
void displayBoard(const Board& board, bool hide_ships) {
    cout << "\n  ";
    for (int i = 0; i < board.size; i++) {
        cout << " " << i + 1 << " ";
    }
    cout << "\n";
    
    for (int i = 0; i < board.size; i++) {
        cout << char('A' + i) << " ";
        for (int j = 0; j < board.size; j++) {
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
bool isValidPosition(int x, int y, int board_size) {
    return x >= 0 && x < board_size && y >= 0 && y < board_size;
}

// Actualizar celda del tablero
void updateCell(Board& board, int x, int y, CellState state) {
    if (isValidPosition(x, y, board.size)) {
        board.grid[x][y] = state;
    }
}