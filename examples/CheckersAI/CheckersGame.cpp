#include "CheckersGame.h"

// Constructor.
CheckersGame::CheckersGame() {
    reset_game();
    undoStackIndex = 0;
    lastMoveValid = false;
    historySize = 0;
}

// Compute a simple hash for the board by folding in each square’s raw value.
uint16_t CheckersGame::computeBoardHash() {
    uint16_t hash = 0;
    for (uint8_t i = 0; i < NUM_SQUARES; i++) {
        // A simple multiplicative hash (you can adjust the constant)
        hash = (hash * 33) ^ board[i].raw;
    }
    return hash;
}

// Reset board to initial checkers position.
// We use the 32 playable squares arranged by rows:
// Rows 0-2 (indices 0–11): AI pieces (men)
// Rows 3-4 (indices 12–19): empty
// Rows 5-7 (indices 20–31): Human pieces (men)
void CheckersGame::reset_game() {
    for (uint8_t i = 0; i < NUM_SQUARES; i++) {
        board[i].raw = 0;
    }
    // Place AI pieces (indices 0–11)
    for (uint8_t i = 0; i < 12; i++) {
        board[i].bits.type = CP_MAN;
        board[i].bits.side = SIDE_AI;
    }
    // Indices 12 to 19 remain empty.
    for (uint8_t i = 12; i < 20; i++) {
        board[i].raw = 0;
    }
    // Place Human pieces (indices 20–31)
    for (uint8_t i = 20; i < NUM_SQUARES; i++) {
        board[i].bits.type = CP_MAN;
        board[i].bits.side = SIDE_HUMAN;
    }
    // Let AI move first.
    currentSide = SIDE_AI;
    undoStackIndex = 0;
    lastMoveValid = false;
    historySize = 0;
    boardHistory[historySize++] = computeBoardHash();
}

// Enhanced evaluation function: adds a bonus for advancing pieces
// and subtracts a penalty if the current board state is repeated.
int CheckersGame::evaluateBoard() {
    int score = 0;
    for (uint8_t i = 0; i < NUM_SQUARES; i++) {
        if (board[i].bits.type == CP_MAN) {
            int bonus = 0;
            uint8_t row, col;
            indexToCoord(i, row, col);
            // For AI pieces (which move down), reward higher row numbers.
            if (board[i].bits.side == SIDE_AI)
                bonus = row;
            else  // For Human pieces (which move up), reward lower row numbers.
                bonus = (7 - row);
            if (board[i].bits.side == SIDE_AI)
                score += 3 + bonus;
            else
                score -= 3 + bonus;
        } else if (board[i].bits.type == CP_KING) {
            if (board[i].bits.side == SIDE_AI) score += 5;
            else score -= 5;
        }
    }
    // Repetition penalty: if the current board hash appears more than once in the history,
    // subtract a penalty to discourage cycles.
    uint16_t currentHash = computeBoardHash();
    uint8_t repetitions = 0;
    for (uint8_t i = 0; i < historySize; i++) {
        if (boardHistory[i] == currentHash)
            repetitions++;
    }
    if (repetitions > 1) {
        // Subtract 10 points for each repeated occurrence beyond the first.
        score -= 10 * (repetitions - 1);
    }
    return score;
}

// Check if (row, col) is within 0–7.
bool CheckersGame::isValidCoord(int row, int col) {
    return (row >= 0 && row < 8 && col >= 0 && col < 8);
}

// Convert board index (0–31) to row and column.
// In our mapping, even rows (0,2,4,6) have playable squares in columns 1,3,5,7,
// and odd rows (1,3,5,7) in columns 0,2,4,6.
void CheckersGame::indexToCoord(uint8_t index, uint8_t &row, uint8_t &col) {
    row = index / 4;
    uint8_t posInRow = index % 4;
    if (row % 2 == 0)
        col = posInRow * 2 + 1;
    else
        col = posInRow * 2;
}

// Convert (row, col) to board index; return 255 if not a playable square.
uint8_t CheckersGame::coordToIndex(uint8_t row, uint8_t col) {
    if (!isValidCoord(row, col)) return 255;
    if ((row + col) % 2 == 0) return 255; // not a dark (playable) square.
    if (row % 2 == 0) {
        if (col < 1 || col > 7) return 255;
        return row * 4 + ((col - 1) / 2);
    } else {
        return row * 4 + (col / 2);
    }
}

