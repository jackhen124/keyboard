#pragma once

// ── Home-row-mod tuning: "close to Sunaku's Glorious Engrammer" ──────────────
// Four features work together so same-hand rolls stop misfiring as modifiers.

#define TAPPING_TERM 200          // global default; overridden per-finger in keymap.c
#define TAPPING_TERM_PER_KEY      // enable get_tapping_term(): longer pinky, shorter index
#define QUICK_TAP_TERM 0          // no auto-repeat on tap-then-hold (essential for HRMs)

// Bilateral holds. A home-row mod engages only if the NEXT key is on the
// OPPOSITE hand; same-hand rolls (sd, df, as…) stay letters. This is Sunaku's
// hold-trigger-key-positions. The handedness map lives in keymap.c.
#define CHORDAL_HOLD

// Flow Tap. If a key was pressed very recently (i.e. you're mid-word typing
// fast), force tap-hold keys to TAP. This is Sunaku's require-prior-idle-ms —
// it kills stray mods inside fast words.
#define FLOW_TAP_TERM 150         // ms; raise toward 200 if misfires persist

// Chordal Hold already resolves the "rolled into another key" case via
// handedness, so permissive hold stays off. Uncomment only if opposite-hand
// holds feel reluctant to trigger:
// #define PERMISSIVE_HOLD
