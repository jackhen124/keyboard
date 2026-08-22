#!/usr/bin/env python3
"""Generate per-layer keymap images for the Voyager straight from keymap.c.

No QMK toolchain required. The script parses keymap.c itself:

  1. reads the `enum layers { ... }` to recover layer names + order,
  2. reads the object-like `#define` keycode aliases (HM_*, TH_*, NM_* ...),
  3. reads the `keymaps[][...] = { [_LAYER] = LAYOUT_xxx(...) }` block,
  4. expands the aliases + resolves layer-name refs (e.g. LT(_SYM,..) -> LT(1,..)),
  5. writes a keymap-drawer-friendly QMK keymap.json, then
  6. shells out to keymap-drawer to render voyager.svg + one SVG per layer.

Re-run it whenever keymap.c changes:  python voyager/gen-keymap-images.py

Requires: keymap-drawer  (pip install keymap-drawer)
"""

from __future__ import annotations

import json
import re
import subprocess
import sys
from pathlib import Path

# ---------------------------------------------------------------------------
# Config -- adjust here if you rename files or move to another board.
# ---------------------------------------------------------------------------
REPO = Path(__file__).resolve().parent.parent          # keyboard/
KEYMAP_C = REPO / "voyager" / "flash" / "keymap.c"
JSON_OUT = REPO / "voyager" / "voyager.keymap.json"
IMG_DIR = REPO / "images"
QMK_KEYBOARD = "zsa/voyager"     # used by keymap-drawer to fetch the physical layout
QMK_LAYOUT = "LAYOUT"            # the layout name in keymap-drawer's info.json for this board

# Tokens QMK provides that aren't #defined in keymap.c. `_____` (5 underscores)
# is an undefined typo in the current Symbol layer; treat it as transparent so
# the image still renders. Remove that line once keymap.c is fixed.
ALIASES = {
    "_______": "KC_TRNS",
    "XXXXXXX": "KC_NO",
    "_____": "KC_TRNS",
    # Lighting-mode selectors (defined in lighting.h, not keymap.c). Give them
    # short labels so the number row renders cleanly instead of "LM_MODE_n".
    **{f"LM_MODE_{n}": f"LM{n}" for n in range(1, 11)},
}


def strip_comments(src: str) -> str:
    """Remove /* ... */ and // ... comments without touching string literals."""
    src = re.sub(r"/\*.*?\*/", "", src, flags=re.DOTALL)
    src = re.sub(r"//[^\n]*", "", src)
    return src


def parse_layers(src: str) -> list[str]:
    """Return layer enum names in declaration order (e.g. ['_BASE', '_SYM', ...])."""
    m = re.search(r"enum\s+layers\s*\{(.*?)\}", src, flags=re.DOTALL)
    if not m:
        sys.exit("error: could not find `enum layers { ... }` in keymap.c")
    names = []
    for entry in m.group(1).split(","):
        entry = entry.strip()
        if not entry:
            continue
        names.append(entry.split("=")[0].strip())
    return names


def parse_defines(src: str) -> dict[str, str]:
    """Object-like #define NAME BODY macros (skips function-like NAME(...) ones)."""
    defines = {}
    for m in re.finditer(r"^\s*#define\s+(\w+)\s+(.+)$", src, flags=re.MULTILINE):
        name, body = m.group(1), m.group(2).strip()
        # skip function-like macros: `#define FOO(x) ...`
        if src[m.start(1) + len(name)] == "(":
            continue
        defines[name] = body
    return defines


def split_top_level(body: str) -> list[str]:
    """Split a LAYOUT(...) body on commas that are not inside nested parens."""
    tokens, depth, cur = [], 0, ""
    for ch in body:
        if ch == "(":
            depth += 1
            cur += ch
        elif ch == ")":
            depth -= 1
            cur += ch
        elif ch == "," and depth == 0:
            tokens.append(cur.strip())
            cur = ""
        else:
            cur += ch
    if cur.strip():
        tokens.append(cur.strip())
    return tokens


def extract_layout_body(src: str, start: int) -> tuple[str, int]:
    """Given index of the '(' after LAYOUT_xxx, return (body, index_after_close)."""
    depth, i = 0, start
    while i < len(src):
        if src[i] == "(":
            depth += 1
        elif src[i] == ")":
            depth -= 1
            if depth == 0:
                return src[start + 1 : i], i + 1
        i += 1
    sys.exit("error: unbalanced parentheses in a LAYOUT(...) block")