// Generate legal moves for a single piece at board index.
// If forceCapture is true, only capture moves are generated.
uint8_t CheckersGame::generatePieceMoves(uint8_t index, Move *moves, bool forceCapture) {
    uint8_t count = 0;
    uint8_t row, col;
    indexToCoord(index, row, col);
    CheckerPiece piece = board[index];
    if (piece.bits.type == CP_EMPTY) return 0;

    int dr[4], dc[4];
    uint8_t numDirs = 0;
    if (piece.bits.type == CP_MAN) {
        if (piece.bits.side == SIDE_AI) {
            dr[0] = 1; dc[0] = -1;
            dr[1] = 1; dc[1] = 1;
            numDirs = 2;
        } else {
            dr[0] = -1; dc[0] = -1;
            dr[1] = -1; dc[1] = 1;
            numDirs = 2;
        }
    } else if (piece.bits.type == CP_KING) {
        dr[0] = 1;  dc[0] = -1;
        dr[1] = 1;  dc[1] = 1;
        dr[2] = -1; dc[2] = -1;
        dr[3] = -1; dc[3] = 1;
        numDirs = 4;
    }
    // First generate capture moves.
    for (uint8_t i = 0; i < numDirs; i++) {
        int midRow = row + dr[i];
        int midCol = col + dc[i];
        int destRow = row + 2 * dr[i];
        int destCol = col + 2 * dc[i];
        if (!isValidCoord(midRow, midCol) || !isValidCoord(destRow, destCol))
            continue;
        uint8_t midIndex = coordToIndex(midRow, midCol);
        uint8_t destIndex = coordToIndex(destRow, destCol);
        if (midIndex == 255 || destIndex == 255)
            continue;
        if (board[midIndex].bits.type != CP_EMPTY &&
            board[midIndex].bits.side != piece.bits.side &&
            board[destIndex].bits.type == CP_EMPTY) {
            moves[count].from = index;
            moves[count].to = destIndex;
            count++;
        }
    }
    if (forceCapture) return count;
    // If no capture moves, generate simple moves.
    if (count == 0) {
        for (uint8_t i = 0; i < numDirs; i++) {
            int destRow = row + dr[i];
            int destCol = col + dc[i];
            if (!isValidCoord(destRow, destCol)) continue;
            uint8_t destIndex = coordToIndex(destRow, destCol);
            if (destIndex == 255) continue;
            if (board[destIndex].bits.type == CP_EMPTY) {
                moves[count].from = index;
                moves[count].to = destIndex;
                count++;
            }
        }
    }
    return count;
}

// Check whether any capture moves exist for the current side.
bool CheckersGame::hasCaptureMoves() {
    Move tempMoves[12];
    for (uint8_t i = 0; i < NUM_SQUARES; i++) {
        if (board[i].bits.type != CP_EMPTY && board[i].bits.side == currentSide) {
            if (generatePieceMoves(i, tempMoves, true) > 0)
                return true;
        }
    }
    return false;
}

// Generate all legal moves from the current game state.
uint8_t CheckersGame::generateMoves(Move *moves) {
    uint8_t count = 0;
    bool forceCapture = hasCaptureMoves();
    for (uint8_t i = 0; i < NUM_SQUARES; i++) {
        if (board[i].bits.type != CP_EMPTY && board[i].bits.side == currentSide) {
            count += generatePieceMoves(i, &moves[count], forceCapture);
        }
    }
    // Filter out moves that are immediate reversals of the last move, if alternatives exist.
    if (lastMoveValid && count > 1) {
        uint8_t nonReversalCount = 0;
        Move nonReversalMoves[32];
        for (uint8_t i = 0; i < count; i++) {
            if (moves[i].from == lastMove.to && moves[i].to == lastMove.from) {
                // Skip this reversal move.
            } else {
                nonReversalMoves[nonReversalCount++] = moves[i];
            }
        }
        if (nonReversalCount > 0) {
            memcpy(moves, nonReversalMoves, nonReversalCount * sizeof(Move));
            count = nonReversalCount;
        }
    }
    return count;
}

// Check if additional capture moves are available for the piece at index.
bool CheckersGame::additionalCaptureAvailable(uint8_t index) {
    Move temp[8];
    return (generatePieceMoves(index, temp, true) > 0);
}

