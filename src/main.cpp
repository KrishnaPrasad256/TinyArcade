#include <Arduino.h>

#include "ArcadeTypes.h"
#include "DdrGame.h"
#include "Display.h"
#include "HighScore.h"
#include "Input.h"
#include "Menu.h"
#include "TetrisGame.h"

namespace {

ArcadeState arcadeState;

void printDebugState(const ArcadeState& state) {
  Serial.print("btn_state: ");
  Serial.println(static_cast<int>(state.buttonState));
  Serial.print("game_state: ");
  Serial.println(static_cast<int>(state.gameState));
  Serial.print("games: ");
  Serial.println(static_cast<int>(state.selectedGame));
  Serial.print("menu_btn: ");
  Serial.println(static_cast<int>(state.menuButton));
  Serial.println("=====================");
}

}  // namespace

void setup() {
  Serial.begin(115200);
  Serial.println("Initialising Tiny_Arcade!");

  arcadeState = ArcadeState{};

  Input::init();

  if (!Display::begin()) {
    Serial.println("SSD1306 NOT FOUND!");
    while (true) {
    }
  }

  if (!HighScore::begin()) {
    Serial.println("EEPROM init failed, highscores will not persist.");
  }

  Menu::showLogo();
  Display::fadeIn();
  delay(2000);
  Display::drawText(1, 15, 55, "Click any Button");
  Menu::waitForAnyButton(arcadeState);
  Display::fadeOut();
  Display::clearScreen();
  Display::resetContrast();
}

void loop() {
  printDebugState(arcadeState);

  switch (arcadeState.gameState) {
    case GameState::MENU:
      Menu::runMainMenu(arcadeState);
      break;

    case GameState::GAME_OVER:
      Menu::runGameOverMenu(arcadeState);
      break;

    case GameState::PLAYING:
      switch (arcadeState.selectedGame) {
        case GameSelection::TETRIS:
          TetrisGame::run(arcadeState);
          break;

        case GameSelection::DDR_GAME:
          DdrGame::run(arcadeState);
          break;
      }
      break;

    case GameState::GAMES_MENU:
      Menu::runGamesMenu(arcadeState);
      break;
  }
}
