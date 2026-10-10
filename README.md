# psychonauts-decomp

Tools to turn the Lua scripts of Psychonauts (2005, PC) back into readable source code. Not "roughly what it did" source, but source that compiles back to the exact same bytes as the original.

Psychonauts runs a huge part of its gameplay in Lua. Raz, the enemies, the bosses, the cutscenes, every level script. The engine itself is C++, everything on top of it is Lua 4.0 with a few Double Fine tweaks.

With the current Steam version all of it goes through:

```
bytecode chunks 9955, plain text sources 2, failed 0
functions 106607, byte identical 106607
MATCHING: 100.0 %
```

Every single function gets decompiled, recompiled and compared byte for byte against the original. If something does not match, the tool tells you.

**This repo does not contain any game files or decompiled code.** You need your own copy of Psychonauts. The tools read it and write the source to your disk.

## What's in here

| Path | What it does |
| --- | --- |
| `tools/psydec.py` | The decompiler. Reads `.lub`, `.lpf`, `.ppf` and `.pkg` files and writes `.lua` source |
| `psyluac/` | Lua 4.0 compiler patched to produce Psychonauts bytecode. Used to verify the output and to build mods |
| `tools/psypack.py` | Puts edited scripts back into the game files |
| `tools/psyinstall.py` | Installs those files into the game folder and keeps a backup of the originals |
| `tools/compare.py` | Shows original and recompiled bytecode of one function side by side, handy if you work on the decompiler |

## Usage

You need Linux or WSL, Python 3 and gcc.

Build the compiler:

```sh
sh psyluac/build.sh
```

Decompile everything (change the path to wherever your game is):

```sh
python3 tools/psydec.py "/mnt/e/SteamLibrary/steamapps/common/Psychonauts" decomp psyluac/psyluac
```

This takes a few minutes. Afterwards you get:

- `decomp/lua/` with all the scripts, the folders follow the game files, e.g. `decomp/lua/WorkResource/PCLevelPackFiles/ASCO/...` for the level `ASCO.ppf`
- `decomp/report.txt` with the result for every chunk
- `decomp/manifest.json`, which `psypack.py` needs later
- `decomp/failed/` only shows up if something did not match

If you leave out the `psyluac/psyluac` argument it only decompiles and skips the check.

## Changing things (experimental)

Edit a `.lua` file in `decomp/lua`, then:

```sh
G="/path/to/Psychonauts"
python3 tools/psypack.py "$G" decomp psyluac/psyluac mod backup
python3 tools/psyinstall.py install "$G" mod backup
```

`psypack.py` only recompiles the files you touched, everything else stays original. Changed game files end up in `mod/`, and `psyinstall.py` copies them over and saves the originals to `backup/` first. To go back:

```sh
python3 tools/psyinstall.py restore "$G" backup
```

What I tested so far:

- Changing text in the localization tables works in game. My first test put my own line on the title screen.
- The game loads a rebuilt `common.ppf` fine, even when the script pack inside got shorter.
- Changing Raz's gravity in `Dart.lua` did nothing visible in the main menu. `Dart` exists three times (`Scripts/packfile/common.lpf`, `PCLevelPackFiles/common.ppf` and a loose copy in `Psychonautsdata2.pkg`), so it is not clear yet which one the game really uses where.

The `.lua` files are Latin-1 like the originals. If you edit them on Windows, save as "ANSI" or "Western (Windows 1252)", otherwise umlauts and accents break.

## How Psychonauts' Lua differs from stock Lua 4.0

This was the fun part. Stock tools can't read the bytecode at all, here is why:

- **16 bit instructions** instead of 32 bit: opcode 6 bits, B 5 bits, A 5 bits, U 10 bits. The Lua 4.0 source even mentions this as an option "for a very small machine".
- **float** instead of double for numbers. The header test number is `3.14159`.
- **Three extra opcodes**, because 5 bits for A are not enough: `EXTARG` (49) carries a big A for the next instruction, `CALLX` (51) and `CLOSUREX` (52) take it. 50 never shows up.
- `EXTARG` kicks in at closure index 30 but only at call base 32. The 30 comes from the growth check in `luaM_growaux` (`nelems >= limit - inc`), which would have errored there.
- `MULT_RET` is 31, the stack can go up to at least 58, lists flush every 31 items and records every 15.
- String constants are never duplicated. Stock 4.0 sometimes adds a second copy when a nested function overwrote the lookup hint. Number constants are searched in full instead of the last 20.
- Scripts in the level packs have no debug info, so there are no local names or line numbers. The localization tables still have both.

## Notes on the decompiler

Lua 4.0 is a simple one pass compiler without real optimization passes, so the decompiler basically runs the parser backwards: a symbolic stack for expressions, jump targets for `if`, `while`, `repeat`, `for` and `and`/`or`.

Some things that took a while to get right:

- Jumps that land on a `JMP` get redirected to its target ("jump threading"). This also hits `FORPREP` and `else` jumps at the end of loops, so targets have to be compared by where they end up, not where they point.
- A `POP` at the end of a block, or the one a `break` emits, gets merged into a `SETTABLE` right before it.
- `if nil then ... end` and `if nil and ...` compile to a bare `JMP`. Double Fine used that to switch off debug code.
- A `return` in the middle of a block was written as `do return end`.
- `and` and `or` have the same priority in 4.0 and are left associative. `a or b and c` means `(a or b) and c`.

Since the level scripts have no debug info, parameters are called `p1`, `p2`... and locals `l1`, `l2`... The names are unique per file, otherwise upvalues (`%name`) would resolve to the wrong variable. Functions sit on the same line numbers as in the original files, which is why you'll see some blank lines.

## Status

- [x] Read all script containers of the Steam version
- [x] Decompile every function byte identical
- [x] Rebuild packs and install with backup
- [ ] Better names for parameters and locals (e.g. `self` for methods)
- [ ] Figure out which copy of a script the game loads in which situation
- [ ] Everything that isn't Lua (textures, models, sound) and the engine itself

## Related

- [Psychonauts Lua API documentation](https://psycholuaapi.readthedocs.io/) documents the functions the engine gives to Lua
- Double Fine Explorer by Quick and Easy Software reads the `.pkg` and `.ppf` containers

## License

My code (`tools/` and `psyluac/psyluac.c`) is under the MIT license, see `LICENSE`.

The rest of `psyluac/` is Lua 4.0 by TeCGraf, PUC-Rio under its own license, see `psyluac/COPYRIGHT`. I changed `llimits.h`, `lparser.c` and `ldo.c`, each of them says so at the top.

Psychonauts belongs to Double Fine Productions. This project is not affiliated with or endorsed by them.
