#ifndef GAME_INTERFACE_H
#define GAME_INTERFACE_H

#include <Arduino.h>
#include <stdint.h>
#include <string.h>

// Adjust MAX_MOVES as needed.
#define MAX_MOVES 9

// A simple structure to represent a move.
struct Move {
    uint8_t from;
    uint8_t to;
};

// Abstract interface that each game must implement.
class GameInterface {
public:
    // Return an evaluation score for the current board state.
    virtual int evaluateBoard() = 0;
    
    // Populate the moves array with legal moves from the current state.
    // Return the number of moves generated.
    virtual uint8_t generateMoves(Move *moves) = 0;
    
    // Apply the move to the game state.
    virtual void applyMove(const Move &m) = 0;
    
    // Undo the move, restoring the previous state.
    virtual void undoMove(const Move &m) = 0;
    
    // Return true if the game is in a terminal state.
    virtual bool isGameOver() = 0;
    
    // Return +1 if it's the maximizing player's turn, -1 otherwise.
    virtual int currentPlayer() = 0;
    
    // Optional: If the game supports an optimal opening move, override this method.
    // The default implementation does nothing.
    virtual bool optimalOpeningMove(Move & /*move*/) { return false; }
};

#endif // GAME_INTERFACE_H
