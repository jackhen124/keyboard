#pragma once

#include QMK_KEYBOARD_H

// Lighting-mode selector keycodes. On the BASE layer these take over the WHOLE
// top row: tapping one switches the active lighting mode (see lighting.c).
// Left-to-right, LM_MODE_1 is the top-left key (where "=" used to be) and
// LM_MODE_12 the top-right (where Backspace used to be). They live in QMK's
// dedicated QK_USER range so they can never collide with the keymap's own custom
// keycodes (e.g. TRIPLE_GRV, which sits at SAFE_RANGE in keymap.c).
enum lighting_keycodes {
    LM_MODE_1 = QK_USER_0,
    LM_MODE_2,
    LM_MODE_3,
    LM_MODE_4,
    LM_MODE_5,
    LM_MODE_6,
    LM_MODE_7,
    LM_MODE_8,
    LM_MODE_9,
    LM_MODE_10,
    LM_MODE_11,
    LM_MODE_12,
};

// Set the initial RGB state. Call once from keyboard_post_init_user().
void lighting_init(void);

// Handle the lighting-mode selector keys. Call from process_record_user().
// Returns false (and consumes the key) when it was a lighting key, true
// otherwise so the keymap can go on to process its own keycodes.
bool lighting_process_record(uint16_t keycode, keyrecord_t *record);
