// ── Lighting controller ───────────────────────────────────────────────────────
// The small, boring core of the lighting module: it holds which pattern and
// color are currently selected, boots the RGB matrix into a clean base state,
// and routes the top-row selector keys to those two variables. It knows nothing
// about how anything is drawn — patterns.c, colors.c and render.c handle that.

#include "lighting.h"
#include "lighting_internal.h"

static uint8_t g_pattern = PAT_REACTIVE;  // boot defaults
static uint8_t g_color   = COLOR_WHITE;

uint8_t lighting_get_pattern(void) { return g_pattern; }
uint8_t lighting_get_color(void)   { return g_color; }

void lighting_init(void) {
    // Start from a clean, static base so no saved animation shows through the
    // per-frame painting in render.c. noeeprom = don't overwrite saved prefs.
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