// Apply a move: move the piece, remove any captured piece,
// handle king promotion, and if a jump was made and further jumps exist, do not switch turn.
void CheckersGame::applyMove(const Move &m) {
    MoveUndo undo;
    memset(&undo, 0, sizeof(undo));
    uint8_t fromRow, fromCol, toRow, toCol;
    indexToCoord(m.from, fromRow, fromCol);
    indexToCoord(m.to, toRow, toCol);
    CheckerPiece movingPiece = board[m.from];
    bool isJump = (abs((int)fromRow - (int)toRow) == 2);
    undo.wasJump = isJump;
    if (isJump) {
        uint8_t capRow = (fromRow + toRow) / 2;
        uint8_t capCol = (fromCol + toCol) / 2;
        uint8_t capIndex = coordToIndex(capRow, capCol);
        undo.capturedIndex = capIndex;
        undo.capturedPieceRaw = board[capIndex].raw;
        board[capIndex].raw = 0;
    }
    // Move the piece.
    board[m.to] = movingPiece;
    board[m.from].raw = 0;
    // Check for promotion.
    uint8_t destRow, destCol;
    indexToCoord(m.to, destRow, destCol);
    if (movingPiece.bits.type == CP_MAN) {
        if ((movingPiece.bits.side == SIDE_AI && destRow == 7) ||
            (movingPiece.bits.side == SIDE_HUMAN && destRow == 0)) {
            undo.promotionOccurred = true;
            undo.previousPieceRaw = board[m.to].raw;
            board[m.to].bits.type = CP_KING;
        }
    }
    // Decide whether to switch turn.
    bool switchTurn = true;
    if (isJump && additionalCaptureAvailable(m.to))
        switchTurn = false;
    undo.turnSwitched = switchTurn;
    if (switchTurn) {
        currentSide = (currentSide == SIDE_AI ? SIDE_HUMAN : SIDE_AI);
        // Update lastMove only when the turn switches.
        lastMove = m;
        lastMoveValid = true;
    }
    // Push undo info.
    undoStack[undoStackIndex++] = undo;
    // Update board history: record the new board hash.
    if (historySize < sizeof(boardHistory) / sizeof(boardHistory[0])) {
        boardHistory[historySize++] = computeBoardHash();
    }
}

// Undo a move: reverse piece movement, restore captured piece (if any),
// revert promotion if occurred, and restore the turn.
void CheckersGame::undoMove(const Move &m) {
    if (undoStackIndex == 0) return;
    MoveUndo undo = undoStack[--undoStackIndex];
    board[m.from] = board[m.to];
    board[m.to].raw = 0;
    if (undo.promotionOccurred) {
        board[m.from].raw = undo.previousPieceRaw;
    }
    if (undo.wasJump) {
        board[undo.capturedIndex].raw = undo.capturedPieceRaw;
    }
    if (undo.turnSwitched) {
        currentSide = (currentSide == SIDE_AI ? SIDE_HUMAN : SIDE_AI);
    }
    // Remove the last board hash from the history.
    if (historySize > 0)
        historySize--;
}

// Game is over if there are no legal moves or one side has no pieces.
bool CheckersGame::isGameOver() {
    Move temp[32];
    uint8_t count = generateMoves(temp);
    bool hasAI = false, hasHuman = false;
    for (uint8_t i = 0; i < NUM_SQUARES; i++) {
        if (board[i].bits.type != CP_EMPTY) {
            if (board[i].bits.side == SIDE_AI) hasAI = true;
            else hasHuman = true;
        }
    }
    return (count == 0 || !hasAI || !hasHuman);
}

// Return +1 if AI's turn, -1 if Human's.
int CheckersGame::currentPlayer() {
    return (currentSide == SIDE_AI ? 1 : -1);
}

// Print an 8x8 representation of the board to Serial.
// Playable squares show a symbol representing the piece:
//   For AI: 'X' for man, 'K' for king.
//   For Human: 'O' for man, 'Q' for king.
// Non-playable squares are shown as a blank.
void CheckersGame::printBoard() {
    char disp[8][9];
    for (uint8_t r = 0; r < 8; r++) {
        for (uint8_t c = 0; c < 8; c++) {
            disp[r][c] = ' ';
        }
        disp[r][8] = '\0';
    }
    for (uint8_t i = 0; i < NUM_SQUARES; i++) {
        uint8_t row, col;
        indexToCoord(i, row, col);
        char symbol = '.';
        if (board[i].bits.type != CP_EMPTY) {
            if (board[i].bits.side == SIDE_AI)
                symbol = (board[i].bits.type == CP_MAN) ? 'X' : 'K';
            else
                symbol = (board[i].bits.type == CP_MAN) ? 'O' : 'Q';
        }
        disp[row][col] = symbol;
    }
    Serial.println(F("Board:"));
    for (uint8_t r = 0; r < 8; r++) {
        Serial.println(disp[r]);
    }
}
