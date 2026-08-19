# Glove80 → Voyager base-layer migration (QWERTY / default layer)

The Glove80's **number row + 3 alpha rows** are 6 columns per hand, which map 1:1
onto the Voyager's 4×6 grid. Everything else on the Glove80 (F-row, outer columns,
4 of the 6 thumbs per hand, magic/bracket keys) has no Voyager home and is dropped —
you'll re-add what you want manually in Oryx.

`*` = Glove80 home-row-mod key (tap = letter, hold = modifier via Sunaku's custom
per-finger behavior). Enter the letter in Oryx now; add a dual-function modifier later.

## Voyager layout (type this into Oryx)

```
 =    1    2    3    4    5          6    7    8    9    0    ⌫? 
Tab   Q    W    E    R    T          Y    U    I    O    P    -
Esc   A*   S*   D*   F*   G          H    J*   K*   L*   ;*   '
Shift Z    X    C    V    B          N    M    ,    .    /   Shift
              [L1] [L2]                  [R1] [R2]
```

Notes:
- Top-right last key (Glove80 pos 21) was `&none` — free slot, suggest Backspace.
- `=` and `-` come from the Glove80 number-row edges and the top alpha-row edge.

## Thumb keys (2 per hand) — recommended default

| Voyager thumb | Tap | Hold (layer) | From Glove80 pos |
|---|---|---|---|
| Left inner  (L1) | Backspace | Cursor  | 69 |
| Left outer  (L2) | Delete    | Number  | 70 |
| Right inner (R1) | Space     | Symbol  | 74 |
| Right outer (R2) | Enter     | System  | 57 |

Gives the 4 essential taps (Space/Enter/Bspc/Del) and the 4 most useful hold-layers.

### Other Glove80 thumbs you dropped (swap in if you prefer)
- Left:  `Esc`/Function (52), `(` (53), `)` (54), `mo Lower` (71)
- Right: `Home` (55), `End` (56), `Tab`/Mouse (73), `Ins`/Lower (72)

## What was dropped (re-add manually as desired)
- Glove80 F-row (top row) — was empty on this layer anyway.
- Extra thumb keys above (Function/Mouse/World/Lower/Emoji layer access, parens, Home/End).
- `magic`, `` ` ``, `[`, `]`, `\`, PgUp/PgDn, mouse-scroll keys.
- 27 non-base layers (Number, Symbol, Cursor, Function, Mouse, etc.) — rebuild the
  ones you use in Oryx; the layer-access is now on the 4 thumb holds above.

## Alpha-layer swap
Default layer here is **QWERTY** (ZMK layer 0). If you actually type **Enthium**,
**Colemak**, or **Dvorak**, swap the 3 letter rows for that layer's letters — same grid,
same thumbs.
