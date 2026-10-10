# Psychonauts

![Lua scripts](https://img.shields.io/badge/Lua%20scripts-100%25-brightgreen)
![Engine](https://img.shields.io/badge/Engine-0%25-red)

A work-in-progress decompilation of Psychonauts (2005, PC).

The game is split into two very different parts:

- **The engine** (`Psychonauts.exe`): C++, 32 bit x86. Rendering, physics, audio, loading, the Lua runtime. **Not started yet.**
- **The scripts**: Lua 4.0 with some Double Fine changes. Raz, enemies, bosses, cutscenes, every level script. **Done:** every function decompiles to source that recompiles to the exact same bytes.

This repository does **not** contain any game assets or game code. You need your own copy of the game.

## Progress

| Part | Matching | Notes |
| --- | --- | --- |
| Lua scripts | 106607 / 106607 functions (100 %) | 9955 bytecode chunks from `.lub`, `.lpf`, `.ppf` and `.pkg` files |
| Engine | 0 % | Nothing decompiled yet |

"Matching" means the decompiled function, compiled again, gives byte for byte the same bytecode as the original. The tools check this for every single function, so the number above is not an estimate.

The engine is by far the bigger part of the game, so the project as a whole is still near the start.

## Supported version

Only the current Steam version has been tested (the one with the 2011 update that added Mac support and achievements). The GOG release probably works the same, but I haven't checked.

## Dependencies

Linux or WSL with:

- Python 3
- gcc

## Decompiling the scripts

Build the patched Lua compiler:

```sh
sh psyluac/build.sh
```

Then point the decompiler at your game folder:

```sh
python3 tools/psydec.py "/path/to/Psychonauts" decomp psyluac/psyluac
```

After a few minutes you get:

- `decomp/lua/` with all scripts, laid out like the game files (`decomp/lua/WorkResource/PCLevelPackFiles/ASCO/...` for `ASCO.ppf` and so on)
- `decomp/report.txt` with the result for every chunk
- `decomp/manifest.json`, needed for repacking
- `decomp/failed/` with the original bytecode of anything that didn't match (should not exist)

Without the `psyluac/psyluac` argument the output is not checked.

## Diffing

To look at a single function, original and recompiled side by side:

```sh
PSYLUAC=psyluac/psyluac python3 tools/compare.py original.lub decompiled.lua main.0.5
```

`main.0.5` is the path through the nested functions: the sixth function inside the first function inside the main chunk.

## Repacking (experimental)

Edited scripts can go back into the game files:

```sh
python3 tools/psypack.py "/path/to/Psychonauts" decomp psyluac/psyluac mod backup
python3 tools/psyinstall.py install "/path/to/Psychonauts" mod backup
python3 tools/psyinstall.py restore "/path/to/Psychonauts" backup
```

`psypack.py` only recompiles the files you changed. `psyinstall.py` saves the originals to `backup/` before overwriting anything.

Tested so far: text changes in the localization tables show up in game, and a rebuilt `common.ppf` with a shorter script pack loads fine. A change to Raz's gravity in `Dart.lua` showed no effect in the main menu. There are three copies of `Dart` in the game files and it's not clear yet which one is used when.

The `.lua` files are Latin-1 like the originals, so save them as "ANSI" or "Western (Windows 1252)" if you edit on Windows.

## About the script format

Stock Lua tools can't read Psychonauts bytecode. What's different from Lua 4.0:

- 16 bit instructions instead of 32 bit (opcode 6 bits, B 5, A 5, U 10)
- `float` numbers instead of `double`, header test number `3.14159`
- Three extra opcodes for arguments that don't fit into 5 bits: `EXTARG` (49) carries a big A for the next instruction, `CALLX` (51) and `CLOSUREX` (52) use it. Closures switch to it at index 30, calls at base 32
- `MULT_RET` is 31, lists flush every 31 items, records every 15
- String constants are never duplicated and number constants are searched in full, both differ from stock 4.0
- Level scripts have no debug info, only the localization tables keep local names and line numbers

The decompiler basically runs the Lua 4.0 parser backwards. Some things that took a while: jump threading (jumps onto a `JMP` get redirected to its target), `POP`s merged into a preceding `SETTABLE`, `if nil then` compiling to a bare `JMP`, and `and`/`or` having the same priority in 4.0.

Since the level scripts have no debug info, the original names of parameters and locals are gone for good, and so are the comments. Everything else (globals, table fields, function names, strings, control flow) is original. The decompiler picks names that follow Double Fine's own conventions where it can tell: `self` for methods, `Ob` for the object in class constructors, `k`/`v` and `i` for loops. Everything else gets `a1`, `a2`... for parameters and `l1`, `l2`... for locals, counted per function. Every file is recompiled with these names and falls back to plain generated names if anything would change, so the output always matches.

The blank lines between functions are on purpose. Each function's starting line is stored in the bytecode, so the functions have to stay on their original lines.

## About the engine

What we know so far, from `tools/peinfo.py`:

- The Steam executable was rebuilt in January 2016 and linked with Visual Studio 2008 (linker 9.00).
- The game code itself is 218 C++ and 45 C object files, all compiled with MSVC 2008 RTM (build 15.00.21022). That compiler shipped with the free Visual C++ 2008 Express, so a matching decompilation is possible in principle.
- Besides the usual sections there are two custom ones, `.dflua` and `.dfluatx`, probably Double Fine's Lua bindings.
- No PDB is shipped, only its build path `c:\dfp-p5ya\Psychonauts\Resource\Psychonauts.pdb` is left in the executable.

The plan is the usual setup for MSVC games: a Ghidra project to map out the functions, then decompiled C++ compiled with the original compiler and compared function by function, similar to how [isledecomp](https://github.com/isledecomp/isle) does it with `reccmp`.

## Roadmap

- [x] Read all script containers of the Steam version
- [x] Match every Lua function
- [x] Repack and install edited scripts
- [x] Readable names for parameters and locals where the role is clear
- [ ] Find out which copy of a script the game loads where
- [ ] Set up the engine: Ghidra project, function list, symbols
- [ ] Start decompiling the engine
- [ ] Asset formats (textures, models, sound)

## Repository layout

| Path | Contents |
| --- | --- |
| `tools/` | Decompiler, repacker, installer, diff tool and `peinfo.py` for the executable (Python) |
| `psyluac/` | Lua 4.0 compiler patched to produce Psychonauts bytecode |

## Related

- [Psychonauts Lua API documentation](https://psycholuaapi.readthedocs.io/) documents the functions the engine gives to Lua
- Double Fine Explorer by Quick and Easy Software reads the `.pkg` and `.ppf` containers

## License

My code (`tools/` and `psyluac/psyluac.c`) is under the MIT license, see `LICENSE`.

The rest of `psyluac/` is Lua 4.0 by TeCGraf, PUC-Rio under its own license, see `psyluac/COPYRIGHT`. I changed `llimits.h`, `lparser.c` and `ldo.c`, each of them says so at the top.

Psychonauts belongs to Double Fine Productions. This project is not affiliated with or endorsed by them.
