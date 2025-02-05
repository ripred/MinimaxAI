#ifndef MINIMAX_AI_H
#define MINIMAX_AI_H

#include "GameInterface.h"

// The MinimaxAI class encapsulates the minimax search with alpha-beta pruning.
class MinimaxAI {
public:
    // Constructor takes a reference to a GameInterface instance and the maximum search depth.
    MinimaxAI(GameInterface &gameRef, uint8_t depth);

    // Finds and returns the best move for the current game state.
    Move findBestMove();

private:
    // The recursive minimax function with alpha-beta pruning.
    int minimaxRecursive(uint8_t depth, int alpha, int beta, bool maximizing);

    GameInterface *game;   // Pointer to the game object
    uint8_t maxDepth;      // Maximum search depth
    Move bestMove;         // Best move found during search
};

#endif // MINIMAX_AI_H
