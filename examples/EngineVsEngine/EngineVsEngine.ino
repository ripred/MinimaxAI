#include <Arduino.h>
#include "TicTacToeGame.h"
#include "MinimaxAI.h"

TicTacToeGame game;
MinimaxAI ai(game, 9);  // Use full-depth search for Tic-Tac-Toe

// Statistics counters.
uint16_t winsPlayer1 = 0;
uint16_t winsPlayer2 = 0;
uint16_t draws = 0;
uint16_t totalGames = 0;

void printBoard() {
  const char symbols[3] = { ' ', 'X', 'O' };
  Serial.println(F("Board:"));
  for (int i = 0; i < 9; i++) {
    Serial.print(symbols[game.board[i]]);
    if ((i % 3) == 2)
      Serial.println();
    else
      Serial.print(F("|"));
  }
  Serial.println(F("-----"));
}

void printStats() {
  Serial.println(F("Game Over!"));
  Serial.print(F("Player 1 (X) wins: "));
  Serial.println(winsPlayer1);
  Serial.print(F("Player 2 (O) wins: "));
  Serial.println(winsPlayer2);
  Serial.print(F("Draws: "));
  Serial.println(draws);
  Serial.print(F("Total Games: "));
  Serial.println(totalGames);
  Serial.println(F("Starting a new game..."));
}

void setup() {
  Serial.begin(115200);
  while (!Serial);
  randomSeed(analogRead(0));
  
  Serial.println(F("Engine vs Engine Tic-Tac-Toe:"));
  game.reset_game();
  printBoard();
}

void loop() {
  if (game.isGameOver()) {
    totalGames++;
    if (game.isWinner(1))
      winsPlayer1++;
    else if (game.isWinner(2))
      winsPlayer2++;
    else
      draws++;
      
    printStats();
    game.reset_game();
//    delay(1000);
    delay(500);
    printBoard();
  } else {
    Move move = ai.findBestMove();
    Serial.print(F("Engine plays at position "));
    Serial.println(move.to + 1);
    game.applyMove(move);
    printBoard();
//    delay(500);
  }
}
