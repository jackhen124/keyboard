// ── Voyager lighting ─────────────────────────────────────────────────────────
// Action-reactive RGB, kept separate from the keymap/keybinds (keymap.c).
//
// Two INDEPENDENT axes, both chosen from the base-layer top row:
//   - PATTERN (left 6 keys, LM_PAT_*): how the board animates / reacts.
//   - COLOR   (right 6 keys, LM_COL_*): the palette that pattern is drawn in.
// They combine: the pattern decides each key's BRIGHTNESS (idle glow, ripple,
// cross flash, …) and the color decides each key's HUE. So any pattern can run
// in any color. The active pattern key and active color key rest dim white on
// the top row; the others are dark, so the row always shows — and switches —
// the current selection.
//
// Patterns, mapped in order to the LEFT top-row keys (LM_PAT_1 = top-left):
//   1  PAT_REACTIVE  dim idle glow + a round ripple that blooms on each keypress
//   2  PAT_CROSS     dark idle; a keypress instantly lights that key's whole
//                    row + column (a cross/plus shape), which then fades out
//   3  PAT_SOLID     steady idle glow, no keypress reaction
//   4  PAT_OFF       all keys dark
//   5..6             unused for now — selecting them is ignored, keys stay dark
//
// Colors, mapped in order to the RIGHT top-row keys (LM_COL_1 = leftmost of the
// right half):
//   1  COLOR_WHITE     plain white
//   2  COLOR_GRADIENT  a blue→green→yellow gradient across the board (by x)
//   3..6               unused for now — selecting them is ignored
//
// Everything is painted per frame in rgb_matrix_indicators_advanced_user(),
// which fully overwrites the underlying SOLID_COLOR mode, so no stock animation
// (the old "rainbow") ever shows through.

#include "lighting.h"

// ── Tunables ────────────────────────────────────────────────────────────────
#define RGB_IDLE_VAL       40    // resting brightness 0-255. Lower = darker.
#define RGB_SPLASH_RADIUS  42    // ripple reach in QMK point units (~2 key-widths)
#define RGB_SPLASH_MS      320   // time for the wavefront to travel one RGB_SPLASH_RADIUS
#define RGB_REACT_MS       400   // how long each key stays lit after the wave passes it

// Cross pattern: a keypress instantly lights that key's whole matrix row +
// column (the full length of each axis, no travelling wavefront), which then
// fades out. Matched on the logical matrix grid, so column stagger can't smear
// the row arm into a neighbouring physical row.
#define RGB_CROSS_FADE_MS  260   // time for the lit cross to fade to black after a press

#define RGB_MATRIX_MAX_X   224   // QMK scales key x-coords to 0..224 across the board

#define LIGHTING_BASE_LAYER 0    // layer index whose top row hosts the selectors

// ── Pattern / color state ─────────────────────────────────────────────────────
enum lighting_pattern {
    PAT_REACTIVE = 0,  // dim idle glow + round keypress ripple
    PAT_CROSS,         // dark idle + white/colored cross flash on press
    PAT_SOLID,         // steady idle glow, no reaction
    PAT_OFF,           // everything dark
    PAT_COUNT,
};

enum lighting_color {
    COLOR_WHITE = 0,   // plain white
    COLOR_GRADIENT,    // blue -> green -> yellow across x
    COLOR_COUNT,
};

static uint8_t g_pattern = PAT_REACTIVE;  // boot defaults
static uint8_t g_color   = COLOR_WHITE;

void lighting_init(void) {
    // Start from a clean, static base so no saved rainbow animation shows through
    // the per-frame painting below. noeeprom = doesn't overwrite saved prefs.
    rgb_matrix_enable_noeeprom();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
}

bool lighting_process_record(uint16_t keycode, keyrecord_t *record) {
    if (keycode >= LM_PAT_1 && keycode <= LM_PAT_6) {         // left half: pattern
        if (record->event.pressed) {
            uint8_t sel = keycode - LM_PAT_1;
            if (sel < PAT_COUNT) g_pattern = sel;             // reserved keys ignored
        }
        return false;                                         // never emits a character
    }
    if (keycode >= LM_COL_1 && keycode <= LM_COL_6) {         // right half: color
        if (record->event.pressed) {
            uint8_t sel = keycode - LM_COL_1;
            if (sel < COLOR_COUNT) g_color = sel;             // reserved keys ignored
        }
        return false;
    }
    return true;                                              // not a lighting key
}

