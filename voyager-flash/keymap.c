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

enum layers {
    _BASE = 0,
    _SYM,   // right inner thumb (hold)  -> type symbols with either hand
    _NUM,   // left  outer thumb (hold)  -> numpad on right hand, F-keys on left
    _NAV,   // left  inner thumb (hold)  -> cursor / navigation on right hand
    _SYS,   // right outer thumb (hold)  -> media, F-keys, bootloader
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

// Thumb layer-taps
#define TH_NUM LT(_NUM, KC_DEL)
#define TH_NAV LT(_NAV, KC_BSPC)
#define TH_SYM LT(_SYM, KC_SPC)
#define TH_SYS LT(_SYS, KC_ENT)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_BASE] = LAYOUT_voyager(
        KC_EQL,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,        KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,        KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_MINS,
        KC_ESC,  HM_A,    HM_S,    HM_D,    HM_F,    KC_G,        KC_H,    HM_J,    HM_K,    HM_L,    HM_SC,   KC_QUOT,
        KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,        KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
                                            TH_NUM,  TH_NAV,      TH_SYM,  TH_SYS
    ),

    // Symbols. Faithful mirror of the Glove80 "Glorious Engrammer" Symbol layer:
    // left hand types symbols while the right thumb holds Space; the right hand
    // keeps whitespace/editing keys. The Glove80's 6th (bottom) row has no home
    // on the 4-row Voyager, so its five orphaned symbols (@ % & + :) are rescued
    // onto the otherwise-empty right half of the number row.
    [_SYM] = LAYOUT_voyager(
        TRIPLE_GRV, KC_GRV,  KC_LABK, KC_RABK, KC_SCLN, KC_COMM,     KC_AT,   KC_PERC, KC_AMPR, KC_PLUS, KC_COLN, _______,
        KC_EXLM,    KC_LBRC, KC_LCBR, KC_RCBR, KC_RBRC, KC_QUES,     KC_DOT,  KC_DEL,  LSFT(KC_TAB), KC_INS, KC_ESC, _______,
        KC_HASH,    KC_CIRC, KC_EQL,  KC_UNDS, KC_DLR,  KC_ASTR,     KC_COMM, KC_BSPC, KC_ENT,  KC_SPC,  KC_ENT,  _______,
        KC_TILD,    KC_LPRN, KC_PIPE, KC_MINS, KC_RPRN, KC_SLSH,     KC_SCLN, KC_DQUO, KC_QUOT, KC_GRV,  KC_BSLS, _______,
                                            _______, _______,     _______, _______
    ),

    // Numpad on the right hand, F1-F12 on the left. Hold left-outer thumb (Del).
    [_NUM] = LAYOUT_voyager(
        _______, _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______, _______,
        _______, KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,      KC_7,    KC_8,    KC_9,    KC_PLUS, KC_ASTR, _______,
        _______, KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,       KC_4,    KC_5,    KC_6,    KC_MINS, KC_SLSH, _______,
        _______, KC_F6,   KC_F12,  _______, _______, _______,     KC_1,    KC_2,    KC_3,    KC_EQL,  KC_DOT,  _______,
                                            _______, _______,     KC_0,    _______
    ),

    // Navigation / cursor on the right hand (vim arrows). Hold left-inner thumb (Bspc).
    [_NAV] = LAYOUT_voyager(
        _______, _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______,     KC_HOME, KC_PGDN, KC_PGUP, KC_END,  KC_DEL,  KC_BSPC,
        _______, _______, _______, _______, _______, _______,     KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_CAPS, _______,
        _______, _______, _______, _______, _______, _______,     KC_ESC,  KC_TAB,  _______, _______, _______, _______,
                                            _______, _______,     _______, _______
    ),

    // System: media + volume + F-keys, plus QK_BOOT (top-left) to enter bootloader
    // for flashing with Wally. Hold right-outer thumb (Enter).
    [_SYS] = LAYOUT_voyager(
        QK_BOOT, _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______, EE_CLR,
        _______, _______, _______, _______, _______, _______,     KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,
        KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, _______, _______,     KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,
        KC_VOLD, KC_VOLU, _______, _______, _______, _______,     _______, _______, _______, _______, _______, _______,
                                            _______, _______,     _______, _______
    ),
};

// ── Custom keycode handling ─────────────────────────────────────────────────
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode == TRIPLE_GRV && record->event.pressed) {
        SEND_STRING("```");
        return false;
    }
    return true;
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

// ── Per-finger tapping terms (TAPPING_TERM_PER_KEY) ─────────────────────────
// Slow fingers (pinky) get more time before a hold counts; fast fingers (index)
// get less. Mirrors Sunaku's per-position holding times. Thumbs and everything
// else fall through to the global TAPPING_TERM.
uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case HM_A:  case HM_SC:  return 260;  // pinky
        case HM_S:  case HM_L:   return 220;  // ring
        case HM_D:  case HM_K:   return 190;  // middle
        case HM_F:  case HM_J:   return 170;  // index
        default:                 return TAPPING_TERM;
    }
}
