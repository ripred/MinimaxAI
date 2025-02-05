#ifndef CHECKERS_GAME_H
#define CHECKERS_GAME_H

#include "GameInterface.h"
#include <Arduino.h>
#include <stdint.h>
#include <string.h>

/// Fixed search depth (adjustable via compile–time flag).
constexpr int OPTION_DEPTH = 4;

/// Piece types.
enum CheckersPieceType {
    CP_EMPTY = 0,
    CP_MAN   = 1,
    CP_KING  = 2
};

/// Sides.
enum CheckersSide {
    SIDE_HUMAN = 0,
    SIDE_AI    = 1
};

/// A checkers piece stored in 1 byte via bitfields.
struct CheckerPiece {
    union {
        uint8_t raw;
        struct {
            uint8_t type   : 2; // 0 = empty, 1 = man, 2 = king
            uint8_t side   : 1; // 0 = HUMAN, 1 = AI
            uint8_t unused : 5;
        } bits;
    };
};

/// We use a 32–square board (only the playable dark squares).
#define NUM_SQUARES 32

/// Information to undo a move.
struct MoveUndo {
    bool wasJump;
    uint8_t capturedIndex;    // index of captured piece (if jump)
    uint8_t capturedPieceRaw; // raw value of captured piece
    bool promotionOccurred;
    uint8_t previousPieceRaw; // piece value before promotion
    bool turnSwitched;        // whether the turn was switched after the move
};

/// CheckersGame implements GameInterface for standard American checkers.
class CheckersGame : public GameInterface {
public:
    CheckerPiece board[NUM_SQUARES];  // 32 playable squares.
    uint8_t currentSide;              // SIDE_AI or SIDE_HUMAN indicating whose turn it is.
    MoveUndo undoStack[64];           // Fixed–size undo stack for minimax recursion.
    uint8_t undoStackIndex;           // Stack pointer.
    
    // Store last move to help avoid immediate reversal moves.
    Move lastMove;
    bool lastMoveValid;
    
    // --- New: Board history for repetition detection ---
    uint16_t boardHistory[32]; // Simple history (one hash per applied move)
    uint8_t historySize;       // Number of entries in boardHistory
    
    CheckersGame();
    
    // Evaluate the board: a positive score favors AI, negative favors Human.
    int evaluateBoard() override;
    
    // Generate legal moves for the current position.
    // If any capture moves exist, only those are generated (forced capture).
    uint8_t generateMoves(Move *moves) override;
    
    // Apply a move (updates board, handles captures, king promotion, and forced multi–jumps).
    void applyMove(const Move &m) override;
    
    // Undo the move, restoring captured pieces and previous turn if needed.
    void undoMove(const Move &m) override;
    
    // Returns true if the game is over (no legal moves or one side has no pieces).
    bool isGameOver() override;
    
    // Returns +1 if it is AI's turn, -1 if Human's.
    int currentPlayer() override;
    
    // Reset the game to the initial checkers position.
    void reset_game();
    
    // Print the board to Serial.
    void printBoard();
    
    // Helper: Check if any capture moves exist for the current side.
    bool hasCaptureMoves();
    
    // Helper: Generate legal moves for a single piece at index.
    // If forceCapture is true, only capture moves are generated.
    uint8_t generatePieceMoves(uint8_t index, Move *moves, bool forceCapture);
    
    // Helper: Convert a board index (0–31) into row and column (0–7) for display.
    void indexToCoord(uint8_t index, uint8_t &row, uint8_t &col);
    
    // Helper: Check whether (row, col) is within board bounds.
    bool isValidCoord(int row, int col);
    
    // Helper: Convert row and col into board index; returns 255 if not a playable square.
    uint8_t coordToIndex(uint8_t row, uint8_t col);
    
    // Helper: Check if additional capture moves are available for a piece at index.
    bool additionalCaptureAvailable(uint8_t index);
    
private:
    // Compute a simple hash of the board state.
    uint16_t computeBoardHash();
};

#endif // CHECKERS_GAME_H
