#pragma once

#include "ArcadeTypes.h"

namespace Input {

void init();
void poll(ArcadeState& state);
bool isHeld(ButtonState button);

}  // namespace Input
