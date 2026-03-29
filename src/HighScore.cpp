#include "HighScore.h"

#include <EEPROM.h>
#include <stdio.h>

namespace {

constexpr size_t kEepromSize = 64;
constexpr uint32_t kStorageMagic = 0x54415243UL;

struct HighScoreStore {
  uint32_t magic = kStorageMagic;
  uint32_t tetris = 0;
  uint32_t ddr = 0;
};

HighScoreStore highScoreStore;
bool highScoreReady = false;

void commitStore() {
  EEPROM.put(0, highScoreStore);
  EEPROM.commit();
}

uint32_t& scoreSlot(GameSelection selection) {
  if (selection == GameSelection::DDR_GAME) {
    return highScoreStore.ddr;
  }

  return highScoreStore.tetris;
}

}  // namespace

namespace HighScore {

bool begin() {
  if (!EEPROM.begin(kEepromSize)) {
    highScoreStore = HighScoreStore{};
    highScoreReady = false;
    return false;
  }

  EEPROM.get(0, highScoreStore);
  if (highScoreStore.magic != kStorageMagic || highScoreStore.tetris == UINT32_MAX ||
      highScoreStore.ddr == UINT32_MAX) {
    highScoreStore = HighScoreStore{};
    commitStore();
  }

  highScoreReady = true;
  return true;
}

uint32_t get(GameSelection selection) { return scoreSlot(selection); }

void update(GameSelection selection, uint32_t score) {
  uint32_t& storedScore = scoreSlot(selection);
  if (score <= storedScore) {
    return;
  }

  storedScore = score;
  if (highScoreReady) {
    commitStore();
  }
}

void formatForMenu(GameSelection selection, char* buffer, size_t bufferSize) {
  snprintf(buffer, bufferSize, "%lu",
           static_cast<unsigned long>(get(selection)));
}

}  // namespace HighScore
