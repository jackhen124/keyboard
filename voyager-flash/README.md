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
3. Download the **`voyager-jack-firmware`** artifact. GitHub always hands you a
   **`.zip`** (that's just how it packages artifacts), so **unzip it** — inside
   is the actual `zsa_voyager_jack.bin` firmware file. That `.bin` is the thing
   you flash; the `.zip` itself can't be flashed.
4. Put the Voyager into **bootloader mode**: press the small reset button on the
   board, **or** hold the `_SYS` layer (right-outer thumb) and tap `QK_BOOT`
   (top-left key) once this keymap is already running.
5. Flash the `.bin` with any **one** flashing tool — you only need one, and the
   `.bin` never flashes itself:
   - **Wally** (GUI, https://www.zsa.io/wally): select the unzipped `.bin` and
     click flash.
   - **Keymapp** (GUI, https://www.zsa.io/keymapp): ZSA's newer app, same idea.
   - **zapp** (CLI, https://github.com/zsa/zapp): `zapp flash zsa_voyager_jack.bin`.
     Optional — only if you prefer the terminal. Note its `zapp flash <url>`
     shortcut works with **Oryx** URLs, not GitHub artifact links, so you still
     download + unzip first.

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

> Note: this is an approximation of the Glorious Engrammer, not a 1:1 clone.
> The home-row mods ARE close to Sunaku's — bilateral holds (`CHORDAL_HOLD`),
> prior-idle gating (`FLOW_TAP_TERM`), and per-finger tapping terms. Still
> missing: the magic/repeat key and emoji/World layers. See
> `../oryx-tricky-features.md` for those gaps.
