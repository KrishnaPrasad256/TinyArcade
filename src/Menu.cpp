#include "Menu.h"

#include <stdio.h>

#include "Bitmaps.h"
#include "DdrGame.h"
#include "Display.h"
#include "GameMusic.h"
#include "HighScore.h"
#include "Input.h"
#include "TetrisGame.h"

namespace {

constexpr uint8_t kMenuScoreTextSize = 1;
constexpr int16_t kMenuScoreX = 75;
constexpr int16_t kMenuScoreY = 56;

void drawMenuScoreText(const char* scoreText) {
  Adafruit_SSD1306& display = Display::instance();
  display.setTextSize(kMenuScoreTextSize);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
  display.setCursor(kMenuScoreX, kMenuScoreY);
  display.print(scoreText);
  display.display();
}

void formatCurrentScoreText(const ArcadeState& state, char* buffer,
                            size_t bufferSize) {
  const int currentScore = state.selectedGame == GameSelection::DDR_GAME
                               ? DdrGame::currentScore()
                               : TetrisGame::currentScore();
  snprintf(buffer, bufferSize, "%d", currentScore);
}

void renderMainMenu(const ArcadeState& state) {
  const uint8_t* bitmap = nullptr;
  
  if (state.selectedGame == GameSelection::TETRIS) {
    bitmap = state.menuButton == MenuButton::PLAY
                 ? Bitmaps::kTetrisPlayBitmap
                 : Bitmaps::kTetrisExitBitmap;
  } else {
    bitmap = state.menuButton == MenuButton::PLAY
                 ? Bitmaps::kDdrPlayBitmap
                 : Bitmaps::kDdrExitBitmap;
  }
  
  Display::drawFullScreenBitmap(bitmap);

  char highScoreText[12];
  HighScore::formatForMenu(state.selectedGame, highScoreText,
                           sizeof(highScoreText));
  drawMenuScoreText(highScoreText);
}

void drawGameOverScore(const ArcadeState& state) {
  constexpr uint8_t kGameOverScoreTextSize = 1;
  constexpr int16_t kGameOverScoreX = 15;
  constexpr int16_t kGameOverScoreY = 50;

  Adafruit_SSD1306& display = Display::instance();
  display.setTextSize(kGameOverScoreTextSize);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
  display.setCursor(kGameOverScoreX, kGameOverScoreY);

  const int currentScore = state.selectedGame == GameSelection::DDR_GAME
                               ? DdrGame::currentScore()
                               : TetrisGame::currentScore();
  char scoreText[12];
  snprintf(scoreText, sizeof(scoreText), "%d", currentScore);
  display.print(scoreText);
  display.display();
}

void renderGamesMenu(const ArcadeState& state) {
  if (state.selectedGame == GameSelection::TETRIS) {
    Display::drawFullScreenBitmap(Bitmaps::kTetrisSelectedBitmap);
  } else {
    Display::drawFullScreenBitmap(Bitmaps::kDdrSelectedBitmap);
  }
}

}  // namespace

namespace Menu {

void showLogo() { Display::drawFullScreenBitmap(Bitmaps::kLogoBitmap); }

void waitForAnyButton(ArcadeState& state) {
  for (;;) {
    Input::poll(state);
    if (state.buttonState != ButtonState::NON) {
      return;
    }
  }
}

void runMainMenu(ArcadeState& state) {
  GameMusic::playMenuMusic();
  renderMainMenu(state);

  while (state.gameState == GameState::MENU) {
    Input::poll(state);
    GameMusic::update();

    if (state.buttonState == ButtonState::DOWN ||
        state.buttonState == ButtonState::UP) {
      state.menuButton = state.menuButton == MenuButton::PLAY
                             ? MenuButton::EXIT
                             : MenuButton::PLAY;
      renderMainMenu(state);
      continue;
    }

    if (state.buttonState != ButtonState::SEL) {
      continue;
    }

    if (state.menuButton == MenuButton::EXIT) {
      state.gameState = GameState::GAMES_MENU;
      state.menuButton = MenuButton::PLAY;
      return;
    }

    state.gameState = GameState::PLAYING;
    return;
  }
}

void runGameOverMenu(ArcadeState& state) {
  GameMusic::playGameOverMusic();
  Display::drawFullScreenBitmap(Bitmaps::kGameOverBitmap);
  drawGameOverScore(state);
  state.buttonState = ButtonState::NON;

  while (state.gameState == GameState::GAME_OVER) {
    Input::poll(state);
    GameMusic::update();

    if (state.buttonState == ButtonState::SEL) {
      state.buttonState = ButtonState::NON;
      state.menuButton = MenuButton::PLAY;
      state.gameState = GameState::MENU;
      return;
    }
  }
}

void runGamesMenu(ArcadeState& state) {
  GameMusic::playMenuMusic();
  bool esc = false;

  while (!esc) {
    renderGamesMenu(state);

    for (;;) {
      Input::poll(state);
      GameMusic::update();

      if (state.buttonState == ButtonState::DOWN ||
          state.buttonState == ButtonState::UP) {
        state.selectedGame = state.selectedGame == GameSelection::TETRIS
                                 ? GameSelection::DDR_GAME
                                 : GameSelection::TETRIS;
        renderGamesMenu(state);
        continue;
      }

      if (state.buttonState == ButtonState::SEL) {
        state.gameState = GameState::MENU;
        esc = true;
        break;
      }
    }
  }
}

}  // namespace Menu
