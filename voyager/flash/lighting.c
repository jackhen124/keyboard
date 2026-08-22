// ── Voyager lighting ─────────────────────────────────────────────────────────
// Action-reactive RGB, kept separate from the keymap/keybinds (keymap.c). The
// base-layer number row acts as a lighting-mode selector (LM_MODE_* in
// lighting.h): the active mode's key rests dim white, every other selector key
// is dark, so the top row always shows — and lets you switch — the mode.
//
// Modes, mapped in order to the top-row keys (LM_MODE_1 = top-left "=" key):
//   1  MODE_CROSS     dark idle; a keypress instantly lights that key's whole
//                     row + column (a cross/plus shape), which then fades out
//   2  MODE_REACTIVE  dim-white idle + per-layer theme colors + keypress ripple
//   3  MODE_SOLID     the same idle/theme colors, held steady (no keypress FX)
//   4  MODE_OFF       all keys dark (the selector row still shows the indicator)
//   5..12             unused for now — selecting them is ignored, keys stay dark
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

// Cross mode: a keypress instantly lights that key's whole row + column (the
// full length of each axis, no travelling wavefront), which then fades out.
#define RGB_CROSS_TOL      8     // max off-axis distance to count as "same row/column"
#define RGB_CROSS_FADE_MS  500   // time for the lit cross to fade to black after a press

#define LIGHTING_BASE_LAYER 0    // layer index whose number row hosts the selectors

// Per-layer "peak" (full-brightness) color: a layer's active keys rest at this
// hue, just dimmed, and flash to it on a keypress. Indexed by layer number.
static const uint8_t layer_theme[][3] = {
    {255, 255, 255},  // 0 _BASE   white
    {  0, 255, 255},  // 1 _SYM    cyan
    {  0, 255,   0},  // 2 _NUM    green
    {160,   0, 255},  // 3 _NAV    purple
    {255,  90,   0},  // 4 _MOUSE  orange
};
#define LAYER_THEME_COUNT (sizeof(layer_theme) / sizeof(layer_theme[0]))

// ── Modes ────────────────────────────────────────────────────────────────────
enum lighting_mode {
    MODE_CROSS = 0,    // top-left "=" key: dark idle, white cross-shaped ripple
    MODE_REACTIVE,     // dim-white idle + layer themes + round keypress ripple
    MODE_SOLID,        // steady idle/theme colors, no keypress reaction
    MODE_OFF,          // everything dark
    MODE_COUNT,
};

static uint8_t g_lighting_mode = MODE_REACTIVE;

void lighting_init(void) {
    // Start from a clean, static base so no saved rainbow animation shows through
    // the per-frame painting below. noeeprom = doesn't overwrite saved prefs.
    rgb_matrix_enable_noeeprom();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
}

bool lighting_process_record(uint16_t keycode, keyrecord_t *record) {
    if (keycode < LM_MODE_1 || keycode > LM_MODE_12) return true;  // not a lighting key
    if (record->event.pressed) {
        uint8_t sel = keycode - LM_MODE_1;         // 0-based mode index
        if (sel < MODE_COUNT) g_lighting_mode = sel;
        // higher keys are reserved: ignore, staying on the current mode
    }
    return false;  // selector keys never emit a character
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

// Round splash (used by MODE_REACTIVE): how "hot" a given LED is right now
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

// Cross (used by MODE_CROSS): a keypress instantly lights the pressed key's
// entire row and column at once — no travelling wavefront, no distance falloff —
// and the whole cross then fades out together over RGB_CROSS_FADE_MS. Returns
// the fade level (0 = faded/at rest, 255 = just pressed) for this LED.
static uint8_t rgb_cross_factor(uint8_t led_index) {
#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
    uint8_t lx = g_led_config.point[led_index].x;
    uint8_t ly = g_led_config.point[led_index].y;
    uint8_t best = 0;
    for (uint8_t i = 0; i < g_last_hit_tracker.count; i++) {
        int16_t dx = (int16_t)lx - (int16_t)g_last_hit_tracker.x[i];
        int16_t dy = (int16_t)ly - (int16_t)g_last_hit_tracker.y[i];
        // On the press's column (same x) or row (same y)? Full axis, no reach cap.
        bool on_col = (dx <= RGB_CROSS_TOL && dx >= -RGB_CROSS_TOL);
        bool on_row = (dy <= RGB_CROSS_TOL && dy >= -RGB_CROSS_TOL);
        if (!on_col && !on_row) continue;

        uint16_t tick = g_last_hit_tracker.tick[i];             // ms since the press
        if (tick >= RGB_CROSS_FADE_MS) continue;                // fully faded
        uint8_t f = 255 - (uint8_t)((uint32_t)tick * 255 / RGB_CROSS_FADE_MS);
        if (f > best) best = f;
    }
    return best;
#else
    (void)led_index;
    return 0;
#endif
}

// Resting theme color for a key. Returns false when the key should be dark
// (i.e. it does nothing on the active layer above base).
static bool key_theme(uint8_t layer, uint8_t row, uint8_t col,
                      uint8_t *r, uint8_t *g, uint8_t *b) {
    if (layer != LIGHTING_BASE_LAYER) {
        uint16_t kc = keymap_key_to_keycode(layer, (keypos_t){.row = row, .col = col});
        if (kc == KC_TRANSPARENT || kc == KC_NO) return false;
    }
    if (layer >= LAYER_THEME_COUNT) layer = LIGHTING_BASE_LAYER;
    *r = layer_theme[layer][0];
    *g = layer_theme[layer][1];
    *b = layer_theme[layer][2];
    return true;
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t layer = get_highest_layer(layer_state);

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t index = g_led_config.matrix_co[row][col];
            if (index == NO_LED || index < led_min || index >= led_max) continue;

            // Mode-selector indicator: on the base layer, the number-row keys
            // show which lighting mode is active (dim white) vs. the rest (dark).
            // This wins over the effect, so the row is always a readable switch.
            if (layer == LIGHTING_BASE_LAYER) {
                uint16_t bkc = keymap_key_to_keycode(LIGHTING_BASE_LAYER,
                                                     (keypos_t){.row = row, .col = col});
                if (bkc >= LM_MODE_1 && bkc <= LM_MODE_12) {
                    uint8_t v = ((bkc - LM_MODE_1) == g_lighting_mode) ? RGB_IDLE_VAL : 0;
                    rgb_matrix_set_color(index, v, v, v);
                    continue;
                }
            }

            // CROSS: dark board; a keypress lights its whole row + column white,
            // then the cross fades out.
            if (g_lighting_mode == MODE_CROSS) {
                uint8_t f = rgb_cross_factor(index);
                rgb_matrix_set_color(index, f, f, f);
                continue;
            }

            if (g_lighting_mode == MODE_OFF) {
                rgb_matrix_set_color(index, 0, 0, 0);
                continue;
            }

            uint8_t pr, pg, pb;
            if (!key_theme(layer, row, col, &pr, &pg, &pb)) {
                rgb_matrix_set_color(index, 0, 0, 0);
                continue;
            }

            // REACTIVE ramps brightness up on a keypress; SOLID stays at rest.
            uint8_t  f = (g_lighting_mode == MODE_REACTIVE) ? rgb_splash_factor(index) : 0;
            uint16_t v = RGB_IDLE_VAL + ((255 - RGB_IDLE_VAL) * (uint16_t)f) / 255;
            rgb_matrix_set_color(index,
                (uint8_t)((uint16_t)pr * v / 255),
                (uint8_t)((uint16_t)pg * v / 255),
                (uint8_t)((uint16_t)pb * v / 255));
        }
    }
    return false;
}
