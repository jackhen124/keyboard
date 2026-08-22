// ── Colors: hue ───────────────────────────────────────────────────────────────
// The palette half of the system. Given an LED, return its FULL-brightness color
// under the active color mode; the pattern brightness (patterns.c) scales it down
// later in render.c. Colors care only about *where* a key is, never about *when*
// it was pressed — that is the pattern's job.

#include "lighting_internal.h"

void lighting_color_for(uint8_t led_index, uint8_t *r, uint8_t *g, uint8_t *b) {
    if (lighting_get_color() == COLOR_GRADIENT) {
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
