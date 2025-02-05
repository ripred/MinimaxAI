#include <Arduino.h>
#include "CheckersGame.h"
#include "MinimaxAI.h"

// Configure both players as COMPUTER.
enum PlayerType { HUMAN, COMPUTER };
PlayerType player_ai_type = COMPUTER;
PlayerType player_human_type = COMPUTER;

CheckersGame game;
MinimaxAI ai(game, OPTION_DEPTH);

// Reads human move input from Serial (if needed).
bool readHumanMove(Move &move) {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    int fromNum, toNum;
    if (sscanf(input.c_str(), "%d %d", &fromNum, &toNum) == 2) {
      // Convert from 1–32 to 0–31 indexing.
      move.from = fromNum - 1;
      move.to = toNum - 1;
      return true;
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { /* Wait for Serial */ }
  
  Serial.println(F("Checkers AI Game"));
  Serial.println(F("Enter moves as: <from> <to> (numbers 1-32)"));
  game.reset_game();
  game.printBoard();
}

void loop() {
  if (game.isGameOver()) {
    int score = game.evaluateBoard();
    if (score > 0) Serial.println(F("AI wins!"));
    else if (score < 0) Serial.println(F("Human wins!"));
    else Serial.println(F("Draw!"));
    while (true) { delay(1000); }
  }
  
  // Determine whose turn it is.
  bool humanTurn = false;
  if (game.currentSide == SIDE_AI)
    humanTurn = (player_ai_type == HUMAN);
  else
    humanTurn = (player_human_type == HUMAN);
  
  Move moves[32];
  uint8_t moveCount = game.generateMoves(moves);
  if (moveCount == 0) return;  // Should not occur if game is not over.
  
  if (humanTurn) {
    if (Serial.available() > 0) {
      Move humanMove;
      if (readHumanMove(humanMove)) {
        bool legal = false;
        for (uint8_t i = 0; i < moveCount; i++) {
          if (moves[i].from == humanMove.from && moves[i].to == humanMove.to) {
            legal = true;
            break;
          }
        }
        if (legal) {
          game.applyMove(humanMove);
          game.printBoard();
        } else {
          Serial.println(F("Illegal move. Try again."));
        }
      }
    }
  } else {
    Move aiMove = ai.findBestMove();
    Serial.print(F("AI plays move from "));
    Serial.print(aiMove.from + 1);
    Serial.print(F(" to "));
    Serial.println(aiMove.to + 1);
    game.applyMove(aiMove);
    game.printBoard();
    delay(500);
  }
}
