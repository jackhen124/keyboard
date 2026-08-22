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

// Logical grid position from an LED's PHYSICAL point, not its matrix wiring.
// The Voyager's matrix is not a clean grid — the bottom row's inner key sits on
// its own matrix row and the thumbs share matrix columns with the alpha keys — so
// matching on matrix (row, col) lights the wrong keys. The RGB point coordinates
// (0..224) are a true physical grid instead: within a row keys curve up by only
// ~5 units, but rows sit ~11 apart and columns ~17 apart, so integer-bucketing the
// raw coordinate drops every key into exactly one row/col with no stagger bleed.
// Rows 0..3 are the alpha rows; anything at row >= 4 (y >= ~53) is a thumb.
static inline uint8_t point_row(uint8_t led_index) {
    return (uint8_t)(g_led_config.point[led_index].y / 11);
}
static inline uint8_t point_col(uint8_t led_index) {
    return (uint8_t)(((uint16_t)g_led_config.point[led_index].x + 8) / 17);
}
static inline bool is_thumb(uint8_t led_index) {
    return point_row(led_index) >= 4;
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
// Matching is done on the physical-coordinate grid (point_row / point_col), NOT on
// the raw matrix wiring: the Voyager's matrix scatters the bottom row and thumbs
// across rows/columns that don't line up visually, so a matrix match lit the wrong
// keys (missed the inner bottom key, lit a thumb on the same matrix column). The
// same-half gate keeps a press from throwing a bar of light onto the other hand.
// Thumbs are excluded entirely (a thumb neither casts a cross nor catches one), so
// an alpha press can never bleed onto a thumb. `led_index` is the LED being
// painted. Returns the fade level (0 = at rest, 255 = just pressed).
static uint8_t cross_factor(uint8_t led_index) {
#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
    if (is_thumb(led_index)) return 0;                          // thumbs sit out the cross
    uint8_t lx   = g_led_config.point[led_index].x;
    uint8_t mid  = RGB_MATRIX_MAX_X / 2;                        // split between the halves
    uint8_t lrow = point_row(led_index);
    uint8_t lcol = point_col(led_index);
    uint8_t best = 0;
    for (uint8_t i = 0; i < g_last_hit_tracker.count; i++) {
        uint8_t hidx = g_last_hit_tracker.index[i];
        if (is_thumb(hidx)) continue;                           // a thumb press casts no cross

        bool same_half = ((lx < mid) == (g_led_config.point[hidx].x < mid));
        if (!same_half) continue;                               // never cross to the other hand
        if (lrow != point_row(hidx) && lcol != point_col(hidx)) continue;

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

// ── PAT_FADE: single-key slow fade ────────────────────────────────────────────
// Everything is dark at rest. A keypress lights ONLY that key (matched on its own
// LED index — no row/column, no ripple to neighbours), which then eases back down
// to black over the long RGB_FADE_MS. Returns the fade level (0 = at rest,
// 255 = just pressed).
static uint8_t fade_factor(uint8_t led_index) {
#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
    uint8_t best = 0;
    for (uint8_t i = 0; i < g_last_hit_tracker.count; i++) {
        if (g_last_hit_tracker.index[i] != led_index) continue;  // only the pressed key
        uint16_t tick = g_last_hit_tracker.tick[i];              // ms since the press
        if (tick >= RGB_FADE_MS) continue;                       // fully faded
        uint8_t f = 255 - (uint8_t)((uint32_t)tick * 255 / RGB_FADE_MS);
        if (f > best) best = f;
    }
    return best;
#else
    (void)led_index;
    return 0;
#endif
}

// ── PAT_AREA: area fade around the press ──────────────────────────────────────
// A slow fade like PAT_FADE, but a single press lights a whole neighbourhood: the
// pressed key at full opacity and every key up to RGB_AREA_RADIUS grid steps away
// at progressively lower opacity, all easing to black together over RGB_AREA_MS.
//
// Reach is measured in KEYS, not raw point units — Chebyshev distance on the
// physical-coordinate grid (point_row / point_col), so "3 keys away" means the same
// in every direction despite columns sitting wider apart than rows. The same-half
// gate keeps the glow on the hand that pressed. Unlike the other reactive patterns
// this one deliberately makes no distinction about a key's function — render.c does
// not gate unmapped keys dark for it — so the neighbourhood lights up whole.
// Returns the combined spatial+time fade level (0 = at rest, 255 = just pressed).
static uint8_t area_factor(uint8_t led_index) {
#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
    uint8_t lx   = g_led_config.point[led_index].x;
    uint8_t mid  = RGB_MATRIX_MAX_X / 2;                        // split between the halves
    int16_t lrow = point_row(led_index);
    int16_t lcol = point_col(led_index);
    uint8_t best = 0;
    for (uint8_t i = 0; i < g_last_hit_tracker.count; i++) {
        uint8_t hidx = g_last_hit_tracker.index[i];

        bool same_half = ((lx < mid) == (g_led_config.point[hidx].x < mid));
        if (!same_half) continue;                               // stay on the pressing hand

        int16_t drow = lrow - (int16_t)point_row(hidx);
        int16_t dcol = lcol - (int16_t)point_col(hidx);
        if (drow < 0) drow = -drow;
        if (dcol < 0) dcol = -dcol;
        uint16_t dist = drow > dcol ? (uint16_t)drow : (uint16_t)dcol;   // Chebyshev, in keys
        if (dist > RGB_AREA_RADIUS) continue;                   // outside the neighbourhood

        uint16_t tick = g_last_hit_tracker.tick[i];             // ms since the press
        if (tick >= RGB_AREA_MS) continue;                      // fully faded
        uint16_t tf = 255 - (uint16_t)((uint32_t)tick * 255 / RGB_AREA_MS);     // linear time fade
        uint16_t sf = 255 - dist * (255 / (RGB_AREA_RADIUS + 1));               // opacity by distance
        uint8_t  f  = (uint8_t)(((uint32_t)tf * sf) / 255);
        if (f > best) best = f;
    }
    return best;
#else
    (void)led_index;
    return 0;
#endif
}

// ── Dispatch ──────────────────────────────────────────────────────────────────
uint8_t lighting_pattern_brightness(uint8_t pattern, uint8_t led_index) {
    switch (pattern) {
        case PAT_CROSS:
            return cross_factor(led_index);
        case PAT_REACTIVE:
            return splash_factor(led_index);                    // dark idle .. 255 peak
        case PAT_AREA:
            return area_factor(led_index);                      // dark idle .. 255 just pressed
        case PAT_FADE:
            return fade_factor(led_index);                      // dark idle .. 255 just pressed
        case PAT_SOLID:
            return RGB_IDLE_VAL;
        case PAT_OFF:
        default:
            return 0;
    }
}
