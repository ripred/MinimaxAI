#include "TicTacToeGame.h"
#include <string.h>
#include <avr/pgmspace.h>

// Declare the optimal moves array as a file-scope constant in PROGMEM.
// These positions are the center and the four corners.
static const uint8_t optimalMoves[5] PROGMEM = {0, 2, 4, 6, 8};

TicTacToeGame::TicTacToeGame() {
    reset_game();
}

int TicTacToeGame::evaluateBoard() {
    // Terminal states have highest priority.
    if (isWinner(1)) return +10;  // AI wins (mark 1)
    if (isWinner(2)) return -10;  // Human wins (mark 2)
    
    // For non-terminal states, use a heuristic evaluation.
    int score = 0;
    
    // Lambda function to evaluate one line given indices.
    auto evaluateLine = [this](uint8_t idx1, uint8_t idx2, uint8_t idx3) -> int {
        int scoreLine = 0;
        int countAI = 0, countHuman = 0;
        uint8_t cells[3] = { board[idx1], board[idx2], board[idx3] };
        for (int i = 0; i < 3; i++) {
            if (cells[i] == 1)
                countAI++;
            else if (cells[i] == 2)
                countHuman++;
        }
        // Only consider lines that are not contested.
        if (countAI > 0 && countHuman == 0) {
            if (countAI == 2)
                scoreLine = 5;   // AI nearly wins.
            else if (countAI == 1)
                scoreLine = 1;
        }
        else if (countHuman > 0 && countAI == 0) {
            if (countHuman == 2)
                scoreLine = -5;  // Human nearly wins => must block.
            else if (countHuman == 1)
                scoreLine = -1;
        }
        return scoreLine;
    };

    // Evaluate rows.
    score += evaluateLine(0, 1, 2);
    score += evaluateLine(3, 4, 5);
    score += evaluateLine(6, 7, 8);
    // Evaluate columns.
    score += evaluateLine(0, 3, 6);
    score += evaluateLine(1, 4, 7);
    score += evaluateLine(2, 5, 8);
    // Evaluate diagonals.
    score += evaluateLine(0, 4, 8);
    score += evaluateLine(2, 4, 6);
    
    return score;
}

uint8_t TicTacToeGame::generateMoves(Move *moves) {
    uint8_t count = 0;
    for (uint8_t i = 0; i < 9; i++) {
        if (board[i] == 0) {
            moves[count].from = i;  // Not used, but set for clarity.
            moves[count].to = i;
            count++;
        }
    }
    return count;
}

void TicTacToeGame::applyMove(const Move &m) {
    uint8_t pos = m.to;
    // Ensure pos is within bounds.
    if (pos < 9) {
        board[pos] = (current == AI ? 1 : 2);
    }
    // Switch turn.
    current = (current == AI ? HUMAN : AI);
}

void TicTacToeGame::undoMove(const Move &m) {
    uint8_t pos = m.to;
    if (pos < 9) {
        board[pos] = 0;
    }
    // Switch turn back.
    current = (current == AI ? HUMAN : AI);
}

bool TicTacToeGame::isGameOver() {
    return isWinner(1) || isWinner(2) || isBoardFull();
}

int TicTacToeGame::currentPlayer() {
    return (current == AI ? 1 : -1);
}

uint32_t get_random_seed() {
    uint32_t seed = 0;
    int pins[] = { A0, A1, A2, A3 };
    for (int i=0; i < 32; i++) {
        uint32_t value = 0;
        for (auto pin :pins) {
            value += analogRead(pin);
        }
        seed = (seed << 31) | (!(value & 0b0100) ? true : false);
    }

    return seed;
}

void TicTacToeGame::reset_game() {
    randomSeed(get_random_seed());

    memset(board, 0, sizeof(board));
    current = AI;  // You may alternate starting players if desired.
}

bool TicTacToeGame::isWinner(uint8_t mark) {
    // Store winning combinations in PROGMEM to save RAM.
    const uint8_t wins[8][3] PROGMEM = {
        {0, 1, 2}, {3, 4, 5}, {6, 7, 8},   // Rows.
        {0, 3, 6}, {1, 4, 7}, {2, 5, 8},   // Columns.
        {0, 4, 8}, {2, 4, 6}              // Diagonals.
    };

    for (uint8_t i = 0; i < 8; i++) {
        uint8_t a = pgm_read_byte(&(wins[i][0]));
        uint8_t b = pgm_read_byte(&(wins[i][1]));
        uint8_t c = pgm_read_byte(&(wins[i][2]));
        if (board[a] == mark && board[b] == mark && board[c] == mark) {
            return true;
        }
    }
    return false;
}

bool TicTacToeGame::isBoardFull() {
    for (uint8_t i = 0; i < 9; i++) {
        if (board[i] == 0)
            return false;
    }
    return true;
}

bool TicTacToeGame::isBoardEmpty() {
    for (uint8_t i = 0; i < 9; i++) {
        if (board[i] != 0)
            return false;
    }
    return true;
}

bool TicTacToeGame::optimalOpeningMove(Move &move) {
    // For Tic-Tac-Toe, the optimal opening moves (with perfect play) are the center (4)
    // and the four corners (0, 2, 6, 8). With perfect play, any of these openings lead to a draw.
    if (isBoardEmpty()) {
        // Randomly choose one from the five optimal moves stored in PROGMEM.
        uint8_t idx = random(5);
        uint8_t chosen = pgm_read_byte(&optimalMoves[idx]);
        move.from = chosen;
        move.to = chosen;
        return true;
    }
    return false;
}
