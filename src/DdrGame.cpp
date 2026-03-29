#include "DdrGame.h"

#include "Display.h"
#include "HighScore.h"
#include "Input.h"

namespace {

constexpr int kDdrTargetY = 50;
constexpr int kDdrSpawnY = -8;
constexpr int kDdrArrowSize = 8;
constexpr int kDdrLaneCount = 4;
constexpr int kDdrMaxArrows = 12;
constexpr int kDdrPerfectWindow = 4;
constexpr int kDdrGoodWindow = 8;
constexpr int kDdrOkWindow = 12;
constexpr unsigned long kFrameIntervalMs = 33;
constexpr int kDdrHudX = 98;
constexpr int kDdrLaneLeft = 8;
constexpr int kDdrLaneWidth = 18;
constexpr int kDdrLaneGap = 4;

enum class ArrowDirection : uint8_t {
  UP,
  DOWN,
  LEFT,
  RIGHT
};

struct DdrArrow {
  int16_t x = 0;
  int16_t y = kDdrSpawnY;
  ArrowDirection direction = ArrowDirection::UP;
  bool active = false;
  bool hit = false;
  unsigned long spawnTime = 0;
};

struct DdrRuntime {
  DdrArrow arrows[kDdrMaxArrows];
  int score = 0;
  int combo = 0;
  int maxCombo = 0;
  int lives = 3;
  int bpm = 60;
  unsigned long lastSpawnMs = 0;
  unsigned long spawnIntervalMs = 500;
  unsigned long gameStartTimeMs = 0;
  unsigned long lastFrameMs = 0;
  bool playing = false;
};

DdrRuntime ddrRuntime;

constexpr int kLaneBoundsX[kDdrLaneCount] = {
    kDdrLaneLeft,
    kDdrLaneLeft + (kDdrLaneWidth + kDdrLaneGap),
    kDdrLaneLeft + (2 * (kDdrLaneWidth + kDdrLaneGap)),
    kDdrLaneLeft + (3 * (kDdrLaneWidth + kDdrLaneGap))};

constexpr int kLaneX[kDdrLaneCount] = {
    kLaneBoundsX[0] + ((kDdrLaneWidth - kDdrArrowSize) / 2),
    kLaneBoundsX[1] + ((kDdrLaneWidth - kDdrArrowSize) / 2),
    kLaneBoundsX[2] + ((kDdrLaneWidth - kDdrArrowSize) / 2),
    kLaneBoundsX[3] + ((kDdrLaneWidth - kDdrArrowSize) / 2)};

constexpr uint8_t kArrowUpBitmap[kDdrArrowSize] PROGMEM = {
    0x18, 0x3C, 0x7E, 0xFF, 0x18, 0x18, 0x18, 0x18};
constexpr uint8_t kArrowDownBitmap[kDdrArrowSize] PROGMEM = {
    0x18, 0x18, 0x18, 0x18, 0xFF, 0x7E, 0x3C, 0x18};
constexpr uint8_t kArrowLeftBitmap[kDdrArrowSize] PROGMEM = {
    0x10, 0x30, 0x60, 0xFF, 0xFF, 0x60, 0x30, 0x10};
constexpr uint8_t kArrowRightBitmap[kDdrArrowSize] PROGMEM = {
    0x08, 0x0C, 0x06, 0xFF, 0xFF, 0x06, 0x0C, 0x08};
constexpr uint8_t kHeartBitmap[8] PROGMEM = {
    0x66, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00};

void initDdr() {
  ddrRuntime.score = 0;
  ddrRuntime.combo = 0;
  ddrRuntime.maxCombo = 0;
  ddrRuntime.lives = 3;
  ddrRuntime.bpm = 120;
  ddrRuntime.spawnIntervalMs = 60000UL / ddrRuntime.bpm;
  ddrRuntime.lastSpawnMs = millis();
  ddrRuntime.gameStartTimeMs = millis();
  ddrRuntime.lastFrameMs = ddrRuntime.lastSpawnMs;
  ddrRuntime.playing = true;

  for (int i = 0; i < kDdrMaxArrows; ++i) {
    ddrRuntime.arrows[i] = DdrArrow{};
  }
}

void spawnArrow() {
  for (int i = 0; i < kDdrMaxArrows; ++i) {
    DdrArrow& arrow = ddrRuntime.arrows[i];
    if (arrow.active) {
      continue;
    }

    arrow.active = true;
    arrow.hit = false;
    arrow.y = kDdrSpawnY;
    arrow.spawnTime = millis();

    const long laneRoll = random(100);
    if (laneRoll < 25) {
      arrow.direction = ArrowDirection::LEFT;
      arrow.x = kLaneX[0];
    } else if (laneRoll < 50) {
      arrow.direction = ArrowDirection::DOWN;
      arrow.x = kLaneX[1];
    } else if (laneRoll < 75) {
      arrow.direction = ArrowDirection::UP;
      arrow.x = kLaneX[2];
    } else {
      arrow.direction = ArrowDirection::RIGHT;
      arrow.x = kLaneX[3];
    }
    return;
  }
}

bool isDirectionButton(ButtonState input) {
  return input == ButtonState::LEFT || input == ButtonState::DOWN ||
         input == ButtonState::UP || input == ButtonState::RIGHT;
}

ArrowDirection directionFromButton(ButtonState input) {
  switch (input) {
    case ButtonState::LEFT:
      return ArrowDirection::LEFT;
    case ButtonState::DOWN:
      return ArrowDirection::DOWN;
    case ButtonState::UP:
      return ArrowDirection::UP;
    case ButtonState::RIGHT:
      return ArrowDirection::RIGHT;
    case ButtonState::SEL:
    case ButtonState::NON:
      return ArrowDirection::UP;
  }

  return ArrowDirection::UP;
}

const uint8_t* bitmapForDirection(ArrowDirection direction) {
  switch (direction) {
    case ArrowDirection::UP:
      return kArrowUpBitmap;
    case ArrowDirection::DOWN:
      return kArrowDownBitmap;
    case ArrowDirection::LEFT:
      return kArrowLeftBitmap;
    case ArrowDirection::RIGHT:
      return kArrowRightBitmap;
  }

  return kArrowUpBitmap;
}

void handleInput(ButtonState input) {
  if (!ddrRuntime.playing || !isDirectionButton(input)) {
    return;
  }

  const ArrowDirection targetDirection = directionFromButton(input);
  int closestIndex = -1;
  int closestDistance = 999;

  for (int i = 0; i < kDdrMaxArrows; ++i) {
    const DdrArrow& arrow = ddrRuntime.arrows[i];
    if (!arrow.active || arrow.hit || arrow.direction != targetDirection) {
      continue;
    }

    const int distance = abs(arrow.y - kDdrTargetY);
    if (distance < closestDistance) {
      closestDistance = distance;
      closestIndex = i;
    }
  }

  if (closestIndex >= 0 && closestDistance <= kDdrOkWindow) {
    DdrArrow& arrow = ddrRuntime.arrows[closestIndex];
    arrow.hit = true;
    arrow.active = false;

    if (closestDistance <= kDdrPerfectWindow) {
      ddrRuntime.score += 100 * (1 + ddrRuntime.combo / 10);
    } else if (closestDistance <= kDdrGoodWindow) {
      ddrRuntime.score += 50 * (1 + ddrRuntime.combo / 10);
    } else {
      ddrRuntime.score += 20;
    }

    ++ddrRuntime.combo;
    if (ddrRuntime.combo > ddrRuntime.maxCombo) {
      ddrRuntime.maxCombo = ddrRuntime.combo;
    }
    return;
  }

  ddrRuntime.combo = 0;
}

void updateDifficulty() {
  if (ddrRuntime.score > 1000 && ddrRuntime.bpm < 160) {
    ddrRuntime.bpm = 160;
    ddrRuntime.spawnIntervalMs = 60000UL / ddrRuntime.bpm;
  } else if (ddrRuntime.score > 500 && ddrRuntime.bpm < 140) {
    ddrRuntime.bpm = 140;
    ddrRuntime.spawnIntervalMs = 60000UL / ddrRuntime.bpm;
  }
}

void updateDdr(ArcadeState& state, unsigned long currentTimeMs) {
  if (!ddrRuntime.playing) {
    return;
  }

  if (currentTimeMs - ddrRuntime.lastSpawnMs > ddrRuntime.spawnIntervalMs) {
    spawnArrow();
    ddrRuntime.lastSpawnMs = currentTimeMs;
    updateDifficulty();
  }

  const int fallSpeed = static_cast<int>(1.5f + (ddrRuntime.bpm / 60.0f));

  for (int i = 0; i < kDdrMaxArrows; ++i) {
    DdrArrow& arrow = ddrRuntime.arrows[i];
    if (!arrow.active || arrow.hit) {
      continue;
    }

    arrow.y += fallSpeed;

    if (arrow.y > Display::kScreenHeight + kDdrArrowSize) {
      arrow.active = false;
      ddrRuntime.combo = 0;
      --ddrRuntime.lives;

      if (ddrRuntime.lives <= 0) {
        ddrRuntime.playing = false;
        state.gameState = GameState::GAME_OVER;
        return;
      }
    }
  }
}

void renderDdr() {
  Adafruit_SSD1306& display = Display::instance();

  display.clearDisplay();

  for (int i = 0; i < kDdrLaneCount; ++i) {
    display.drawRect(kLaneBoundsX[i], 0, kDdrLaneWidth, Display::kScreenHeight,
                     SSD1306_WHITE);
  }

  for (int i = 0; i < kDdrLaneCount; ++i) {
    display.drawRoundRect(kLaneBoundsX[i] - 1, kDdrTargetY + 1,
                          kDdrLaneWidth + 2, kDdrArrowSize + 2, 2,
                          SSD1306_WHITE);
  }

  display.drawBitmap(kLaneX[0], kDdrTargetY + 2, kArrowLeftBitmap, kDdrArrowSize,
                     kDdrArrowSize, SSD1306_WHITE);
  display.drawBitmap(kLaneX[1], kDdrTargetY + 2, kArrowDownBitmap, kDdrArrowSize,
                     kDdrArrowSize, SSD1306_WHITE);
  display.drawBitmap(kLaneX[2], kDdrTargetY + 2, kArrowUpBitmap, kDdrArrowSize,
                     kDdrArrowSize, SSD1306_WHITE);
  display.drawBitmap(kLaneX[3], kDdrTargetY + 2, kArrowRightBitmap,
                     kDdrArrowSize, kDdrArrowSize, SSD1306_WHITE);

  for (int i = 0; i < kDdrMaxArrows; ++i) {
    const DdrArrow& arrow = ddrRuntime.arrows[i];
    if (!arrow.active || arrow.hit) {
      continue;
    }

    display.drawBitmap(arrow.x, arrow.y, bitmapForDirection(arrow.direction),
                       kDdrArrowSize, kDdrArrowSize, SSD1306_WHITE);
  }

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false);
  display.setCursor(kDdrHudX, 0);
  display.print("SCORE");
  display.setCursor(kDdrHudX, 8);
  display.print(ddrRuntime.score);