// ── Keypress ripple ───────────────────────────────────────────────────────────
// Cheap integer distance (octagonal approximation, ~max + 0.42*min). Good enough
// for lighting and avoids pulling in floating-point sqrt.
static uint16_t rgb_approx_dist(int16_t dx, int16_t dy) {
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    int16_t mn = dx < dy ? dx : dy;
    int16_t mx = dx < dy ? dy : dx;
    return (uint16_t)mx + (uint16_t)((mn * 27) >> 6);
}

// Round splash (used by PAT_REACTIVE): how "hot" a given LED is right now
// (0 = at rest, 255 = peak of the wave), taken as the max over every recent
// hit's expanding ripple. Returns 0 when keypress tracking is compiled out.
static uint8_t rgb_splash_factor(uint8_t led_index) {
#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
    uint8_t lx = g_led_config.point[led_index].x;
    uint8_t ly = g_led_config.point[led_index].y;
    uint8_t best = 0;
    for (uint8_t i = 0; i < g_last_hit_tracker.count; i++) {
        int16_t  dx = (int16_t)lx - (int16_t)g_last_hit_tracker.x[i];
        int16_t  dy = (int16_t)ly - (int16_t)g_last_hit_tracker.y[i];
        uint16_t d  = rgb_approx_dist(dx, dy);
        if (d > RGB_SPLASH_RADIUS) continue;                     // outside the pulse

        uint16_t tick   = g_last_hit_tracker.tick[i];           // ms since the press
        uint16_t arrive = (uint16_t)((uint32_t)d * RGB_SPLASH_MS / RGB_SPLASH_RADIUS);
        if (tick < arrive) continue;                            // wavefront not here yet
        uint16_t age = tick - arrive;
        if (age >= RGB_REACT_MS) continue;                      // already faded out

        uint16_t tf = 255 - (uint16_t)((uint32_t)age * 255 / RGB_REACT_MS);   // time fade
        uint16_t df = 255 - (uint16_t)((uint32_t)d * 150 / RGB_SPLASH_RADIUS); // far keys dimmer
        uint8_t  f  = (uint8_t)(((uint32_t)tf * df) / 255);
        if (f > best) best = f;
    }
    return best;
#else
    (void)led_index;
    return 0;
#endif
}

// Find the matrix (row, col) that drives a given LED. Returns false if none.
static bool led_to_matrix(uint8_t led_index, uint8_t *out_row, uint8_t *out_col) {
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            if (g_led_config.matrix_co[r][c] == led_index) {
                *out_row = r;
                *out_col = c;
                return true;
            }
        }
    }
    return false;
}

// Cross (used by PAT_CROSS): a keypress instantly lights the pressed key's
// entire row and column at once — no travelling wavefront, no distance falloff —
// and the whole cross then fades out together over RGB_CROSS_FADE_MS.
//
// Matching is done on the MATRIX row/column of the pressed key, NOT on physical
// x/y coordinates. The Voyager's columns are vertically staggered, so a physical
// "same-y" test with any usable tolerance bleeds a press into the neighbouring
// physical row (e.g. pressing O lit several top-row keys). Matrix row/col is the
// logical grid and is stagger-proof. The same-half gate keeps a press from
// throwing a bar of light across onto the other hand. (row, col) is the LED
// currently being painted. Returns the fade level (0 = at rest, 255 = pressed).
static uint8_t rgb_cross_factor(uint8_t led_index, uint8_t row, uint8_t col) {
#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
    uint8_t lx  = g_led_config.point[led_index].x;
    uint8_t mid = RGB_MATRIX_MAX_X / 2;                          // split between the halves
    uint8_t best = 0;
    for (uint8_t i = 0; i < g_last_hit_tracker.count; i++) {
        uint8_t hidx = g_last_hit_tracker.index[i];
        uint8_t hrow, hcol;
        if (!led_to_matrix(hidx, &hrow, &hcol)) continue;

        bool same_half = ((lx < mid) == (g_led_config.point[hidx].x < mid));
        if (!same_half) continue;                               // never cross to the other hand
        bool on_col = (col == hcol);                            // same matrix column
        bool on_row = (row == hrow);                            // same matrix row
        if (!on_col && !on_row) continue;

        uint16_t tick = g_last_hit_tracker.tick[i];             // ms since the press
        if (tick >= RGB_CROSS_FADE_MS) continue;                // fully faded
        uint8_t f = 255 - (uint8_t)((uint32_t)tick * 255 / RGB_CROSS_FADE_MS);
        if (f > best) best = f;
    }
    return best;
#else
    (void)led_index; (void)row; (void)col;
    return 0;
#endif
}

