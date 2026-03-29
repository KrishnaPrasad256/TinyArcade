#pragma once

#include <Arduino.h>

enum class GameState : uint8_t {
  MENU,
  GAME_OVER,
  PLAYING,
  GAMES_MENU
};

enum class GameSelection : uint8_t {
  TETRIS,
  DDR_GAME
};

enum class ButtonState : uint8_t {
  UP,
  DOWN,
  LEFT,
  RIGHT,
  SEL,
  NON
};

enum class MenuButton : uint8_t {
  PLAY,
  EXIT
};

struct ArcadeState {
  GameState gameState = GameState::GAMES_MENU;
  GameSelection selectedGame = GameSelection::TETRIS;
  ButtonState buttonState = ButtonState::NON;
  MenuButton menuButton = MenuButton::PLAY;
};
