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
#define RGB_SPLASH_RADIUS   90   // ripple reach in QMK point units (~4-5 key-widths)
#define RGB_SPLASH_MS       180  // time for the wavefront to travel one RGB_SPLASH_RADIUS
#define RGB_REACT_MS        400  // how long each key stays lit after the wave passes it
#define RGB_CROSS_FADE_MS   260  // time for a lit cross to fade to black after a press
#define RGB_FADE_MS         3600 // time for a single pressed key to slowly fade to black
#define RGB_AREA_MS         3600 // time for an area-fade press to fade to black
#define RGB_AREA_RADIUS     3    // area-fade reach in keys (grid steps from the press)
#define RGB_MATRIX_MAX_X    224  // QMK scales key x-coords to 0..224 across the board
#define LIGHTING_BASE_LAYER 0    // layer index whose top row hosts the selectors

// ── Modes ─────────────────────────────────────────────────────────────────────
// The two independent axes. PATTERN decides each key's BRIGHTNESS (idle glow,
// ripple, cross flash, …); COLOR decides each key's HUE. They are chosen
// separately and multiply at paint time, so any pattern can run in any color.
enum lighting_pattern {
    PAT_REACTIVE = 0,  // dark idle + round keypress ripple
    PAT_CROSS,         // dark idle + a cross (matrix row + column) flash on press
    PAT_SOLID,         // steady idle glow, no keypress reaction
    PAT_AREA,          // dark idle + a long fade over the keys around the press
    PAT_FADE,          // dark idle + only the pressed key lights, then slowly fades
    PAT_OFF,           // everything dark
    PAT_COUNT,         // number of real patterns; higher selector keys are ignored
};

enum lighting_color {
    COLOR_WHITE = 0,   // plain white
    COLOR_GRADIENT,    // blue -> green -> yellow across x
    COLOR_SUNSET,      // yellow -> orange -> red across x
    COLOR_COUNT,       // number of real colors; higher selector keys are ignored
};

// ── Current selection (owned by lighting.c) ───────────────────────────────────
uint8_t lighting_get_pattern(void);
uint8_t lighting_get_color(void);

// ── Patterns (patterns.c) ─────────────────────────────────────────────────────
// Peak brightness 0..255 for one LED this frame under `pattern`. Grid-based
// patterns (cross) derive the LED's row/column from its physical RGB coordinates.
uint8_t lighting_pattern_brightness(uint8_t pattern, uint8_t led_index);

// ── Colors (colors.c) ─────────────────────────────────────────────────────────
// Full-brightness RGB for one LED under the active color mode. The pattern
// brightness later scales this down.
void lighting_color_for(uint8_t led_index, uint8_t *r, uint8_t *g, uint8_t *b);
