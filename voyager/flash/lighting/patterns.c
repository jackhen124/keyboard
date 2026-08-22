// ── Patterns: brightness ──────────────────────────────────────────────────────
// Each pattern answers one question per LED per frame: how bright should this key
// be right now, 0..255? Hue is somebody else's job (colors.c). The two combine in
// render.c. All patterns depend on QMK's keypress tracker (g_last_hit_tracker),
// so with RGB_MATRIX_KEYREACTIVE_ENABLED off they degrade to "no reaction".

#include "lighting_internal.h"

// ── Geometry helpers (private) ────────────────────────────────────────────────

// Cheap integer distance (octagonal approximation, ~max + 0.42*min). Good enough
// for lighting and avoids pulling in floating-point sqrt.
static uint16_t approx_dist(int16_t dx, int16_t dy) {
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    int16_t mn = dx < dy ? dx : dy;
    int16_t mx = dx < dy ? dy : dx;
    return (uint16_t)mx + (uint16_t)((mn * 27) >> 6);
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

// ── PAT_REACTIVE: round keypress ripple ───────────────────────────────────────
// How "hot" a given LED is right now (0 = at rest, 255 = peak of the wave), taken
// as the max over every recent hit's expanding ripple. Returns 0 when keypress
// tracking is compiled out.
static uint8_t splash_factor(uint8_t led_index) {
#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
    uint8_t lx = g_led_config.point[led_index].x;
    uint8_t ly = g_led_config.point[led_index].y;
    uint8_t best = 0;
    for (uint8_t i = 0; i < g_last_hit_tracker.count; i++) {
        int16_t  dx = (int16_t)lx - (int16_t)g_last_hit_tracker.x[i];
        int16_t  dy = (int16_t)ly - (int16_t)g_last_hit_tracker.y[i];
        uint16_t d  = approx_dist(dx, dy);
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

// ── PAT_CROSS: row + column flash ─────────────────────────────────────────────
// A keypress instantly lights the pressed key's entire row and column at once —
// no travelling wavefront, no distance falloff — and the whole cross then fades
// out together over RGB_CROSS_FADE_MS.
//
// Matching is done on the MATRIX row/column of the pressed key, NOT on physical
// x/y coordinates. The Voyager's columns are vertically staggered, so a physical
// "same-y" test with any usable tolerance bleeds a press into the neighbouring
// physical row (e.g. pressing O lit several top-row keys). Matrix row/col is the
// logical grid and is stagger-proof. The same-half gate keeps a press from
// throwing a bar of light across onto the other hand. (row, col) is the LED
// currently being painted. Returns the fade level (0 = at rest, 255 = pressed).
static uint8_t cross_factor(uint8_t led_index, uint8_t row, uint8_t col) {
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

// ── Dispatch ──────────────────────────────────────────────────────────────────
uint8_t lighting_pattern_brightness(uint8_t pattern, uint8_t led_index,
                                    uint8_t row, uint8_t col) {
    switch (pattern) {
        case PAT_CROSS:
            return cross_factor(led_index, row, col);
        case PAT_REACTIVE: {
            uint8_t f = splash_factor(led_index);               // 0 idle .. 255 peak
            return RGB_IDLE_VAL + (uint8_t)(((255 - RGB_IDLE_VAL) * (uint16_t)f) / 255);
        }
        case PAT_SOLID:
            return RGB_IDLE_VAL;
        case PAT_OFF:
        default:
            return 0;
    }
}
