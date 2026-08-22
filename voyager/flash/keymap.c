// ZSA Voyager keymap migrated from Glove80 "Glorious Engrammer" (QWERTY base).
//
// This is a STARTING POINT, not a 1:1 clone of Sunaku's ZMK keymap:
//  - Base alpha rows + number row map 1:1 from the Glove80.
//  - Home-row mods use GACS (GUI/ALT/CTRL/SHIFT) with Sunaku-style misfire
//    proofing: bilateral holds (CHORDAL_HOLD), prior-idle gating (FLOW_TAP_TERM),
//    and per-finger tapping terms. See config.h and the two functions below.
//  - 4 thumb keys carry the 4 most useful layers (Sym/Num/Nav/Sys).
//  - The remaining Glove80 layers (Mouse, Emoji, World, Function, etc.) are not
//    ported; add what you use by editing the layers below.
//
// Voyager LAYOUT_voyager() argument order (52 keys):
//   row0 L(6) row0 R(6) | row1 L(6) row1 R(6) | row2 L(6) row2 R(6)
//   row3 L(6) row3 R(6) | thumbs: L-outer L-inner R-inner R-outer

#include QMK_KEYBOARD_H
#include "lighting.h"   // RGB lighting + the LM_PAT_*/LM_COL_* top-row selector keys

enum layers {
    _BASE = 0,
    _SYM,    // right outer thumb (hold) -> type symbols with either hand
    _NUM,    // left  inner thumb (hold) -> numpad on right hand, F-keys on left
    _NAV,    // left  outer thumb (hold) -> cursor / navigation on right hand
    _MOUSE,  // right inner thumb (hold) -> pointer move / click / scroll
};

// Custom keycodes. TRIPLE_GRV reproduces the Glove80's triple_backtick macro
// (markdown code fence) that lived at the top-left of the Symbol layer.
enum custom_keycodes {
    TRIPLE_GRV = SAFE_RANGE,
};

// Home-row mods (GACS). Left hand uses left-side mods, right hand right-side.
#define HM_A LGUI_T(KC_A)
#define HM_S LALT_T(KC_S)
#define HM_D LCTL_T(KC_D)
#define HM_F LSFT_T(KC_F)
#define HM_J RSFT_T(KC_J)
#define HM_K RCTL_T(KC_K)
#define HM_L RALT_T(KC_L)   // RALT = AltGr; swap to LALT_T if that causes issues
#define HM_SC RGUI_T(KC_SCLN)

// NUM-layer home-row mods. The numpad lives on the right hand, so the LEFT
// home row keeps WIN/ALT/CTRL/SHIFT (tap = F-key, hold = mod), matching the
// base layer. NAV needs no equivalent: its left home row is transparent, so
// the base HM_* mod-taps already shine through.
#define NM_F1 LGUI_T(KC_F1)  // A: tap F1,  hold WIN
#define NM_F2 LALT_T(KC_F2)  // S: tap F2,  hold ALT
#define NM_F3 LCTL_T(KC_F3)  // D: tap F3,  hold CTRL
#define NM_F4 LSFT_T(KC_F4)  // F: tap F4,  hold SHIFT