  if (ddrRuntime.combo > 1) {
    display.setCursor(kDdrHudX, 18);
    display.print(ddrRuntime.combo);
    display.print("x");
  }

  for (int i = 0; i < ddrRuntime.lives; ++i) {
    display.drawBitmap(kDdrHudX + (i * 10), 30, kHeartBitmap, 8, 8,
                       SSD1306_WHITE);
  }

  display.display();
}

}  // namespace

namespace DdrGame {

void run(ArcadeState& state) {
  initDdr();
  renderDdr();

  while (state.gameState == GameState::PLAYING && ddrRuntime.playing) {
    Input::poll(state);

    if (state.buttonState == ButtonState::SEL) {
      ddrRuntime.playing = false;
      state.gameState = GameState::GAME_OVER;
      break;
    }

    handleInput(state.buttonState);

    const unsigned long currentTimeMs = millis();
    if (currentTimeMs - ddrRuntime.lastFrameMs >= kFrameIntervalMs) {
      updateDdr(state, currentTimeMs);
      renderDdr();
      ddrRuntime.lastFrameMs = currentTimeMs;
    }

    delay(1);
  }

  HighScore::update(GameSelection::DDR_GAME,
                    static_cast<uint32_t>(ddrRuntime.score));
}

int currentScore() { return ddrRuntime.score; }

}  // namespace DdrGame
