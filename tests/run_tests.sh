#!/bin/sh
# Testes no PC (sem PSP): compila o motor com detector de erros e roda os fluxos.
cd "$(dirname "$0")/.." || exit 1
F="-std=gnu11 -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=undefined -Wall -Wextra -Wno-unused-parameter"
gcc $F src/game_core.c src/game_ui.c src/plat_null.c tests/test.c -lm -o /tmp/sps_fuzz && /tmp/sps_fuzz || exit 1
gcc $F src/game_core.c src/game_ui.c src/plat_null.c tests/test2.c -lm -o /tmp/sps_flow && /tmp/sps_flow || exit 1