// Thumb layer-taps. Order across the 4 thumbs (L-outer .. R-outer):
//   Cursor/Nav, Number, Mouse, Symbol. Both LEFT thumbs tap Backspace so
//   whichever one the thumb lands on sends Bspc; right thumbs keep Space/Enter.
//   (Del still lives on the Nav and Sym layers.)
#define TH_NAV LT(_NAV,   KC_BSPC)  // L-outer: tap Bspc,  hold Cursor/Nav
#define TH_NUM LT(_NUM,   KC_DEL)  // L-inner: tap Del,  hold Number
#define TH_MOU LT(_MOUSE, KC_TAB)   // R-inner: tap tab, hold Mouse
#define TH_SYM LT(_SYM,   KC_SPC)   // R-outer: tap space, hold Symbol

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    // Top row: the ENTIRE row is LIGHTING selectors, not keys — type numbers
    // from the _NUM layer; "=" lives on _SYM/_NUM and Backspace on the left thumb
    // (TH_NAV). The row is split into two independent selectors: the LEFT half
    // (LM_PAT_1..6) picks the lighting PATTERN and the RIGHT half (LM_COL_1..6)
    // picks the COLOR, chosen separately. See lighting.c.
    [_BASE] = LAYOUT_voyager(
        LM_PAT_1, LM_PAT_2, LM_PAT_3, LM_PAT_4, LM_PAT_5, LM_PAT_6,  LM_COL_1, LM_COL_2, LM_COL_3, LM_COL_4, LM_COL_5, LM_COL_6,
        _______,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,        KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_MINS,
        KC_ESC,   HM_A,    HM_S,    HM_D,    HM_F,    KC_G,        KC_H,    HM_J,    HM_K,    HM_L,    HM_SC,   KC_QUOT,
        KC_LSFT,  KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,        KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
                                            TH_NAV,  TH_NUM,      TH_MOU,  TH_SYM
    ),

    // Symbols. Faithful mirror of the Glove80 "Glorious Engrammer" Symbol layer:
    // left hand types symbols while the right-outer thumb (Enter) holds the layer;
    // the right hand keeps whitespace/editing keys. The Glove80's 6th (bottom) row has no home
    // on the 4-row Voyager, so its five orphaned symbols (@ % & + :) are rescued
    // onto the otherwise-empty right half of the number row.
    [_SYM] = LAYOUT_voyager(
        TRIPLE_GRV, KC_GRV,  KC_LABK, KC_RABK, KC_SCLN, KC_COMM,     _______, _______, _______, _______, KC_COLN, _______,
        KC_EXLM,    KC_LBRC, KC_LCBR, KC_RCBR, KC_RBRC, KC_QUES,     KC_DOT,  KC_DEL,  LSFT(KC_TAB), KC_INS, KC_ESC, _______,
        KC_HASH,    KC_CIRC, KC_EQL,  KC_UNDS, KC_DLR,  KC_ASTR,     KC_AT,   KC_BSPC, KC_ENT,  KC_SPC,  KC_ENT,  KC_GRV,
        KC_TILD,    KC_LPRN, KC_PIPE, KC_MINS, KC_RPRN, KC_SLSH,     KC_AMPR, KC_PERC, KC_PLUS, KC_GRV,  KC_BSLS, _______,
                                            _______, _______,     _______, _______
    ),

    // Numpad on the right hand, F1-F12 on the left. Hold left-inner thumb (Bspc).
    [_NUM] = LAYOUT_voyager(
        _______, _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______, _______,
        _______, KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,      _______, KC_7,    KC_8,    KC_9,    KC_PLUS, KC_ASTR, 
        _______, NM_F1,   NM_F2,   NM_F3,   NM_F4,   KC_F5,       _______, KC_4,    KC_5,    KC_6,    KC_MINS, KC_SLSH,
        _______, KC_F6,   KC_F12,  _______, _______, _______,     _______, KC_1,    KC_2,    KC_3,    KC_EQL,  KC_DOT, 
                                            _______, _______,     _______, KC_0    
    ),

    // Navigation / cursor on the right hand (vim arrows). Hold left-outer thumb (Del).
    [_NAV] = LAYOUT_voyager(
        _______, _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______,     KC_HOME, KC_PGDN, KC_PGUP, KC_END,  KC_DEL,  KC_BSPC,
        _______, _______, _______, _______, _______, _______,     KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_CAPS, _______,
        _______, _______, _______, _______, _______, _______,     KC_ESC,  KC_TAB,  _______, _______, _______, _______,
                                            _______, _______,     _______, _______
    ),

    // Mouse. Ported from the Glove80 "Glorious Engrammer" Mouse layer, collapsed
    // to a single speed (its slow/fast/warp graded layers have no home here).
    // Hold right-inner thumb (Space): move the pointer with the LEFT hand
    // (E=up, S/D/F=left/down/right), click with the RIGHT (U/I/O = L/M/R), scroll
    // on the outer columns, and cut/copy/paste on Z/X/C. QK_BOOT stays top-left
    // for entering the bootloader to flash.
    [_MOUSE] = LAYOUT_voyager(
        QK_BOOT, _______, _______,  _______, _______, _______,     _______, _______, _______,      _______,      _______, _______,
        _______, _______, MS_WHLL,  MS_UP,   MS_WHLR, _______,     MS_WHLL, MS_BTN1, MS_BTN3,      MS_BTN2,      MS_WHLR, _______,
        KC_ESC,  MS_WHLU, MS_LEFT,  MS_DOWN, MS_RGHT, MS_WHLU,     MS_WHLU, _______, _______,      _______,      KC_APP,  MS_BTN4,
        _______, MS_WHLD, KC_CUT,   KC_COPY, KC_PSTE, MS_WHLD,     MS_WHLD, KC_F13,  LSFT(KC_F13), LALT(KC_F13), KC_F16,  MS_BTN5,
                                             MS_BTN1, MS_BTN2,     _______, _______
    ),
};

// ── Custom keycode handling ─────────────────────────────────────────────────
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!lighting_process_record(keycode, record)) return false;
    if (keycode == TRIPLE_GRV && record->event.pressed) {
        SEND_STRING("```");
        return false;
    }
    return true;
}

// RGB lighting lives in lighting.c; just hand it the init hook.
void keyboard_post_init_user(void) {
    lighting_init();
}

// ── Bilateral hold handedness (CHORDAL_HOLD) ────────────────────────────────
// Marks each physical key's hand. A home-row mod holds only when the following
// key is on the opposite hand; same-hand rolls stay taps. Thumbs are '*'
// (exempt) so their layer-holds always fire, even with same-hand keys.
const char chordal_hold_layout[MATRIX_ROWS][MATRIX_COLS] PROGMEM = LAYOUT_voyager(
    'L', 'L', 'L', 'L', 'L', 'L',     'R', 'R', 'R', 'R', 'R', 'R',
    'L', 'L', 'L', 'L', 'L', 'L',     'R', 'R', 'R', 'R', 'R', 'R',
    'L', 'L', 'L', 'L', 'L', 'L',     'R', 'R', 'R', 'R', 'R', 'R',
    'L', 'L', 'L', 'L', 'L', 'L',     'R', 'R', 'R', 'R', 'R', 'R',
                        '*', '*',     '*', '*'
);

// Tapping term is a single global value (see TAPPING_TERM in config.h); there is
// no per-finger override. PERMISSIVE_HOLD + CHORDAL_HOLD, not a longer term, are
// what keep slow-finger holds reliable.
