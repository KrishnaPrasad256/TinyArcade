#include "Display.h"

#include <Wire.h>

namespace {

constexpr int kOledReset = -1;
constexpr uint8_t kOledAddress = 0x3C;
constexpr int kSdaPin = 0;   // GPIO 0 for SDA (ESP32C3)
constexpr int kSclPin = 1;   // GPIO 1 for SCL (ESP32C3)

Adafruit_SSD1306 display(Display::kScreenWidth, Display::kScreenHeight, &Wire,
                         kOledReset);

}  // namespace

namespace Display {

bool begin() {
  Wire.begin(kSdaPin, kSclPin);
  return display.begin(SSD1306_SWITCHCAPVCC, kOledAddress);
}

Adafruit_SSD1306& instance() { return display; }

void drawFullScreenBitmap(const uint8_t* bitmap) {
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap, kScreenWidth, kScreenHeight, SSD1306_WHITE);
  display.display();
}

void drawText(uint8_t textSize, int16_t x, int16_t y, const char* text) {
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(x, y);
  display.println(text);
  display.display();
}

void clearScreen() {
  display.clearDisplay();
  display.display();
}

void resetContrast() {
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(255);
}

}  // namespace Display
