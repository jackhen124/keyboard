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
// (held Backspace to bulk-delete, held Space, etc.) — matching the Glove80's
// *_REPEAT_DECAY. 120 ms is short enough that a deliberate hold-for-mod after a
// pause still registers as the hold, so HRMs are unaffected.
#define QUICK_TAP_TERM 120

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