def parse_keymaps(src: str, layer_order: list[str]) -> dict[str, str]:
    """Map layer-enum-name -> raw LAYOUT body string, for the keymaps[] array."""
    km = re.search(r"keymaps\s*\[\s*\]\s*\[.*?\]\s*\[.*?\]\s*=\s*\{", src, flags=re.DOTALL)
    if not km:
        sys.exit("error: could not find the `keymaps[][][] = { ... }` array")
    bodies = {}
    for m in re.finditer(r"\[\s*(\w+)\s*\]\s*=\s*\w+\s*\(", src[km.end():]):
        layer = m.group(1)
        if layer not in layer_order:
            continue
        body, _ = extract_layout_body(src, km.end() + m.end() - 1)
        bodies[layer] = body
    missing = [l for l in layer_order if l not in bodies]
    if missing:
        sys.exit(f"error: no keymap entry found for layer(s): {', '.join(missing)}")
    return bodies


def expand_token(tok: str, defines: dict[str, str], layer_idx: dict[str, int]) -> str:
    """Expand macro aliases, then resolve layer-name references to indices."""
    # Repeatedly expand object-like macros until stable (guarded against cycles).
    for _ in range(10):
        new = re.sub(r"\b\w+\b", lambda w: defines.get(w.group(0), w.group(0)), tok)
        if new == tok:
            break
        tok = new
    tok = ALIASES.get(tok, tok)
    # Resolve layer enum names (e.g. LT(_SYM, KC_SPC) -> LT(1, KC_SPC)).
    for name, idx in layer_idx.items():
        tok = re.sub(rf"\b{name}\b", str(idx), tok)
    return tok


def main() -> None:
    src = strip_comments(KEYMAP_C.read_text(encoding="utf-8"))
    layer_order = parse_layers(src)
    layer_idx = {name: i for i, name in enumerate(layer_order)}
    defines = parse_defines(src)
    bodies = parse_keymaps(src, layer_order)

    layers = []
    for name in layer_order:
        tokens = [expand_token(t, defines, layer_idx) for t in split_top_level(bodies[name])]
        layers.append(tokens)

    JSON_OUT.write_text(
        json.dumps(
            {"keyboard": QMK_KEYBOARD, "keymap": "voyager", "layout": QMK_LAYOUT, "layers": layers},
            indent=2,
        ),
        encoding="utf-8",
    )
    print(f"wrote {JSON_OUT.relative_to(REPO)}  ({len(layers)} layers)")

    # Human-friendly layer names for the yaml: strip the leading underscore.
    display_names = [n.lstrip("_") for n in layer_order]

    IMG_DIR.mkdir(exist_ok=True)
    yaml_path = IMG_DIR / "voyager.yaml"

    def kd(*args: str, **kw):
        # Force UTF-8 so the Windows console codec doesn't choke on glyphs like ▽.
        return subprocess.run(
            [sys.executable, "-m", "keymap_drawer", *args],
            check=True,
            env={**__import__("os").environ, "PYTHONUTF8": "1"},
            **kw,
        )

    # 1. Parse json -> yaml.
    with yaml_path.open("w", encoding="utf-8") as f:
        kd("parse", "-q", str(JSON_OUT), stdout=f)

    # 2. Rename L0/L1/... to real layer names and pin the layout name.
    text = yaml_path.read_text(encoding="utf-8")
    text = text.replace(f"layout_name: {QMK_LAYOUT}_voyager", f"layout_name: {QMK_LAYOUT}")
    for i, disp in enumerate(display_names):
        text = re.sub(rf"^  L{i}:", f"  {disp}:", text, flags=re.MULTILINE)
    yaml_path.write_text(text, encoding="utf-8")

    # 3. Draw the combined image + one SVG per layer.
    layers_dir = IMG_DIR / "layers"
    layers_dir.mkdir(exist_ok=True)
    with (IMG_DIR / "voyager.svg").open("w", encoding="utf-8") as f:
        kd("draw", str(yaml_path), stdout=f)
    for disp in display_names:
        with (layers_dir / f"{disp}.svg").open("w", encoding="utf-8") as f:
            kd("draw", str(yaml_path), "-s", disp, stdout=f)

    print(f"rendered images/voyager.svg + images/layers/{{{','.join(display_names)}}}.svg")


if __name__ == "__main__":
    main()
