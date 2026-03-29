#include "TetrisGame.h"

#include "Display.h"
#include "HighScore.h"
#include "Input.h"

namespace {

constexpr int kBoardWidth = 10;
constexpr int kBoardHeight = 16;
constexpr int kCellSize = 4;
constexpr int kBoardPixelWidth = kBoardWidth * kCellSize;
constexpr int kSidebarX = 46;
constexpr int kPreviewBoxX = 84;
constexpr int kPreviewBoxY = 8;
constexpr int kPreviewCellSize = 3;
constexpr int kSpawnX = 3;
constexpr int kSpawnY = 0;
constexpr unsigned long kFrameIntervalMs = 33;
constexpr unsigned long kSoftDropIntervalMs = 60;
constexpr uint16_t kLineClearScore[5] = {0, 100, 300, 500, 800};

enum class Tetromino : uint8_t {
  I,
  O,
  T,
  L,
  J,
  S,
  Z,
  COUNT
};

struct FallingPiece {
  Tetromino type = Tetromino::I;
  uint8_t rotation = 0;
  int8_t x = kSpawnX;
  int8_t y = kSpawnY;
};

struct TetrisRuntime {
  bool board[kBoardHeight][kBoardWidth] = {};
  FallingPiece currentPiece{};
  Tetromino nextPiece = Tetromino::I;
  uint8_t bag[static_cast<int>(Tetromino::COUNT)] = {};
  uint8_t bagIndex = static_cast<uint8_t>(Tetromino::COUNT);
  int score = 0;
  int lines = 0;
  int level = 1;
  unsigned long lastFallMs = 0;
  unsigned long lastSoftDropMs = 0;
  unsigned long lastFrameMs = 0;
  bool playing = false;
};

TetrisRuntime tetrisRuntime;

constexpr uint8_t kTetrominoShapes[static_cast<int>(Tetromino::COUNT)][4][4] = {
    {// I
     {0b0000, 0b1111, 0b0000, 0b0000},
     {0b0010, 0b0010, 0b0010, 0b0010},
     {0b0000, 0b1111, 0b0000, 0b0000},
     {0b0010, 0b0010, 0b0010, 0b0010}},
    {// O
     {0b0000, 0b0110, 0b0110, 0b0000},
     {0b0000, 0b0110, 0b0110, 0b0000},
     {0b0000, 0b0110, 0b0110, 0b0000},
     {0b0000, 0b0110, 0b0110, 0b0000}},
    {// T
     {0b0000, 0b1110, 0b0100, 0b0000},
     {0b0100, 0b1100, 0b0100, 0b0000},
     {0b0100, 0b1110, 0b0000, 0b0000},
     {0b0100, 0b0110, 0b0100, 0b0000}},
    {// L
     {0b0010, 0b1110, 0b0000, 0b0000},
     {0b0100, 0b0100, 0b0110, 0b0000},
     {0b0000, 0b1110, 0b1000, 0b0000},
     {0b1100, 0b0100, 0b0100, 0b0000}},
    {// J
     {0b1000, 0b1110, 0b0000, 0b0000},
     {0b0110, 0b0100, 0b0100, 0b0000},
     {0b0000, 0b1110, 0b0010, 0b0000},
     {0b0100, 0b0100, 0b1100, 0b0000}},
    {// S
     {0b0110, 0b1100, 0b0000, 0b0000},
     {0b1000, 0b1100, 0b0100, 0b0000},
     {0b0110, 0b1100, 0b0000, 0b0000},
     {0b1000, 0b1100, 0b0100, 0b0000}},
    {// Z
     {0b1100, 0b0110, 0b0000, 0b0000},
     {0b0100, 0b1100, 0b1000, 0b0000},
     {0b1100, 0b0110, 0b0000, 0b0000},
     {0b0100, 0b1100, 0b1000, 0b0000}}};

bool pieceCellFilled(Tetromino type, uint8_t rotation, int localX, int localY) {
  const uint8_t rowMask =
      kTetrominoShapes[static_cast<int>(type)][rotation % 4][localY];
  return (rowMask & (0x8 >> localX)) != 0;
}

void clearBoard() {
  for (int row = 0; row < kBoardHeight; ++row) {
    for (int col = 0; col < kBoardWidth; ++col) {
      tetrisRuntime.board[row][col] = false;
    }
  }
}

void refillBag() {
  for (int i = 0; i < static_cast<int>(Tetromino::COUNT); ++i) {
    tetrisRuntime.bag[i] = static_cast<uint8_t>(i);
  }

  for (int i = static_cast<int>(Tetromino::COUNT) - 1; i > 0; --i) {
    const int swapIndex = random(i + 1);
    const uint8_t tmp = tetrisRuntime.bag[i];
    tetrisRuntime.bag[i] = tetrisRuntime.bag[swapIndex];
    tetrisRuntime.bag[swapIndex] = tmp;
  }

  tetrisRuntime.bagIndex = 0;
}

Tetromino nextBagPiece() {
  if (tetrisRuntime.bagIndex >= static_cast<uint8_t>(Tetromino::COUNT)) {
    refillBag();
  }

  return static_cast<Tetromino>(tetrisRuntime.bag[tetrisRuntime.bagIndex++]);
}

bool pieceFits(Tetromino type, uint8_t rotation, int pieceX, int pieceY) {
  for (int localY = 0; localY < 4; ++localY) {
    for (int localX = 0; localX < 4; ++localX) {
      if (!pieceCellFilled(type, rotation, localX, localY)) {
        continue;
      }

      const int boardX = pieceX + localX;
      const int boardY = pieceY + localY;

      if (boardX < 0 || boardX >= kBoardWidth || boardY < 0 ||
          boardY >= kBoardHeight) {
        return false;
      }

      if (tetrisRuntime.board[boardY][boardX]) {
        return false;
      }
    }
  }

  return true;
}

unsigned long fallIntervalMs() {
  const long interval = 700L - ((tetrisRuntime.level - 1) * 55L);
  return static_cast<unsigned long>(interval < 120L ? 120L : interval);
}

void updateLevel() { tetrisRuntime.level = 1 + (tetrisRuntime.lines / 10); }

bool spawnPiece(ArcadeState& state) {
  tetrisRuntime.currentPiece.type = tetrisRuntime.nextPiece;
  tetrisRuntime.currentPiece.rotation = 0;
  tetrisRuntime.currentPiece.x = kSpawnX;
  tetrisRuntime.currentPiece.y = kSpawnY;
  tetrisRuntime.nextPiece = nextBagPiece();

  if (!pieceFits(tetrisRuntime.currentPiece.type,
                 tetrisRuntime.currentPiece.rotation,
                 tetrisRuntime.currentPiece.x, tetrisRuntime.currentPiece.y)) {
    tetrisRuntime.playing = false;
    state.gameState = GameState::GAME_OVER;
    return false;
  }

  return true;
}

void initTetris(ArcadeState& state) {
  randomSeed(micros());

  clearBoard();
  refillBag();

  tetrisRuntime.score = 0;
  tetrisRuntime.lines = 0;
  tetrisRuntime.level = 1;
  tetrisRuntime.playing = true;
  tetrisRuntime.nextPiece = nextBagPiece();
  tetrisRuntime.lastFallMs = millis();
  tetrisRuntime.lastSoftDropMs = tetrisRuntime.lastFallMs;
  tetrisRuntime.lastFrameMs = tetrisRuntime.lastFallMs;

  spawnPiece(state);
}

bool movePiece(int deltaX, int deltaY) {
  const int nextX = tetrisRuntime.currentPiece.x + deltaX;
  const int nextY = tetrisRuntime.currentPiece.y + deltaY;

  if (!pieceFits(tetrisRuntime.currentPiece.type,
                 tetrisRuntime.currentPiece.rotation, nextX, nextY)) {
    return false;
  }

  tetrisRuntime.currentPiece.x = static_cast<int8_t>(nextX);
  tetrisRuntime.currentPiece.y = static_cast<int8_t>(nextY);
  return true;
}

void rotatePiece() {
  const uint8_t rotated = (tetrisRuntime.currentPiece.rotation + 1) % 4;
  constexpr int kKickOffsets[] = {0, -1, 1, -2, 2};

  for (int offset : kKickOffsets) {
    const int nextX = tetrisRuntime.currentPiece.x + offset;
    if (!pieceFits(tetrisRuntime.currentPiece.type, rotated, nextX,
                   tetrisRuntime.currentPiece.y)) {
      continue;
    }

    tetrisRuntime.currentPiece.rotation = rotated;
    tetrisRuntime.currentPiece.x = static_cast<int8_t>(nextX);
    return;
  }
}

void lockPiece() {
  for (int localY = 0; localY < 4; ++localY) {
    for (int localX = 0; localX < 4; ++localX) {
      if (!pieceCellFilled(tetrisRuntime.currentPiece.type,
                           tetrisRuntime.currentPiece.rotation, localX,
                           localY)) {
        continue;
      }

      const int boardX = tetrisRuntime.currentPiece.x + localX;
      const int boardY = tetrisRuntime.currentPiece.y + localY;
      if (boardX >= 0 && boardX < kBoardWidth && boardY >= 0 &&
          boardY < kBoardHeight) {
        tetrisRuntime.board[boardY][boardX] = true;
      }
    }
  }
}

int clearCompletedLines() {
  int clearedLines = 0;

  for (int row = kBoardHeight - 1; row >= 0; --row) {
    bool fullRow = true;
    for (int col = 0; col < kBoardWidth; ++col) {
      if (!tetrisRuntime.board[row][col]) {
        fullRow = false;
        break;
      }
    }

    if (!fullRow) {
      continue;
    }

    ++clearedLines;
    for (int moveRow = row; moveRow > 0; --moveRow) {
      for (int col = 0; col < kBoardWidth; ++col) {
        tetrisRuntime.board[moveRow][col] = tetrisRuntime.board[moveRow - 1][col];
      }
    }

    for (int col = 0; col < kBoardWidth; ++col) {
      tetrisRuntime.board[0][col] = false;
    }

    ++row;
  }

  return clearedLines;
}

void applyLineClearScore(int clearedLines) {
  if (clearedLines <= 0) {
    return;
  }

  tetrisRuntime.lines += clearedLines;
  updateLevel();
  tetrisRuntime.score += kLineClearScore[clearedLines] * tetrisRuntime.level;
}

void stepGame(ArcadeState& state) {
  if (movePiece(0, 1)) {
    return;
  }

  lockPiece();
  applyLineClearScore(clearCompletedLines());
  spawnPiece(state);
}

void softDrop(ArcadeState& state) {
  if (movePiece(0, 1)) {
    ++tetrisRuntime.score;
    return;
  }

  lockPiece();
  applyLineClearScore(clearCompletedLines());
  spawnPiece(state);
}

void handleInput(ArcadeState& state, unsigned long currentTimeMs) {
  switch (state.buttonState) {
    case ButtonState::LEFT:
      movePiece(-1, 0);
      break;

    case ButtonState::RIGHT:
      movePiece(1, 0);
      break;

    case ButtonState::UP:
      rotatePiece();
      break;

    case ButtonState::DOWN:
      softDrop(state);
      tetrisRuntime.lastFallMs = currentTimeMs;
      tetrisRuntime.lastSoftDropMs = currentTimeMs;
      break;

    case ButtonState::SEL:
      tetrisRuntime.playing = false;
      state.gameState = GameState::GAME_OVER;
      break;

    case ButtonState::NON:
      break;
  }
}

void drawBoardCell(Adafruit_SSD1306& display, int col, int row, int cellSize) {
  const int pixelX = col * cellSize;
  const int pixelY = row * cellSize;
  display.fillRect(pixelX + 1, pixelY + 1, cellSize - 1, cellSize - 1,
                   SSD1306_WHITE);
}

void drawPreviewPiece(Adafruit_SSD1306& display) {
  display.drawRect(kPreviewBoxX, kPreviewBoxY, 18, 18, SSD1306_WHITE);

  for (int localY = 0; localY < 4; ++localY) {
    for (int localX = 0; localX < 4; ++localX) {
      if (!pieceCellFilled(tetrisRuntime.nextPiece, 0, localX, localY)) {
        continue;
      }

      const int previewX = kPreviewBoxX + 2 + (localX * kPreviewCellSize);
      const int previewY = kPreviewBoxY + 2 + (localY * kPreviewCellSize);
      display.fillRect(previewX, previewY, kPreviewCellSize - 1,
                       kPreviewCellSize - 1, SSD1306_WHITE);
    }
  }
}

void renderTetris() {
  Adafruit_SSD1306& display = Display::instance();
  display.clearDisplay();
  display.drawRect(0, 0, kBoardPixelWidth + 1, Display::kScreenHeight,
                   SSD1306_WHITE);

  for (int row = 0; row < kBoardHeight; ++row) {
    for (int col = 0; col < kBoardWidth; ++col) {
      if (tetrisRuntime.board[row][col]) {
        drawBoardCell(display, col, row, kCellSize);
      }
    }
  }

  for (int localY = 0; localY < 4; ++localY) {
    for (int localX = 0; localX < 4; ++localX) {
      if (!pieceCellFilled(tetrisRuntime.currentPiece.type,
                           tetrisRuntime.currentPiece.rotation, localX,
                           localY)) {
        continue;
      }

      const int boardX = tetrisRuntime.currentPiece.x + localX;
      const int boardY = tetrisRuntime.currentPiece.y + localY;
      if (boardX >= 0 && boardX < kBoardWidth && boardY >= 0 &&
          boardY < kBoardHeight) {
        drawBoardCell(display, boardX, boardY, kCellSize);
      }
    }
  }

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);

