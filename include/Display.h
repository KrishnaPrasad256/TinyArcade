#pragma once

#include <Adafruit_SSD1306.h>

namespace Display {

constexpr int kScreenWidth = 128;
constexpr int kScreenHeight = 64;

bool begin();
Adafruit_SSD1306& instance();
void drawFullScreenBitmap(const uint8_t* bitmap);
void drawText(uint8_t textSize, int16_t x, int16_t y, const char* text);
void fadeIn();
void fadeOut();
void clearScreen();
void resetContrast();

}  // namespace Display
