#include "GameMusic.h"

#include <Arduino.h>

namespace {

#ifndef ARCADE_BUZZER_PIN
#define ARCADE_BUZZER_PIN 26
#endif

constexpr uint8_t kBuzzerPin = ARCADE_BUZZER_PIN;
constexpr uint16_t kNoteRest = 0;
constexpr uint16_t kNoteC3 = 131;
constexpr uint16_t kNoteF3 = 175;
constexpr uint16_t kNoteG3 = 196;
constexpr uint16_t kNoteGs3 = 208;
constexpr uint16_t kNoteGs4 = 415;
constexpr uint16_t kNoteA3 = 220;
constexpr uint16_t kNoteA4 = 440;
constexpr uint16_t kNoteA5 = 880;
constexpr uint16_t kNoteB3 = 247;
constexpr uint16_t kNoteB4 = 494;
constexpr uint16_t kNoteCs4 = 277;
constexpr uint16_t kNoteCs5 = 554;
constexpr uint16_t kNoteC4 = 262;
constexpr uint16_t kNoteC5 = 523;
constexpr uint16_t kNoteC6 = 1047;
constexpr uint16_t kNoteD4 = 294;
constexpr uint16_t kNoteD5 = 587;
constexpr uint16_t kNoteDs4 = 311;
constexpr uint16_t kNoteE4 = 330;
constexpr uint16_t kNoteE5 = 659;
constexpr uint16_t kNoteF4 = 349;
constexpr uint16_t kNoteF5 = 698;
constexpr uint16_t kNoteFs4 = 370;
constexpr uint16_t kNoteG4 = 392;
constexpr uint16_t kNoteG5 = 784;
constexpr uint16_t kNoteGs5 = 831;

struct NoteEvent {
  uint16_t frequency;
  int8_t divider;
};

struct ThemeData {
  const NoteEvent* notes;
  size_t noteCount;
  uint16_t tempo;
  uint16_t loopGapMs;
};

struct PlaybackState {
  const ThemeData* theme = nullptr;
  size_t noteIndex = 0;
  unsigned long noteStartedMs = 0;
  unsigned long noteLengthMs = 0;
  bool initialized = false;
  bool playing = false;
};

#define SIXTEENTH(note) \
  NoteEvent { note, 16 }

static constexpr NoteEvent kDdrTheme[] = {
    // Part 1: sweeping arpeggios
    SIXTEENTH(kNoteC4), SIXTEENTH(kNoteD4), SIXTEENTH(kNoteE4),
    SIXTEENTH(kNoteG4), SIXTEENTH(kNoteC5), SIXTEENTH(kNoteD5),
    SIXTEENTH(kNoteE5), SIXTEENTH(kNoteG5), SIXTEENTH(kNoteC6),
    SIXTEENTH(kNoteG5), SIXTEENTH(kNoteE5), SIXTEENTH(kNoteD5),
    SIXTEENTH(kNoteC5), SIXTEENTH(kNoteG4), SIXTEENTH(kNoteE4),
    SIXTEENTH(kNoteD4), SIXTEENTH(kNoteA3), SIXTEENTH(kNoteB3),
    SIXTEENTH(kNoteC4), SIXTEENTH(kNoteE4), SIXTEENTH(kNoteA4),
    SIXTEENTH(kNoteB4), SIXTEENTH(kNoteC5), SIXTEENTH(kNoteE5),
    SIXTEENTH(kNoteA5), SIXTEENTH(kNoteE5), SIXTEENTH(kNoteC5),
    SIXTEENTH(kNoteB4), SIXTEENTH(kNoteA4), SIXTEENTH(kNoteE4),
    SIXTEENTH(kNoteC4), SIXTEENTH(kNoteB3), SIXTEENTH(kNoteF3),
    SIXTEENTH(kNoteG3), SIXTEENTH(kNoteA3), SIXTEENTH(kNoteC4),
    SIXTEENTH(kNoteF4), SIXTEENTH(kNoteG4), SIXTEENTH(kNoteA4),
    SIXTEENTH(kNoteC5), SIXTEENTH(kNoteF5), SIXTEENTH(kNoteC5),
    SIXTEENTH(kNoteA4), SIXTEENTH(kNoteG4), SIXTEENTH(kNoteF4),
    SIXTEENTH(kNoteC4), SIXTEENTH(kNoteA3), SIXTEENTH(kNoteG3),
    SIXTEENTH(kNoteG3), SIXTEENTH(kNoteA3), SIXTEENTH(kNoteB3),
    SIXTEENTH(kNoteD4), SIXTEENTH(kNoteG4), SIXTEENTH(kNoteA4),
    SIXTEENTH(kNoteB4), SIXTEENTH(kNoteD5), SIXTEENTH(kNoteG5),
    SIXTEENTH(kNoteD5), SIXTEENTH(kNoteB4), SIXTEENTH(kNoteA4),
    SIXTEENTH(kNoteG4), SIXTEENTH(kNoteD4), SIXTEENTH(kNoteB3),
    SIXTEENTH(kNoteA3),

    // Part 2: pedal-point drive
    SIXTEENTH(kNoteC3), SIXTEENTH(kNoteC4), SIXTEENTH(kNoteC3),
    SIXTEENTH(kNoteE4), SIXTEENTH(kNoteC3), SIXTEENTH(kNoteG4),
    SIXTEENTH(kNoteC3), SIXTEENTH(kNoteC5), SIXTEENTH(kNoteC3),
    SIXTEENTH(kNoteC5), SIXTEENTH(kNoteC3), SIXTEENTH(kNoteG4),
    SIXTEENTH(kNoteC3), SIXTEENTH(kNoteE4), SIXTEENTH(kNoteC3),
    SIXTEENTH(kNoteC4), SIXTEENTH(kNoteC3), SIXTEENTH(kNoteA3),
    SIXTEENTH(kNoteC3), SIXTEENTH(kNoteC4), SIXTEENTH(kNoteC3),
    SIXTEENTH(kNoteE4), SIXTEENTH(kNoteC3), SIXTEENTH(kNoteA4),
    SIXTEENTH(kNoteC3), SIXTEENTH(kNoteA4), SIXTEENTH(kNoteC3),
    SIXTEENTH(kNoteE4), SIXTEENTH(kNoteC3), SIXTEENTH(kNoteC4),
    SIXTEENTH(kNoteC3), SIXTEENTH(kNoteA3), SIXTEENTH(kNoteC3),
    SIXTEENTH(kNoteF3), SIXTEENTH(kNoteC3), SIXTEENTH(kNoteA3),
    SIXTEENTH(kNoteC3), SIXTEENTH(kNoteC4), SIXTEENTH(kNoteC3),
    SIXTEENTH(kNoteF4), SIXTEENTH(kNoteC3), SIXTEENTH(kNoteF4),
    SIXTEENTH(kNoteC3), SIXTEENTH(kNoteC4), SIXTEENTH(kNoteC3),
    SIXTEENTH(kNoteA3), SIXTEENTH(kNoteC3), SIXTEENTH(kNoteF3),
    SIXTEENTH(kNoteC3), SIXTEENTH(kNoteG3), SIXTEENTH(kNoteC3),
    SIXTEENTH(kNoteB3), SIXTEENTH(kNoteC3), SIXTEENTH(kNoteD4),
    SIXTEENTH(kNoteC3), SIXTEENTH(kNoteG4), SIXTEENTH(kNoteC3),
    SIXTEENTH(kNoteG4), SIXTEENTH(kNoteC3), SIXTEENTH(kNoteD4),
    SIXTEENTH(kNoteC3), SIXTEENTH(kNoteB3), SIXTEENTH(kNoteC3),
    SIXTEENTH(kNoteG3),

    // Part 3: octave jumps
    SIXTEENTH(kNoteC4), SIXTEENTH(kNoteC5), SIXTEENTH(kNoteG4),
    SIXTEENTH(kNoteC5), SIXTEENTH(kNoteE4), SIXTEENTH(kNoteC5),
    SIXTEENTH(kNoteG4), SIXTEENTH(kNoteC5), SIXTEENTH(kNoteC4),
    SIXTEENTH(kNoteC5), SIXTEENTH(kNoteG4), SIXTEENTH(kNoteC5),
    SIXTEENTH(kNoteE4), SIXTEENTH(kNoteC5), SIXTEENTH(kNoteG4),
    SIXTEENTH(kNoteC5), SIXTEENTH(kNoteA3), SIXTEENTH(kNoteA4),
    SIXTEENTH(kNoteE4), SIXTEENTH(kNoteA4), SIXTEENTH(kNoteC4),
    SIXTEENTH(kNoteA4), SIXTEENTH(kNoteE4), SIXTEENTH(kNoteA4),
    SIXTEENTH(kNoteA3), SIXTEENTH(kNoteA4), SIXTEENTH(kNoteE4),
    SIXTEENTH(kNoteA4), SIXTEENTH(kNoteC4), SIXTEENTH(kNoteA4),
    SIXTEENTH(kNoteE4), SIXTEENTH(kNoteA4), SIXTEENTH(kNoteF3),
    SIXTEENTH(kNoteF4), SIXTEENTH(kNoteC4), SIXTEENTH(kNoteF4),
    SIXTEENTH(kNoteA3), SIXTEENTH(kNoteF4), SIXTEENTH(kNoteC4),
    SIXTEENTH(kNoteF4), SIXTEENTH(kNoteF3), SIXTEENTH(kNoteF4),
    SIXTEENTH(kNoteC4), SIXTEENTH(kNoteF4), SIXTEENTH(kNoteA3),
    SIXTEENTH(kNoteF4), SIXTEENTH(kNoteC4), SIXTEENTH(kNoteF4),
    SIXTEENTH(kNoteG3), SIXTEENTH(kNoteG4), SIXTEENTH(kNoteD4),
    SIXTEENTH(kNoteG4), SIXTEENTH(kNoteB3), SIXTEENTH(kNoteG4),
    SIXTEENTH(kNoteD4), SIXTEENTH(kNoteG4), SIXTEENTH(kNoteG3),
    SIXTEENTH(kNoteG4), SIXTEENTH(kNoteD4), SIXTEENTH(kNoteG4),
    SIXTEENTH(kNoteB3), SIXTEENTH(kNoteG4), SIXTEENTH(kNoteD4),
    SIXTEENTH(kNoteG4)};

#undef SIXTEENTH

#define NOTE_EVENT(note, divider) \
  NoteEvent { note, divider }

static constexpr NoteEvent kTetrisTheme[] = {
    // Part 1 (A section)
    NOTE_EVENT(kNoteE5, 4),    NOTE_EVENT(kNoteB4, 8),
    NOTE_EVENT(kNoteC5, 8),    NOTE_EVENT(kNoteD5, 4),
    NOTE_EVENT(kNoteC5, 8),    NOTE_EVENT(kNoteB4, 8),
    NOTE_EVENT(kNoteA4, 4),    NOTE_EVENT(kNoteA4, 8),
    NOTE_EVENT(kNoteC5, 8),    NOTE_EVENT(kNoteE5, 4),
    NOTE_EVENT(kNoteD5, 8),    NOTE_EVENT(kNoteC5, 8),
    NOTE_EVENT(kNoteB4, 4),    NOTE_EVENT(kNoteC5, 8),
    NOTE_EVENT(kNoteD5, 4),    NOTE_EVENT(kNoteE5, 4),
    NOTE_EVENT(kNoteC5, 4),    NOTE_EVENT(kNoteA4, 4),
    NOTE_EVENT(kNoteA4, 4),    NOTE_EVENT(kNoteRest, 4),

    // Part 2 (B section)
    NOTE_EVENT(kNoteD5, 4),    NOTE_EVENT(kNoteF5, 8),
    NOTE_EVENT(kNoteA5, 4),    NOTE_EVENT(kNoteG5, 8),
    NOTE_EVENT(kNoteF5, 8),    NOTE_EVENT(kNoteE5, 4),
    NOTE_EVENT(kNoteC5, 8),    NOTE_EVENT(kNoteE5, 4),
    NOTE_EVENT(kNoteD5, 8),    NOTE_EVENT(kNoteC5, 8),
    NOTE_EVENT(kNoteB4, 4),    NOTE_EVENT(kNoteB4, 8),
    NOTE_EVENT(kNoteC5, 8),    NOTE_EVENT(kNoteD5, 4),
    NOTE_EVENT(kNoteE5, 4),    NOTE_EVENT(kNoteC5, 4),
    NOTE_EVENT(kNoteA4, 4),    NOTE_EVENT(kNoteA4, 4),
    NOTE_EVENT(kNoteRest, 4),

    // Part 3 (bridge)
    NOTE_EVENT(kNoteE4, 2),    NOTE_EVENT(kNoteC4, 2),
    NOTE_EVENT(kNoteD4, 2),    NOTE_EVENT(kNoteB3, 2),
    NOTE_EVENT(kNoteC4, 2),    NOTE_EVENT(kNoteA3, 2),
    NOTE_EVENT(kNoteGs3, 2),   NOTE_EVENT(kNoteRest, 2),
    NOTE_EVENT(kNoteE4, 2),    NOTE_EVENT(kNoteC4, 2),
    NOTE_EVENT(kNoteD4, 2),    NOTE_EVENT(kNoteB3, 2),
    NOTE_EVENT(kNoteC4, 4),    NOTE_EVENT(kNoteE4, 4),
    NOTE_EVENT(kNoteA4, 2),    NOTE_EVENT(kNoteGs4, 2)};

#undef NOTE_EVENT

#define MENU_NOTE_EVENT(note, divider) \
  NoteEvent { note, divider }

static constexpr NoteEvent kMenuTheme[] = {
    // Phrase 1: main motif
    MENU_NOTE_EVENT(kNoteB3, 4),  MENU_NOTE_EVENT(kNoteE4, 4),
    MENU_NOTE_EVENT(kNoteGs4, 2), MENU_NOTE_EVENT(kNoteFs4, 4),
    MENU_NOTE_EVENT(kNoteE4, 4),  MENU_NOTE_EVENT(kNoteFs4, 2),
    MENU_NOTE_EVENT(kNoteB3, 4),  MENU_NOTE_EVENT(kNoteE4, 4),
    MENU_NOTE_EVENT(kNoteGs4, 2), MENU_NOTE_EVENT(kNoteB4, 4),
    MENU_NOTE_EVENT(kNoteA4, 4),  MENU_NOTE_EVENT(kNoteGs4, 4),
    MENU_NOTE_EVENT(kNoteFs4, 4), MENU_NOTE_EVENT(kNoteE4, 1),
    MENU_NOTE_EVENT(kNoteRest, 2),

    // Phrase 2: bridge response
    MENU_NOTE_EVENT(kNoteCs4, 4), MENU_NOTE_EVENT(kNoteDs4, 4),
    MENU_NOTE_EVENT(kNoteE4, 2),  MENU_NOTE_EVENT(kNoteFs4, 2),
    MENU_NOTE_EVENT(kNoteDs4, 2), MENU_NOTE_EVENT(kNoteB3, 1),
    MENU_NOTE_EVENT(kNoteRest, 2),

    // Phrase 3: ascending variation
    MENU_NOTE_EVENT(kNoteE4, 4),  MENU_NOTE_EVENT(kNoteGs4, 4),
    MENU_NOTE_EVENT(kNoteB4, 2),  MENU_NOTE_EVENT(kNoteCs5, 2),
    MENU_NOTE_EVENT(kNoteB4, 2),  MENU_NOTE_EVENT(kNoteA4, 4),
    MENU_NOTE_EVENT(kNoteGs4, 4), MENU_NOTE_EVENT(kNoteFs4, 2),
    MENU_NOTE_EVENT(kNoteE4, 1),  MENU_NOTE_EVENT(kNoteRest, 4),

    // Phrase 4: descending resolution
    MENU_NOTE_EVENT(kNoteGs4, 2), MENU_NOTE_EVENT(kNoteFs4, 2),
    MENU_NOTE_EVENT(kNoteE4, 2),  MENU_NOTE_EVENT(kNoteDs4, 2),
    MENU_NOTE_EVENT(kNoteCs4, 1),

    // Trailing silence before restart
    MENU_NOTE_EVENT(kNoteRest, 1), MENU_NOTE_EVENT(kNoteRest, 1)};

#undef MENU_NOTE_EVENT

constexpr ThemeData kDdrThemeData = {
    kDdrTheme, sizeof(kDdrTheme) / sizeof(kDdrTheme[0]), 120, 0};

constexpr ThemeData kTetrisThemeData = {
    kTetrisTheme, sizeof(kTetrisTheme) / sizeof(kTetrisTheme[0]), 144, 700};

constexpr ThemeData kMenuThemeData = {
  kMenuTheme, sizeof(kMenuTheme) / sizeof(kMenuTheme[0]), 75, 2000};

PlaybackState playbackState;

const ThemeData* themeForGame(GameSelection game) {
  switch (game) {
    case GameSelection::TETRIS:
      return &kTetrisThemeData;
    case GameSelection::DDR_GAME:
      return &kDdrThemeData;
  }

  return nullptr;
}

unsigned long durationForEvent(const ThemeData& theme, const NoteEvent& event) {
  const unsigned long wholeNoteMs = (60000UL * 4UL) / theme.tempo;
  const int divider = event.divider;

  if (divider > 0) {
    return wholeNoteMs / static_cast<unsigned long>(divider);
  }

  return (wholeNoteMs * 3UL) / (static_cast<unsigned long>(-divider) * 2UL);
}

void startCurrentNote(unsigned long nowMs) {
  if (!playbackState.playing || playbackState.theme == nullptr) {
    return;
  }

  if (playbackState.noteIndex >= playbackState.theme->noteCount) {
    if (playbackState.theme->loopGapMs > 0) {
      noTone(kBuzzerPin);
      playbackState.noteIndex = 0;
      playbackState.noteStartedMs = nowMs;
      playbackState.noteLengthMs = playbackState.theme->loopGapMs;
      return;
    }

    playbackState.noteIndex = 0;
  }

  const NoteEvent& event = playbackState.theme->notes[playbackState.noteIndex++];
  const unsigned long noteDurationMs =
      max(1UL, durationForEvent(*playbackState.theme, event));
  const unsigned long toneDurationMs = max(1UL, (noteDurationMs * 9UL) / 10UL);

  if (event.frequency == kNoteRest) {
    noTone(kBuzzerPin);
  } else {
    tone(kBuzzerPin, event.frequency, toneDurationMs);
  }

  playbackState.noteStartedMs = nowMs;
  playbackState.noteLengthMs = noteDurationMs;
}

void ensureInitialized() {
  if (playbackState.initialized) {
    return;
  }

  pinMode(kBuzzerPin, OUTPUT);
  noTone(kBuzzerPin);
  playbackState.initialized = true;
}

}  // namespace