  display.setCursor(kSidebarX, 0);
  display.print("SCORE");
  display.setCursor(kSidebarX, 8);
  display.print(tetrisRuntime.score);

  display.setCursor(kSidebarX, 20);
  display.print("LINE");
  display.setCursor(kSidebarX, 28);
  display.print(tetrisRuntime.lines);

  display.setCursor(kSidebarX, 40);
  display.print("LEVEL");
  display.setCursor(kSidebarX, 48);
  display.print(tetrisRuntime.level);

  display.setCursor(kPreviewBoxX, 0);
  display.print("NEXT");
  drawPreviewPiece(display);

  display.display();
}

}  // namespace

namespace TetrisGame {

void run(ArcadeState& state) {
  initTetris(state);
  if (!tetrisRuntime.playing) {
    HighScore::update(GameSelection::TETRIS,
                      static_cast<uint32_t>(tetrisRuntime.score));
    return;
  }

  renderTetris();

  while (state.gameState == GameState::PLAYING && tetrisRuntime.playing) {
    const unsigned long currentTimeMs = millis();

    Input::poll(state);
    handleInput(state, currentTimeMs);

    if (!tetrisRuntime.playing || state.gameState != GameState::PLAYING) {
      break;
    }

    if (Input::isHeld(ButtonState::DOWN) &&
        currentTimeMs - tetrisRuntime.lastSoftDropMs >= kSoftDropIntervalMs) {
      softDrop(state);
      tetrisRuntime.lastSoftDropMs = currentTimeMs;
      tetrisRuntime.lastFallMs = currentTimeMs;
    }

    if (!tetrisRuntime.playing || state.gameState != GameState::PLAYING) {
      break;
    }

    if (currentTimeMs - tetrisRuntime.lastFallMs >= fallIntervalMs()) {
      stepGame(state);
      tetrisRuntime.lastFallMs = currentTimeMs;
    }

    if (currentTimeMs - tetrisRuntime.lastFrameMs >= kFrameIntervalMs) {
      renderTetris();
      tetrisRuntime.lastFrameMs = currentTimeMs;
    }

    delay(1);
  }

  HighScore::update(GameSelection::TETRIS,
                    static_cast<uint32_t>(tetrisRuntime.score));
}

int currentScore() { return tetrisRuntime.score; }

}  // namespace TetrisGame
