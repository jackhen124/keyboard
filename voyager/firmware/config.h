#pragma once

// ── Home-row-mod tuning: "close to Sunaku's Glorious Engrammer" ──────────────
// Four features work together so same-hand rolls stop misfiring as modifiers.

// One global tapping term for every tap-hold key. Chosen from measured tap
// durations: normal taps top out at ~100 ms, so 170 leaves ~70 ms of margin
// before a tap is mistaken for a hold. With PERMISSIVE_HOLD (below) this term
// only governs LONE holds; any hold-then-key chord resolves on the next press.
#define TAPPING_TERM 170
// Tap-then-hold auto-repeat. If you re-press a tap-hold key within this window
// of tapping it, the second press is a real tap you can hold to auto-repeat
// (held Space, held Del, etc.) — matching the Glove80's *_REPEAT_DECAY. 120 ms
// is short enough that a deliberate hold-for-mod after a pause still registers
// as the hold, so HRMs are unaffected. The Nav thumb opts OUT per-key (see
// QUICK_TAP_TERM_PER_KEY below): its tap is Backspace, and quick-tap was
// locking out the cursor layer right after correcting a typo.
#define QUICK_TAP_TERM 120
#define QUICK_TAP_TERM_PER_KEY   // get_quick_tap_term() in keymap.c: 0 for TH_NAV

// Bilateral holds. A home-row mod engages only if the NEXT key is on the
// OPPOSITE hand; same-hand rolls (sd, df, as…) stay letters. This is Sunaku's
// hold-trigger-key-positions. The handedness map lives in keymap.c.
#define CHORDAL_HOLD

// Flow Tap. If a key was pressed very recently (i.e. you're mid-word typing
// fast), force tap-hold keys to TAP. This is Sunaku's require-prior-idle-ms —
// it kills stray mods inside fast words.
#define FLOW_TAP_TERM 150         // ms; raise toward 200 if misfires persist

// Permissive Hold = QMK's equivalent of ZMK's "balanced" flavor: a tap-hold
// key settles as HOLD the moment a nested key is pressed AND released, instead
// of waiting out the whole tapping term. This is what the Glove80 thumbs did
// ("balanced"); without it, fast rolls into a layer/mod drop as taps (Sym-Enter
// coming through as a base "k"). Safe globally because CHORDAL_HOLD still vetoes
// same-hand rolls, so early holds only fire on opposite-hand or exempt thumbs.
#define PERMISSIVE_HOLD

// Thumb layer-taps settle as HOLD the moment ANY other key is PRESSED — no
// waiting for that key's release (PERMISSIVE_HOLD) or the 170 ms term. This is
// what makes the layers feel instant mid-typing: hold thumb, press arrow, the
// arrow moves on the press. Applied per-key (get_hold_on_other_key_press() in
// keymap.c) to the four thumbs only; letters/HRMs keep the gentler
// PERMISSIVE_HOLD + CHORDAL_HOLD resolution above.
#define HOLD_ON_OTHER_KEY_PRESS_PER_KEY

// Outer home-row Shift keys: tap for the next key, hold for ordinary Shift.
// Match the Glove80's existing sticky Shift expiration.
#define ONESHOT_TIMEOUT 500

// ── RGB Matrix: action-reactive lighting ────────────────────────────────────
// RGB_MATRIX is already enabled by the Voyager's data-driven config; we only
// need to opt into keypress tracking. RGB_MATRIX_KEYPRESSES turns on QMK's
// "last hit tracker" (g_last_hit_tracker), which records which LEDs were pressed
// and how long ago — that's what powers the keypress flash in keymap.c. The
// default mode is left on a static SOLID_COLOR because the custom indicator
// function repaints every LED each frame anyway (see rgb_matrix_indicators_
// advanced_user in keymap.c), so no stock animation (the old "rainbow") shows.
#define RGB_MATRIX_KEYPRESSES
#define RGB_MATRIX_DEFAULT_MODE RGB_MATRIX_SOLID_COLOR
