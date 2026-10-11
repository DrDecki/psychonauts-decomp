# Psychonauts

![Lua scripts](https://img.shields.io/badge/Lua%20scripts-100%25-brightgreen)
![Engine](https://img.shields.io/badge/Engine-4%25%20(432%2F10930)-red)

A work-in-progress decompilation of Psychonauts (2005, PC).

The game is split into two very different parts:

- **The engine** (`Psychonauts.exe`): C++, 32 bit x86. Rendering, physics, audio, loading, the Lua runtime. **Just started:** the Lua runtime inside the executable comes first.
- **The scripts**: Lua 4.0 with some Double Fine changes. Raz, enemies, bosses, cutscenes, every level script. **Done:** every function decompiles to source that recompiles to the exact same bytes.

This repository does **not** contain any game assets or game code. You need your own copy of the game.

## Progress

| Part | Matching | Notes |
| --- | --- | --- |
| Lua scripts | 106607 / 106607 functions (100 %) | 9955 bytecode chunks from `.lub`, `.lpf`, `.ppf` and `.pkg` files |
| Engine | 432 / 10930 functions (3.95 %) | Lua 4.0 runtime in `src/lua`, everything else not started |

"Matching" means the decompiled function, compiled again, gives byte for byte the same code as the original. For the scripts that is Lua bytecode, for the engine x86 machine code. The tools check this for every single function, so the numbers above are not estimates.

The engine is by far the bigger part of the game, so the project as a whole is still near the start.

## Supported version

Only the current Steam version has been tested. Its executable:

```
Psychonauts.exe  sha1 21e9b89c80347c8360a8ce4b17ce45e7cd1d4a07
```

The GOG release probably has the same scripts, but I haven't checked.

## Dependencies

Linux or WSL with:

- Python 3
- gcc
- for the engine: `pip install pefile capstone`, Visual C++ 2008 (see below) and Wine if you are not on WSL

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

## Building the engine

The engine has to be compiled with the same compiler Double Fine used, Visual C++ 2008 without any service pack (version 15.00.21022.08). It was part of the free Visual Studio 2008 Express Edition. The offline ISO with all Express editions has this checksum:

```
sha1 76c6d28274a67741da720744026ea991a70867d1
```

The installer doesn't like Windows 11, but the compiler doesn't need to be installed. It can be unpacked from the ISO instead:

```sh
sudo apt install p7zip-full msitools
7z x VS2008Express.iso VCExpress/Ixpvc.exe
7z x -oixp VCExpress/Ixpvc.exe
cd ixp && msiextract -C ../vc vs_setup.msi && cd ..
mv "vc/Program Files/Microsoft Visual Studio 9.0/VC:Vc7" VC
cp "vc/Program Files/Microsoft Visual Studio 9.0/Common7/IDE/"{mspdb80.dll,mspdbcore.dll,mspdbsrv.exe,msobj80.dll} VC/bin/
```

`VC:Vc7` is really the folder name, msiextract keeps the internal name from the installer. `VC/bin/cl.exe` should now print `Version 15.00.21022.08 for 80x86`.

Then build and compare:

```sh
sh tools/build.sh /path/to/VC
python3 tools/objmatch.py /path/to/Psychonauts.exe engine/functions.csv build/*/*.obj
```

`build.sh` compiles everything in `src/` into `build/`. On WSL it runs `cl.exe` directly, anywhere else through Wine. `objmatch.py` looks for every compiled function in the executable and prints how many match. With `--write` it also puts the names into `engine/functions.csv`, `-v` lists every function.

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

`tools/funclist.py` finds the functions by disassembling from the entry point, following calls, jumps and switch tables, and then looking at pointers in the data sections and the gaps in between. MSVC aligned every function in this build to 16 bytes, which makes it easy to tell real starts from data that just looks like an address. It ends up with 10930 functions covering 95.5 % of the code. Most of the rest is exception handler stubs the compiler puts at the end of `.text`, they get matched together with their parent function and are not counted on their own.

```sh
pip install capstone pefile
python3 tools/funclist.py /path/to/Psychonauts.exe engine/functions.csv
```

The game code was compiled with `/Od /Oi /MD /GS-`, so without optimizations. That is a lucky break: unoptimized code follows the source very closely, which makes a matching decompilation a lot easier than usual. Functions are padded to 16 bytes with `int3`, exactly like the compiler does it inside an object file.

### The Lua runtime

The first thing being matched is the Lua 4.0 runtime that is linked into the executable, because its source is public. `src/lua` started as the unmodified Lua 4.0 release (the first commit that touches it), everything after that are changes needed to match the executable. Comparing against 4.0.1 gives fewer matches, so Double Fine used 4.0.

Changes found so far:

- Instructions are 16 bit (`SIZE_INSTRUCTION 16`, `SIZE_B 5`), same as in the script files
- Numbers are `float`, also in the C API (`lua_tonumber`, `lua_pushnumber`, `luaL_check_number`)
- New `lua_pushint` and `lua_pushuint`, the libraries use them wherever they push an integer
- `luaM_realloc` takes the old size of the block as a fourth argument, `-1` when it isn't known
- `lua_State` has two more fields after `Mbuffsize`, not clear yet what they are
- No stack size clamping, `MAXSTACK` is 250 and `LFIELDS_PER_FLUSH` 31
- The I/O library doesn't use `errno`, errors are always "generic I/O error". Reading numbers uses `fscanf("%lf")` into a `float`, a real bug in the game that is kept as is.
- Table nodes are 12 bytes instead of 20: both values, then the two type tags as bytes, then the index of the next node as a `short`. Not done yet.
- There are two more value types, `LUA_TCLASS` and `LUA_TLSO`. Not done yet.

The rest of the engine will follow the usual setup for MSVC games: a Ghidra project to map out the functions, then decompiled C++ compiled with the original compiler and compared function by function, similar to how [isledecomp](https://github.com/isledecomp/isle) does it with `reccmp`.

## Repository layout

| Path | Contents |
| --- | --- |
| `tools/` | Script decompiler, repacker, installer, diff tool, and the tools for the executable: `peinfo.py`, `funclist.py`, `build.sh`, `objmatch.py` |
| `src/lua/` | Lua 4.0 runtime as used by the engine |
| `engine/` | Function list of `Psychonauts.exe`, with names for the functions that match |
| `psyluac/` | Lua 4.0 compiler patched to produce Psychonauts bytecode |

## Related

- [Psychonauts Lua API documentation](https://psycholuaapi.readthedocs.io/) documents the functions the engine gives to Lua
- Double Fine Explorer by Quick and Easy Software reads the `.pkg` and `.ppf` containers

## License

My code (`tools/` and `psyluac/psyluac.c`) is under the MIT license, see `LICENSE`.

The rest of `psyluac/` and all of `src/lua/` is Lua 4.0 by TeCGraf, PUC-Rio under its own license, see `COPYRIGHT` in both folders. Every changed file says so at the top. In `psyluac/` I changed `llimits.h`, `lparser.c` and `ldo.c`. In `src/lua/` the changes are Double Fine's, reconstructed from the executable (`tools/mark.py` adds the note).

Psychonauts belongs to Double Fine Productions. This project is not affiliated with or endorsed by them.
