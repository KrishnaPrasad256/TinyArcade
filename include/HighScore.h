#pragma once

#include <Arduino.h>

#include "ArcadeTypes.h"

namespace HighScore {

bool begin();
uint32_t get(GameSelection selection);
void update(GameSelection selection, uint32_t score);
void formatForMenu(GameSelection selection, char* buffer, size_t bufferSize);

}  // namespace HighScore
