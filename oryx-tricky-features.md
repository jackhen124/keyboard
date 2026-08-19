# Glorious Engrammer → Oryx: guide to the tricky keys

Your Glove80 export has empty `holdTaps`, `combos`, `macros`, and `inputListeners`
arrays — every clever behavior lives in Sunaku's `custom_defined_behaviors`
(`&left_pinky_tap`, `&sticky_key_modtap`, `&magic`, `&thumb …`). Those are ZMK C
macros with tuning Oryx's GUI does not expose. This guide covers each one: what it
does, whether Oryx can do it, how to get closest, and where you'll have to
compromise (or eject to custom QMK firmware).

**Reality check up front:** Oryx can reproduce the *shape* of most of this
(home-row mods, layer-thumbs, one-shot shift, combos, mouse keys). It cannot
reproduce Sunaku's *timing tricks* — the per-finger tapping terms, `require-prior-idle`,
and bilateral positional conditions that make his HRMs not fire during fast typing.
If those matter to you, the ceiling in Oryx is "good HRMs," not "Sunaku's HRMs."

Legend for each section: **What it does → Oryx verdict → How → Caveats.**

---

## 1. Home-row mods (`&left_pinky_tap`, `&left_ringy_tap`, `&left_middy`, `&left_index`, and right-hand twins)

**What it does.** Each home-row key is tap = letter, hold = modifier. Sunaku uses a
*separate behavior per finger* on purpose:
- Pinky gets a longer tapping term (slow finger, needs more time before a hold counts).
- Index/middle get shorter terms (fast, precise fingers).
- All of them use `require-prior-idle-ms`: if you were typing quickly a moment ago,
  the key is forced to tap — so `sd`, `df`, `as` in a fast word never emit a stray
  Ctrl/Alt. This is the "timeless" HRM trick.
- Bilateral `hold-trigger-key-positions`: a left-hand mod only engages if the next key
  is on the *right* hand. Same-hand rolls stay as letters.

**Oryx verdict.** ⚠️ Partial. You get tap/hold mods easily; you lose the anti-misfire tuning.

**How in Oryx.**
1. Click the home-row key (e.g. `A`), open **Dual-Function**.
2. Set **Tap = A**, **Hold = Left GUI** (⌘/Win). Repeat across the row:
   `A`=GUI, `S`=Alt, `D`=Ctrl, `F`=Shift, and mirror on the right (`J`=Shift,
   `K`=Ctrl, `L`=Alt, `;`=GUI). This is the standard GACS order you already have in
   the QMK version.
3. Under keyboard **Settings → Tapping Term**, raise the global term to ~200 ms as a
   starting point.

**Caveats.**
- Oryx's tapping term is **global**, not per-finger. You can't give the pinky 260 ms
  and the index 180 ms. Pick one compromise value.
- No `require-prior-idle` and no bilateral positional condition in the GUI → expect
  occasional accidental mods on fast same-hand rolls until you tune the term up.
- Oryx's hold flavor: it exposes basic dual-function; it does **not** expose
  ZMK-style `hold-preferred` vs `balanced` vs `tap-preferred` per key. If misfires
  annoy you, your only lever is the global term.
- **To truly match Sunaku:** you must eject to a custom QMK keymap and add an
  Achordion-style library (bilateral hold gating) + per-key tapping terms. That's the
  keymap.c route, not Oryx.

---

## 2. Sticky Shift (`&sticky_key_modtap LSFT LSFT`)

**What it does.** The outer thumb/pinky shift is a *combined* behavior: tap once =
one-shot (sticky) Shift that applies to the next key only; hold = normal Shift held.
It also has a timeout so a stale sticky Shift auto-cancels.

**Oryx verdict.** ⚠️ Close, but split into two decisions.

**How in Oryx.**
- For **one-shot Shift**: click the key → **Advanced / One Shot** category → **One
  Shot Left Shift** (`OSM(MOD_LSFT)`). Tap = sticky Shift for the next keypress.
- Oryx one-shot keys already behave as hold=Shift/tap=sticky in QMK's default
  one-shot implementation (holding past the tapping term acts as a normal mod), so a
  single One-Shot-Shift key usually gives you both behaviors.

