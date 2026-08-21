# Symbol & special-key typing practice

A drill sheet for exercising every symbol and "weird" key on the Voyager
keymap, so you can feel out whether each placement earns its spot. Open this in
your editor and retype each line **underneath itself** (or into a scratch buffer)
until it flows without hunting.

Layers referenced (hold the thumb, then type):

| Layer     | Held thumb            | What it gives you                          |
|-----------|-----------------------|--------------------------------------------|
| **Sym**   | R-outer (Space)       | every symbol, both hands                    |
| **Num**   | L-inner (Del)         | numpad (right), F1–F12 (left)               |
| **Nav**   | L-outer (Bspc)        | arrows / Home / End / PgUp / PgDn           |
| **Mouse** | R-inner (Tab)         | pointer, click, scroll, cut/copy/paste      |

---

## 1. One symbol at a time (Sym layer)

Type each row several times. If a character makes you pause, note it.

```
` ` ` ` `   < < < < <   > > > > >   ; ; ; ; ;   , , , , ,
! ! ! ! !   [ [ [ [ [   { { { { {   } } } } }   ] ] ] ] ]
? ? ? ? ?   . . . . .   # # # # #   ^ ^ ^ ^ ^   = = = = =
_ _ _ _ _   $ $ $ $ $   * * * * *   @ @ @ @ @   ~ ~ ~ ~ ~
( ( ( ( (   | | | | |   - - - - -   ) ) ) ) )   / / / / /
" " " " "   ' ' ' ' '   \ \ \ \ \   % % % % %   & & & & &
+ + + + +   : : : : :
```

## 2. The awkward ones, isolated

These are the placements most likely to trip you up — alternating them forces
the muscle memory instead of letting a lucky roll cover for it.

```
{ } { } { } { }   [ ] [ ] [ ] [ ]   ( ) ( ) ( ) ( )   < > < > < >
| \ | \ | \ | \   ~ ` ~ ` ~ ` ~ `   ^ $ ^ $ ^ $   # @ # @ # @
_ - _ - _ - _ -   * & * & * & *   % $ % $ % $   + = + = + =
: ; : ; : ; : ;   ? ! ? ! ? ! ?   " ' " ' " ' "
```

## 3. Bracket & quote pairs (open → content → close)

```
(x)  (foo)  (a, b, c)  ((nested))  (deep(er(est)))
[i]  [0]    [a][b][c]  [[i, j]]    arr[idx][key]
{k}  {v}    {a: 1}     {{mustache}} {nested: {deep: true}}
<T>  <div>  <a href>   </close>    <Generic<Inner>>
"s"  'c'    "it's"     'say "hi"'  `code`  ```fence```
```

## 4. Real code lines (mixed symbols, both hands)

Retype these verbatim — they pack the symbols into realistic runs.

```c
int *p = &arr[i];              // & * [ ] = ;
x = (a + b) * c - d / e % f;   // ( ) + * - / %
if (a >= b && c != d) x |= 1;  // > = & ! | 
obj->field = ptr->next;        // - > . =
#define MAX(a, b) ((a) > (b) ? (a) : (b))
map["key"] = value;            // [ ] " =
result = flag ? yes : no;      // ? :
mask = ~(1 << n) | (0x0F & y); // ~ ( < & )
path = "C:\\Users\\me";        // \ "
regex = /^\d+\.\d+$/;          // / ^ \ . $
```

```python
data = {"id": 1, "tags": ["a", "b"], "ok": True}
total = sum(x**2 for x in nums if x % 2 == 0)
name, *rest = line.split("|")
print(f"{user}@{host}:~$ ")     # @ : ~ $
lambda x: x + 1 if x > 0 else -x
```

```bash
grep -E '^\s*#' file.md | awk '{print $1}'
cat a.txt >> b.txt 2>&1 && echo "done" || exit 1
export PATH="$HOME/bin:$PATH"
for f in *.md; do mv "$f" "${f%.md}.txt"; done
```

## 5. Markdown-flavored (the `TRIPLE_GRV` key + friends)

Your top-left Sym key sends ```` ``` ```` in one tap. Exercise it plus the
other markup symbols:

- **bold** and _italic_ and `inline code`
- [link](https://example.com) and ![img](pic.png)
- > blockquote with a *nested* emphasis
- headings: # H1  ## H2  ### H3
- lists: - item, 1. item, - [ ] todo, - [x] done
- pipe tables: | a | b | c |  and  |---|---|---|

Fenced block (start it with the triple-backtick key):

```
one
two
three
```

## 6. Numbers & math (Num layer — right-hand numpad)

```
0 1 2 3 4 5 6 7 8 9   9 8 7 6 5 4 3 2 1 0
12 + 34 = 46          100 - 1 = 99
6 * 7 = 42            84 / 2 = 42
3.14159  2.71828  1.41421  0.57721
(1 + 2) * (3 + 4) = 21
```

Also reachable on the base number row (no layer): `= 1 2 3 4 5 6 7 8 9 0`

## 7. F-keys (Num layer — left hand)

```
F1 F2 F3 F4 F5 F6   F7 F8 F9 F10 F11 F12
```

## 8. Navigation feel (Nav layer)

Not typeable as text, but drill the motions in your editor with Nav held:

- Arrows: `← ↓ ↑ →` (vim-style H J K L positions)
- Word/line hops: Home, End, PgUp, PgDn
- Del / Bspc on the right pinky columns

## 9. Endurance paragraph

One pass, no stopping. Every symbol appears at least once:

```
The regex /^[A-Za-z_]\w*$/ matched ~90% of tokens; the parser then called
build(node, {depth: n+1, flags: FAST|SAFE}) and returned <Result> objects
whose .cost fields (measured in $ & %) were summed as: total = a*b + c/d - e.
"Done!" she said — or was it 'done'? Either way: exit(0); # and we're out.
```

---

### Placement review checklist

As you go, jot which keys felt wrong so you can revisit the keymap:

- [ ] Any symbol you reached for on the **wrong hand**?
- [ ] Any pair (`()`, `{}`, `[]`, `<>`) that didn't roll cleanly?
- [ ] `_` vs `-` — did they land where your fingers expected?
- [ ] `/` `\` `|` — the slashes and pipe, easy to confuse?
- [ ] `~` `` ` `` `^` — the "top of the pyramid" symbols?
- [ ] `:` and `;` — split across positions, any hesitation?
- [ ] Home-row mods misfiring while typing symbols fast?
```