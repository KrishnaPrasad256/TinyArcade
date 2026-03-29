#include "Input.h"

namespace {

constexpr uint8_t kButtonCount = 5;
constexpr int kPins[kButtonCount] = {4, 5, 18, 19, 23};
constexpr unsigned long kDebounceDelayMs = 50;

bool buttonPressed(int pin, uint8_t index) {
  static int lastReading[kButtonCount] = {1, 1, 1, 1, 1};
  static int buttonState[kButtonCount] = {1, 1, 1, 1, 1};
  static int lastButtonState[kButtonCount] = {1, 1, 1, 1, 1};
  static unsigned long lastDebounceTime[kButtonCount] = {0, 0, 0, 0, 0};

  const int reading = digitalRead(pin);

  if (reading != lastReading[index]) {
    lastDebounceTime[index] = millis();
  }

  if ((millis() - lastDebounceTime[index]) > kDebounceDelayMs) {
    if (reading != buttonState[index]) {
      buttonState[index] = reading;
    }
  }

  lastReading[index] = reading;

  bool pressed = false;
  if (lastButtonState[index] == HIGH && buttonState[index] == LOW) {
    pressed = true;
  }

  lastButtonState[index] = buttonState[index];

  return pressed;
}

ButtonState buttonStateFromIndex(uint8_t index) {
  switch (index) {
    case 0:
      return ButtonState::UP;
    case 1:
      return ButtonState::DOWN;
    case 2:
      return ButtonState::LEFT;
    case 3:
      return ButtonState::RIGHT;
    case 4:
      return ButtonState::SEL;
    default:
      return ButtonState::NON;
  }
}

int pinFromButtonState(ButtonState button) {
  switch (button) {
    case ButtonState::UP:
      return kPins[0];
    case ButtonState::DOWN:
      return kPins[1];
    case ButtonState::LEFT:
      return kPins[2];
    case ButtonState::RIGHT:
      return kPins[3];
    case ButtonState::SEL:
      return kPins[4];
    case ButtonState::NON:
      return -1;
  }

  return -1;
}

}  // namespace

namespace Input {

void init() {
  for (uint8_t i = 0; i < kButtonCount; ++i) {
    pinMode(kPins[i], INPUT_PULLUP);
  }
}

void poll(ArcadeState& state) {
  state.buttonState = ButtonState::NON;

  for (uint8_t i = 0; i < kButtonCount; ++i) {
    if (buttonPressed(kPins[i], i)) {
      state.buttonState = buttonStateFromIndex(i);
    }
  }
}

bool isHeld(ButtonState button) {
  const int pin = pinFromButtonState(button);
  if (pin < 0) {
    return false;
  }

  return digitalRead(pin) == LOW;
}

}  // namespace Input
