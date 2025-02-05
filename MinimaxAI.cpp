#include "MinimaxAI.h"

MinimaxAI::MinimaxAI(GameInterface &gameRef, uint8_t depth)
    : game(&gameRef), maxDepth(depth) {
}

Move MinimaxAI::findBestMove() {
    Move optMove;
    // If the game provides an optimal opening move, use it.
    if (game->optimalOpeningMove(optMove)) {
        return optMove;
    }

    bool maximizing = (game->currentPlayer() > 0);
    int alpha = -32767;  // Represents -∞.
    int beta  =  32767;  // Represents +∞.

    int bestVal = (maximizing ? -32767 : 32767);
    Move bestMoveCandidate = {0, 0};

    Move moves[MAX_MOVES];
    uint8_t moveCount = game->generateMoves(moves);

    for (uint8_t i = 0; i < moveCount; i++) {
        game->applyMove(moves[i]);
        int eval = minimaxRecursive(maxDepth - 1, alpha, beta, !maximizing);
        game->undoMove(moves[i]);

        if (maximizing) {
            if (eval > bestVal) {
                bestVal = eval;
                bestMoveCandidate = moves[i];
            }
            if (eval > alpha) {
                alpha = eval;
            }
        } else { // Minimizing.
            if (eval < bestVal) {
                bestVal = eval;
                bestMoveCandidate = moves[i];
            }
            if (eval < beta) {
                beta = eval;
            }
        }
        if (alpha >= beta) {
            break;  // Alpha-beta cutoff.
        }
    }
    return bestMoveCandidate;
}

int MinimaxAI::minimaxRecursive(uint8_t depth, int alpha, int beta, bool maximizing) {
    if (depth == 0 || game->isGameOver()) {
        return game->evaluateBoard();
    }

    Move moves[MAX_MOVES];
    uint8_t moveCount = game->generateMoves(moves);

    if (maximizing) {
        int maxScore = -32767;
        for (uint8_t i = 0; i < moveCount; i++) {
            game->applyMove(moves[i]);
            int score = minimaxRecursive(depth - 1, alpha, beta, false);
            game->undoMove(moves[i]);

            if (score > maxScore) {
                maxScore = score;
            }
            if (score > alpha) {
                alpha = score;
            }
            if (alpha >= beta) {
                break;
            }
        }
        return maxScore;
    } else {
        int minScore = 32767;
        for (uint8_t i = 0; i < moveCount; i++) {
            game->applyMove(moves[i]);
            int score = minimaxRecursive(depth - 1, alpha, beta, true);
            game->undoMove(moves[i]);

            if (score < minScore) {
                minScore = score;
            }
            if (score < beta) {
                beta = score;
            }
            if (alpha >= beta) {
                break;
            }
        }
        return minScore;
    }
}
