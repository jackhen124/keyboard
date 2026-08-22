#pragma once
// Internal contract shared across the lighting module's .c files. The PUBLIC
// interface — the selector keycodes plus lighting_init() / lighting_process_record()
// — lives in lighting.h; keymap.c only ever includes that. Everything here is the
// module talking to itself: tunables, the mode vocabulary, and the handful of
// functions the render loop calls into.

#include QMK_KEYBOARD_H

// ── Tunables ──────────────────────────────────────────────────────────────────
// Every lighting magic number, in one place to make tweaking painless.
#define RGB_IDLE_VAL        40   // resting brightness 0-255. Lower = darker.
#define RGB_SPLASH_RADIUS   42   // ripple reach in QMK point units (~2 key-widths)
#define RGB_SPLASH_MS       320  // time for the wavefront to travel one RGB_SPLASH_RADIUS
#define RGB_REACT_MS        400  // how long each key stays lit after the wave passes it
#define RGB_CROSS_FADE_MS   260  // time for a lit cross to fade to black after a press
#define RGB_MATRIX_MAX_X    224  // QMK scales key x-coords to 0..224 across the board
#define LIGHTING_BASE_LAYER 0    // layer index whose top row hosts the selectors

// ── Modes ─────────────────────────────────────────────────────────────────────
// The two independent axes. PATTERN decides each key's BRIGHTNESS (idle glow,
// ripple, cross flash, …); COLOR decides each key's HUE. They are chosen
// separately and multiply at paint time, so any pattern can run in any color.
enum lighting_pattern {
    PAT_REACTIVE = 0,  // dim idle glow + round keypress ripple
    PAT_CROSS,         // dark idle + a cross (matrix row + column) flash on press
    PAT_SOLID,         // steady idle glow, no keypress reaction
    PAT_OFF,           // everything dark
    PAT_COUNT,         // number of real patterns; higher selector keys are ignored
};

enum lighting_color {
    COLOR_WHITE = 0,   // plain white
    COLOR_GRADIENT,    // blue -> green -> yellow across x
    COLOR_COUNT,       // number of real colors; higher selector keys are ignored
};

// ── Current selection (owned by lighting.c) ───────────────────────────────────
uint8_t lighting_get_pattern(void);
uint8_t lighting_get_color(void);

// ── Patterns (patterns.c) ─────────────────────────────────────────────────────
// Peak brightness 0..255 for one LED this frame under `pattern`. (row, col) is
// the LED's matrix position; grid-based patterns (cross) match on it.
uint8_t lighting_pattern_brightness(uint8_t pattern, uint8_t led_index,
                                    uint8_t row, uint8_t col);

// ── Colors (colors.c) ─────────────────────────────────────────────────────────
// Full-brightness RGB for one LED under the active color mode. The pattern
// brightness later scales this down.
void lighting_color_for(uint8_t led_index, uint8_t *r, uint8_t *g, uint8_t *b);