// ── Color ─────────────────────────────────────────────────────────────────────
// Peak (full-brightness) color for a key under the active color mode. The
// pattern later scales this by a brightness factor.
static void color_for(uint8_t led_index, uint8_t *r, uint8_t *g, uint8_t *b) {
    if (g_color == COLOR_GRADIENT) {
        // Left half of the board: blue -> green. Right half: green -> yellow.
        uint8_t x   = g_led_config.point[led_index].x;          // 0..RGB_MATRIX_MAX_X
        uint8_t mid = RGB_MATRIX_MAX_X / 2;
        if (x < mid) {
            uint8_t t = (uint8_t)((uint16_t)x * 255 / mid);     // 0..255
            *r = 0;
            *g = t;
            *b = 255 - t;
        } else {
            uint8_t t = (uint8_t)((uint16_t)(x - mid) * 255 / (RGB_MATRIX_MAX_X - mid));
            *r = t;
            *g = 255;
            *b = 0;
        }
        return;
    }
    // COLOR_WHITE (default)
    *r = 255;
    *g = 255;
    *b = 255;
}

// Whether a key should light at all on the active layer. On layers above base,
// keys that do nothing (transparent / KC_NO) stay dark — a cheat-sheet view.
static bool key_active(uint8_t layer, uint8_t row, uint8_t col) {
    if (layer == LIGHTING_BASE_LAYER) return true;
    uint16_t kc = keymap_key_to_keycode(layer, (keypos_t){.row = row, .col = col});
    return !(kc == KC_TRANSPARENT || kc == KC_NO);
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t layer = get_highest_layer(layer_state);

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t index = g_led_config.matrix_co[row][col];
            if (index == NO_LED || index < led_min || index >= led_max) continue;

            // Selector indicators: on the base layer the top row shows the active
            // pattern (left half) and active color (right half) as dim white; the
            // rest of the row is dark. In the CROSS pattern the cross still sweeps
            // this row like any other — when its fade is brighter than the resting
            // indicator, the cross wins; otherwise the indicator floor shows through
            // so the row stays a readable switch.
            if (layer == LIGHTING_BASE_LAYER) {
                uint16_t bkc = keymap_key_to_keycode(LIGHTING_BASE_LAYER,
                                                     (keypos_t){.row = row, .col = col});
                bool is_pat = (bkc >= LM_PAT_1 && bkc <= LM_PAT_6);
                bool is_col = (bkc >= LM_COL_1 && bkc <= LM_COL_6);
                if (is_pat || is_col) {
                    uint8_t sel_v = 0;  // resting indicator: dim white iff this is the active selection
                    if (is_pat && (bkc - LM_PAT_1) == g_pattern) sel_v = RGB_IDLE_VAL;
                    if (is_col && (bkc - LM_COL_1) == g_color)   sel_v = RGB_IDLE_VAL;

                    if (g_pattern == PAT_CROSS) {
                        uint8_t f = rgb_cross_factor(index, row, col);
                        if (f > sel_v) {
                            uint8_t cr, cg, cb;
                            color_for(index, &cr, &cg, &cb);
                            rgb_matrix_set_color(index,
                                (uint8_t)((uint16_t)cr * f / 255),
                                (uint8_t)((uint16_t)cg * f / 255),
                                (uint8_t)((uint16_t)cb * f / 255));
                            continue;
                        }
                    }
                    rgb_matrix_set_color(index, sel_v, sel_v, sel_v);
                    continue;
                }
            }

            if (g_pattern == PAT_OFF || !key_active(layer, row, col)) {
                rgb_matrix_set_color(index, 0, 0, 0);
                continue;
            }

            // Hue from the color mode …
            uint8_t cr, cg, cb;
            color_for(index, &cr, &cg, &cb);

            // … brightness from the pattern mode.
            uint16_t v;
            if (g_pattern == PAT_CROSS) {
                v = rgb_cross_factor(index, row, col);          // 0 idle (dark) .. 255 on press
            } else if (g_pattern == PAT_REACTIVE) {
                uint8_t f = rgb_splash_factor(index);
                v = RGB_IDLE_VAL + ((255 - RGB_IDLE_VAL) * (uint16_t)f) / 255;
            } else {                                            // PAT_SOLID
                v = RGB_IDLE_VAL;
            }

            rgb_matrix_set_color(index,
                (uint8_t)((uint16_t)cr * v / 255),
                (uint8_t)((uint16_t)cg * v / 255),
                (uint8_t)((uint16_t)cb * v / 255));
        }
    }
    return false;
}
