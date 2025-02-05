#ifndef TIC_TAC_TOE_GAME_H
#define TIC_TAC_TOE_GAME_H

#include "GameInterface.h"

// Simple enumeration for the two players.
enum Player { HUMAN = 0, AI = 1 };

// TicTacToeGame implements GameInterface for a standard 3x3 Tic-Tac-Toe game.
class TicTacToeGame : public GameInterface {
public:
    uint8_t board[9];  // 3x3 board stored in a 1D array (0 = empty, 1 = X, 2 = O)
    Player current;    // Indicates whose turn it is

    TicTacToeGame();

    // Evaluate board: +10 if AI wins, -10 if Human wins,
    // or a heuristic score based on potential wins/losses.
    int evaluateBoard() override;

    // Generate all possible moves (all empty cells).
    uint8_t generateMoves(Move *moves) override;

    // Apply the move: place the mark and switch turns.
    void applyMove(const Move &m) override;

    // Undo the move: remove the mark and revert the turn.
    void undoMove(const Move &m) override;

    // Return true if the game has ended (win or draw).
    bool isGameOver() override;

    // Return +1 if it's AI's turn (maximizing), -1 if Human's turn.
    int currentPlayer() override;

    // Reset the game board and state for a new game.
    void reset_game();

    // Expose isWinner publicly so that it can be checked externally.
    bool isWinner(uint8_t mark);

    // Check if the board is completely empty.
    bool isBoardEmpty();

    // Override the optimalOpeningMove function: return center if board is empty.
    bool optimalOpeningMove(Move &move) override;

private:
    bool isBoardFull();
};

#endif // TIC_TAC_TOE_GAME_H

