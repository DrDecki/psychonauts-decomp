#!/bin/sh
cd "$(dirname "$0")" && gcc -O1 -w -DLUA_NUM_TYPE=float -o psyluac \
  lapi.c lcode.c ldebug.c ldo.c lfunc.c lgc.c llex.c lmem.c lobject.c lparser.c \
  lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c psyluac.c -lm