**Caveats.**
- The auto-cancel **timeout** for a dangling one-shot is a global QMK setting
  (`ONESHOT_TIMEOUT`) not surfaced in Oryx — you can't tune it in the GUI.
- If you want *tap = sticky, hold = a different thing entirely*, Oryx dual-function
  can't combine "one-shot" as the tap action. One-shot-Shift is your one-key answer.

---

## 3. The Magic key (`&magic`)

**What it does.** This is the hardest one. In Glorious Engrammer the magic key is a
multi-purpose beast: a **repeat / alternate-repeat** ("magic") tap that outputs a
context-dependent key based on what you last typed, plus **hold = the Magic layer**
for less-common actions. It's what powers Sunaku's "sunaku magic" repeat tricks.

**Oryx verdict.** ❌ Largely not reproducible in the GUI.

**How in Oryx (closest approximation).**
- **Hold = Magic layer:** doable — make it a **Dual-Function** key with Hold = *Toggle/
  Momentary Layer → Magic*. Build a "Magic" layer for the extras you actually use.
- **Tap = repeat:** QMK *has* `QK_REP` (Repeat Key) and `QK_AREP` (Alternate Repeat),
  which are the real primitives behind this — but **Oryx does not expose them in the
  keycode picker.** So in Oryx the tap has to become something concrete (e.g. a real
  key) instead of the adaptive repeat.

**Caveats.**
- Honest recommendation: don't try to clone the magic key in Oryx. Decide what you
  *actually used it for*. If it was mostly the Magic layer, keep that on a hold. If you
  relied on adaptive repeat daily, that alone is a reason to go custom-firmware.

---

## 4. Thumb layer keys (`&thumb LAYER_Cursor BACKSPACE`, `&thumb LAYER_Number DELETE`, `&thumb LAYER_Symbol SPACE`, `&thumb LAYER_System ENTER`, etc.)

**What it does.** Each thumb is tap = a key (Space/Enter/Bspc/Del/Tab…), hold = a
layer (Cursor/Number/Symbol/System/Mouse/World/Lower/Emoji/Function). Sunaku's
`&thumb` behavior is tuned `hold-preferred` and *without* require-prior-idle, because
thumbs rarely misfire — so the layer engages fast and reliably.

**Oryx verdict.** ✅ Doable — this is the cleanest port.

**How in Oryx.**
- Each thumb → **Dual-Function** → Tap = the key, Hold = **Momentary Layer → (target)**.
- You only have **4 thumbs on the Voyager** vs the Glove80's 12, so you must pick your
  four most-used from the list below. Your QMK draft already chose:
  `Del/Number`, `Bspc/Cursor(Nav)`, `Space/Symbol`, `Enter/System`.

**Glove80 thumb inventory (choose 4):**
| Behavior | Tap | Hold layer |
|---|---|---|
| `&thumb LAYER_Cursor` | Backspace / Space | Cursor (nav) |
| `&thumb LAYER_Number` | Delete / Tab | Number |
| `&thumb LAYER_Symbol` | Space | Symbol |
| `&thumb LAYER_System` | Enter | System |
| `&thumb LAYER_Function` | Esc | Function |
| `&thumb LAYER_Mouse` | Tab / Backspace | Mouse |
| `&thumb LAYER_World` | Page Up | World (international) |
| `&thumb LAYER_Lower` | Insert / Delete | Lower |
| `&thumb LAYER_Emoji` | Backslash | Emoji |

**Caveats.** Oryx's hold flavor and term are global, so a thumb hold uses the same
tapping term as your home-row mods. Thumbs tolerate a lower term well, so if you tuned
the global term *up* for HRMs, your layer-holds will feel slightly laggy. Minor.

---

## 5. Caps Word / Caps Lock

**What it does.** Engrammer has a caps-word behavior (SHIFT that stays on for one word,
auto-releasing at space/punctuation) distinct from true Caps Lock.

**Oryx verdict.** ✅ Supported.

**How in Oryx.** Add the **Caps Word** key from the keycode picker (QMK `CW_TOGG`).
Tap it, type a word, it auto-releases. Use the standard **Caps Lock** key if you want
the persistent one.