namespace GameMusic {

void init() { ensureInitialized(); }

void playMenuMusic() {
  ensureInitialized();

  if (playbackState.playing && playbackState.theme == &kMenuThemeData) {
    return;
  }

  playbackState.theme = &kMenuThemeData;
  playbackState.noteIndex = 0;
  playbackState.playing = true;
  startCurrentNote(millis());
}

void playForGame(GameSelection game) {
  ensureInitialized();
  const ThemeData* selectedTheme = themeForGame(game);
  if (selectedTheme == nullptr) {
    stop();
    return;
  }

  if (playbackState.playing && playbackState.theme == selectedTheme) {
    return;
  }

  playbackState.theme = selectedTheme;
  playbackState.noteIndex = 0;
  playbackState.playing = true;

  startCurrentNote(millis());
}

void update() {
  if (!playbackState.playing || playbackState.theme == nullptr) {
    return;
  }

  const unsigned long nowMs = millis();
  if (nowMs - playbackState.noteStartedMs < playbackState.noteLengthMs) {
    return;
  }

  startCurrentNote(nowMs);
}

void stop() {
  if (playbackState.initialized) {
    noTone(kBuzzerPin);
  }

  playbackState.theme = nullptr;
  playbackState.noteIndex = 0;
  playbackState.noteStartedMs = 0;
  playbackState.noteLengthMs = 0;
  playbackState.playing = false;
}

}  // namespace GameMusic
