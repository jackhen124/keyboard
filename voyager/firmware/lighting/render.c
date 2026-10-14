// ── Render: the per-frame paint loop ──────────────────────────────────────────
// This is where the two axes meet. For every LED, each frame, we ask patterns.c
// for a brightness and colors.c for a hue and multiply them. It also owns the two
// pieces of UI that aren't really "a pattern": the top-row selector indicators and
// the cheat-sheet gating (dark keys on higher layers).
//
// rgb_matrix_indicators_advanced_user() is a weak QMK hook; defining it here fully
// overwrites the underlying SOLID_COLOR mode, so no stock animation shows through.

#include "lighting.h"
#include "lighting_internal.h"

// Scale one 0..255 channel by a 0..255 brightness factor.
static inline uint8_t scale8(uint8_t channel, uint8_t factor) {
    return (uint8_t)((uint16_t)channel * factor / 255);
}

// Paint one LED as `color` dimmed to `v`.
static inline void paint(uint8_t index, uint8_t v) {
    uint8_t r, g, b;
    lighting_color_for(index, &r, &g, &b);
    rgb_matrix_set_color(index, scale8(r, v), scale8(g, v), scale8(b, v));
}

// Whether a key should light at all on the active layer. On layers above base,
// keys that do nothing (transparent / KC_NO) stay dark — a cheat-sheet view.
static bool key_active(uint8_t layer, uint8_t row, uint8_t col) {
    if (layer == LIGHTING_BASE_LAYER) return true;
    uint16_t kc = keymap_key_to_keycode(layer, (keypos_t){.row = row, .col = col});
    return !(kc == KC_TRANSPARENT || kc == KC_NO);
}

// The base-layer top row is the selector strip. Draw one of its keys and return
// true; return false if (row, col) is not a selector so the caller paints it
// normally. The active pattern key (left half) and active color key (right half)
// rest at a dim glow; the rest of the row is dark, so the row always shows the
// current selection. In the CROSS pattern the cross still sweeps this row like any
// other — when its fade is brighter than the resting indicator, the cross wins;
// otherwise the indicator floor shows through so the row stays a readable switch.
static bool draw_selector(uint8_t layer, uint8_t index, uint8_t row, uint8_t col) {
    if (layer != LIGHTING_BASE_LAYER) return false;

    uint16_t bkc = keymap_key_to_keycode(LIGHTING_BASE_LAYER,
                                         (keypos_t){.row = row, .col = col});
    bool is_pat = (bkc >= LM_PAT_1 && bkc <= LM_PAT_6);
    bool is_col = (bkc >= LM_COL_1 && bkc <= LM_COL_6);
    if (!is_pat && !is_col) return false;

    uint8_t sel_v = 0;  // resting indicator: dim white iff this is the active selection
    if (is_pat && (bkc - LM_PAT_1) == lighting_get_pattern()) sel_v = RGB_IDLE_VAL;
    if (is_col && (bkc - LM_COL_1) == lighting_get_color())   sel_v = RGB_IDLE_VAL;

    if (lighting_get_pattern() == PAT_CROSS) {
        uint8_t f = lighting_pattern_brightness(PAT_CROSS, index);
        if (f > sel_v) {
            paint(index, f);        // the cross sweep, in the active color
            return true;
        }
    }
    rgb_matrix_set_color(index, sel_v, sel_v, sel_v);
    return true;
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t layer   = get_highest_layer(layer_state);
    uint8_t pattern = lighting_get_pattern();

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            uint8_t index = g_led_config.matrix_co[row][col];
            if (index == NO_LED || index < led_min || index >= led_max) continue;

            if (draw_selector(layer, index, row, col)) continue;

            // PAT_AREA deliberately ignores a key's function: its neighbourhood
            // glow lights unmapped keys too, so it skips the cheat-sheet gating.
            bool gated = pattern != PAT_AREA && !key_active(layer, row, col);
            if (pattern == PAT_OFF || gated) {
                rgb_matrix_set_color(index, 0, 0, 0);
                continue;
            }

            // Brightness from the pattern, hue from the color — combined in paint().
            paint(index, lighting_pattern_brightness(pattern, index));
        }
    }
    return false;
}
