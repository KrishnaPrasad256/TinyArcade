#include "Menu.h"

#include <stdio.h>

#include "Bitmaps.h"
#include "DdrGame.h"
#include "Display.h"
#include "HighScore.h"
#include "Input.h"
#include "TetrisGame.h"

namespace {

constexpr uint8_t kMenuScoreTextSize = 1;
constexpr int16_t kMenuScoreX = 15;
constexpr int16_t kMenuScoreY = 50;

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
  if (state.menuButton == MenuButton::PLAY) {
    Display::drawFullScreenBitmap(Bitmaps::kMenuPlayBitmap);
  } else {
    Display::drawFullScreenBitmap(Bitmaps::kMenuExitBitmap);
  }

  char highScoreText[12];
  HighScore::formatForMenu(state.selectedGame, highScoreText,
                           sizeof(highScoreText));
  drawMenuScoreText(highScoreText);
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
  renderMainMenu(state);

  while (state.gameState == GameState::MENU) {
    Input::poll(state);

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
  Display::drawFullScreenBitmap(Bitmaps::kGameOverBitmap);
  char currentScoreText[12];
  formatCurrentScoreText(state, currentScoreText, sizeof(currentScoreText));
  drawMenuScoreText(currentScoreText);
  state.buttonState = ButtonState::NON;
  waitForAnyButton(state);
  state.buttonState = ButtonState::NON;
  state.menuButton = MenuButton::PLAY;
  state.gameState = GameState::MENU;
}

void runGamesMenu(ArcadeState& state) {
  bool esc = false;

  while (!esc) {
    renderGamesMenu(state);

    for (;;) {
      Input::poll(state);

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