**Caveats.** The word-break characters and idle timeout are QMK compile-time settings
(`CAPS_WORD_IDLE_TIMEOUT`, the continuation set) — not tunable in Oryx. Defaults are
sane for English.

---

## 6. Combos

**What it does.** Chording two keys for a third output (Sunaku ships many, though note:
**your export's `combos` array is empty**, so none came across in this file — they live
in his ZMK config, not the layout JSON).

**Oryx verdict.** ✅ Supported (Oryx has a Combos panel).

**How in Oryx.** Use the **Combos** section: pick 2+ source keys and the resulting
keycode/layer. Rebuild only the combos you actually use from memory — nothing to import.

**Caveats.** Combo tapping/hold timing (`COMBO_TERM`) is global. High combo counts can
get finicky; add them incrementally and test.

---

## 7. Mouse layers (Mouse / MouseSlow / MouseFast / MouseWarp)

**What it does.** Four graded pointer layers: normal, slow-precision, fast, and "warp"
(big jumps), i.e. mouse movement with multiple acceleration profiles.

**Oryx verdict.** ⚠️ Basic mouse yes; graded profiles no.

**How in Oryx.** Add **Mouse Keys** (cursor move, buttons, scroll) from the picker on a
Mouse layer. Set global mouse speed/acceleration under **Settings**.

**Caveats.** Oryx gives you *one* global mouse acceleration curve. The slow/fast/warp
*tiers* rely on per-layer speed constants (`MK_*` defines) that the GUI doesn't expose.
Collapse to a single Mouse layer, or go custom firmware for the tiers.

---

## 8. Emoji + World (international) layers

**What it does.** Emoji layer types 😀-style Unicode; World layer types accented/
international characters — both via ZMK Unicode macros.

**Oryx verdict.** ❌ / ⚠️ Weakest area in Oryx.

**How in Oryx.** Oryx's Unicode support is limited and OS-dependent; there's no clean
emoji-picker equivalent. For accented Latin chars you can sometimes use the **AltGr /
right-Alt** approach with your OS layout, or add a few specific Unicode inputs.

**Caveats.** If emoji-by-keyboard is important, this is a genuine gap. Options: rely on
the OS emoji picker (Win `.`), or eject to QMK with `UNICODE_ENABLE` + `UNICODEMAP` and
set your host Unicode input mode. Not a Oryx-GUI thing.

---

## 9. Alternate base layouts (Enthium / Colemak / Dvorak / macOS variants)

**What it does.** Your export carries QWERTY, Enthium, Colemak, Dvorak, plus macOS
variants as separate base layers, swappable via the Magic layer.

**Oryx verdict.** ✅ Trivial, just tedious.

**How in Oryx.** Each is just a full base layer with the letters rearranged — same grid,
same thumbs. Create one Oryx layer per alphabet you actually switch between, and put a
layer-toggle key somewhere. Don't port all six; port the one you type in daily (plus one
spare if you truly switch).

---

## Summary: what ports cleanly vs. what forces custom firmware

| Feature | Oryx GUI | Notes |
|---|---|---|
| Thumb layer-taps | ✅ | Pick 4 of the Glove80's thumbs |
| Home-row mods (shape) | ✅ | Standard GACS dual-function |
| HRM anti-misfire tuning | ❌ | Per-finger term / prior-idle / bilateral → needs keymap.c |
| One-shot / sticky Shift | ✅ | One-Shot-Shift key |
| Caps Word | ✅ | `CW_TOGG` |
| Combos | ✅ | Rebuild by hand; none in your export |
| Basic mouse keys | ✅ | One accel curve only |
| Graded mouse (slow/fast/warp) | ❌ | Per-layer speed constants |
| Magic / repeat key | ❌ | `QK_REP`/`QK_AREP` not in Oryx picker |
| Emoji / Unicode World layer | ❌ | Oryx Unicode is limited |
| Alt base layouts | ✅ | One layer each |

**Bottom line for a GUI-only workflow:** you can rebuild a very usable ~85% of the
Engrammer in Oryx. The three things you'll miss most are (a) misfire-proof HRMs,
(b) the magic/repeat key, and (c) emoji. If any of those three is a daily must-have,
that's your cue to flash the `keymap.c` route instead of (or alongside) Oryx.
