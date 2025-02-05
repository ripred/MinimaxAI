#include <Arduino.h>
#include "TicTacToeGame.h"
#include "MinimaxAI.h"

TicTacToeGame game;
MinimaxAI ai(game, 9);  // Use full-depth search for Tic-Tac-Toe

// Helper function to print the board state to Serial.
void printBoard() {
  const char symbols[3] = { ' ', 'X', 'O' };
  Serial.println(F("Board:"));
  for (int i = 0; i < 9; i++) {
    Serial.print(symbols[ game.board[i] ]);
    if ((i % 3) == 2)
      Serial.println();
    else
      Serial.print(F("|"));
  }
  Serial.println(F("-----"));
}

void setup() {
  Serial.begin(115200);  // Use 115200 baud
  while (!Serial);       // Wait for Serial Monitor to open (if needed)
  
  Serial.println(F("Tic-Tac-Toe AI: Human vs Engine"));
  game.reset_game();
  printBoard();
  
  Serial.println(F("Enter your move (1-9):"));
}

void loop() {
  // If game is over, print the result and halt.
  if (game.isGameOver()) {
    if (game.isWinner(1))
      Serial.println(F("AI wins!"));
    else if (game.isWinner(2))
      Serial.println(F("You win!"));
    else
      Serial.println(F("It's a draw."));
      
    // Halt execution.
    while (true) { delay(1000); }
  }
  
  // Human turn: wait for Serial input.
  if (game.current == HUMAN) {
    if (Serial.available() > 0) {
      char c = Serial.read();
      if (c >= '1' && c <= '9') {
        uint8_t pos = c - '1';  // Convert char '1'-'9' to board index 0-8.
        if (game.board[pos] == 0) {
          Move m = { pos, pos };
          game.applyMove(m);
          printBoard();
        } else {
          Serial.println(F("Cell occupied, try again."));
        }
      }
    }
  } 
  // Engine turn.
  else {
    Move m = ai.findBestMove();
    Serial.print(F("Engine plays at position "));
    Serial.println(m.to + 1);
    game.applyMove(m);
    printBoard();
    delay(500);  // Brief delay for readability.
  }
}

