#!/bin/sh
# Compiles everything in src/ (C and C++) with Visual C++ 2008, the compiler Psychonauts.exe
# was built with. The objects end up in build/<folder>/.
#
# usage: sh tools/build.sh <VC folder>
#
# <VC folder> is the one with bin/cl.exe and include/ in it. On WSL cl.exe runs
# directly, everywhere else through Wine.
set -e
VC=$(cd "${1:?usage: sh tools/build.sh <VC folder of Visual C++ 2008>}" && pwd)
ROOT=$(cd "$(dirname "$0")/.." && pwd)

if command -v wslpath >/dev/null 2>&1; then
    RUN=
    INCLUDE=$(wslpath -w "$VC/include")
else
    RUN=wine
    export WINEDEBUG=-all
    INCLUDE=$(winepath -w "$VC/include" 2>/dev/null)
fi

# no optimization, intrinsics on, dynamic CRT (msvcr90.dll), no stack cookies
CFLAGS="/nologo /c /Od /Ob1 /Oi /MD /GS-"

for dir in "$ROOT"/src/*/; do
    name=$(basename "$dir")
    rm -rf "$ROOT/build/$name"
    mkdir -p "$ROOT/build/$name"
    cd "$ROOT/build/$name"
    $RUN "$VC/bin/cl.exe" $CFLAGS /I"$INCLUDE" $(ls ../../src/"$name"/*.c ../../src/"$name"/*.cpp 2>/dev/null) | grep -vE '^(Compiling|Generating Code)|\.(c|cpp)\s*$|C4005|definition of .errno.' || true
done
