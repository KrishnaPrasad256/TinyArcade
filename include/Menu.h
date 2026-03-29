#pragma once

#include "ArcadeTypes.h"

namespace Menu {

void showLogo();
void waitForAnyButton(ArcadeState& state);
void runMainMenu(ArcadeState& state);
void runGameOverMenu(ArcadeState& state);
void runGamesMenu(ArcadeState& state);

}  // namespace Menu
