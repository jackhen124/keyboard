# voyager-flash

Everything needed to build and flash the ZSA Voyager firmware migrated from the
Glove80 "Glorious Engrammer" (QWERTY base).

## Files

| File | Purpose |
|---|---|
| `keymap.c` | The keymap: 5 layers (Base, Sym, Num, Nav, Sys). Edit this to change keys. |
| `config.h` | Home-row-mod tuning (`TAPPING_TERM`, `PERMISSIVE_HOLD`, `QUICK_TAP_TERM`). |
| `rules.mk` | Optional QMK feature toggles (combos, tap dance, unicode). |

The build workflow lives at **`.github/workflows/build-voyager.yml`** (GitHub
requires workflows to sit under `.github/workflows/`, so it can't live in this
folder). It copies these three files into a QMK checkout and compiles them.

## How to flash

1. **Push** any change under `voyager-flash/` to GitHub (or trigger the
   *Build Voyager firmware* workflow manually from the Actions tab).
2. Open the **Actions** tab → the latest *Build Voyager firmware* run.
3. Download the **`voyager-jack-firmware`** artifact (contains the `.bin`).
4. Flash with **Wally** (https://www.zsa.io/wally):
   - Put the Voyager into bootloader mode: press the small reset button on the
     board, **or** hold the `_SYS` layer (right-outer thumb) and tap `QK_BOOT`
     (top-left key) once this keymap is already running.
   - In Wally, select the downloaded `.bin` and click flash.

## Editing the keymap

Edit `keymap.c` directly. The `LAYOUT_voyager()` macro takes exactly 52 keys per
layer in this order:

```
row0 L(6)  row0 R(6)
row1 L(6)  row1 R(6)
row2 L(6)  row2 R(6)
row3 L(6)  row3 R(6)
thumbs: L-outer  L-inner  R-inner  R-outer
```

Keep each layer at 52 arguments or the build fails. Commit + push and the
workflow rebuilds automatically.

> Note: this is an approximation of the Glorious Engrammer, not a 1:1 clone —
> standard GACS home-row mods and 4 thumb layers, without Sunaku's per-finger
> timing / magic key / emoji layers. See `../oryx-tricky-features.md` for the
> gaps.
