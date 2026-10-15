#!/bin/sh
# Testes no PC (sem PSP): compila o motor com detector de erros e roda os fluxos.
cd "$(dirname "$0")/.." || exit 1
F="-std=gnu11 -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=undefined -Wall -Wextra -Wno-unused-parameter"
for t in test test2 test3 test4 test6; do
  gcc $F src/game_core.c src/game_draw.c src/game_mgr.c src/game_ui.c src/plat_null.c tests/$t.c -lm -o /tmp/sps_$t && /tmp/sps_$t || exit 1
done
gcc $F src/game_core.c src/game_draw.c src/game_mgr.c src/game_ui.c src/plat_null.c tests/test2.c -lm -o /tmp/sps_test2b && /tmp/sps_test2b big || exit 1
rm -f career.dat manager.dat save.txt
