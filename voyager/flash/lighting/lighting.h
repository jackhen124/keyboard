#pragma once

#include QMK_KEYBOARD_H

// PUBLIC interface of the lighting module — the only lighting header keymap.c
// needs. The implementation is split across this folder by concern:
//   lighting.c          state + init + selector-key routing (this file's owner)
//   patterns.c          brightness: reactive ripple, cross, solid, off
//   colors.c            hue: white, gradient
//   render.c            the per-frame paint loop that combines the two
//   lighting_internal.h the shared tunables / enums / decls those files pass around

// Lighting selector keycodes. On the BASE layer these take over the WHOLE top
// row, split into two INDEPENDENT selectors:
//   - LEFT half  (LM_PAT_1..6): pick the lighting PATTERN (how keys animate).
//   - RIGHT half (LM_COL_1..6): pick the lighting COLOR  (the palette used).
// Pattern and color are chosen separately and combine (pattern = brightness,
// color = hue), so e.g. "reactive ripple" can run in white OR in the gradient.
// Left-to-right, LM_PAT_1 is the top-left key and LM_COL_6 the top-right. They
// live in QMK's dedicated QK_USER range so they can never collide with the
// keymap's own custom keycodes (e.g. TRIPLE_GRV at SAFE_RANGE in keymap.c).
enum lighting_keycodes {
    LM_PAT_1 = QK_USER_0,  // left top row: pattern selectors
    LM_PAT_2,
    LM_PAT_3,
    LM_PAT_4,
    LM_PAT_5,
    LM_PAT_6,
    LM_COL_1,              // right top row: color selectors
    LM_COL_2,
    LM_COL_3,
    LM_COL_4,
    LM_COL_5,
    LM_COL_6,
};

// Set the initial RGB state. Call once from keyboard_post_init_user().
void lighting_init(void);

// Handle the lighting selector keys. Call from process_record_user().
// Returns false (and consumes the key) when it was a lighting key, true
// otherwise so the keymap can go on to process its own keycodes.
bool lighting_process_record(uint16_t keycode, keyrecord_t *record);
