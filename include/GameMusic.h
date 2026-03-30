#pragma once

#include "ArcadeTypes.h"

namespace GameMusic {

void init();
void playMenuMusic();
void playGameOverMusic();
void playForGame(GameSelection game);
void update();
void stop();

}  // namespace GameMusic
