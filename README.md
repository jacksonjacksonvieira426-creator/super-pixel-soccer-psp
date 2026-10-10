# Super Pixel Soccer PSP
Jogo de futebol 2D de fas, nao oficial, em C (PSPSDK + SDL2).

Modos: Amistoso, Campeonato, Copa, Penaltis, Futsal, Treino, Opcoes.
Controles: direcional/analogico move | X passe | O chute (segure) | [] lob | R correr | START pausa.

- `src/`    codigo do jogo (game_core.c = partida, game_ui.c = menus/modos, plat_sdl.c = PSP)
- `gfx/` `sfx/`  sprites, fundos e sons
- `lua/`    versao antiga para Lua Player
- `tests/`  testes no PC (`sh tests/run_tests.sh`)
- `.github/workflows/build.yml`  compila o EBOOT.PBP no GitHub Actions

Baixe em Actions > ultimo build > Artifacts > SuperPixelSoccer-PSP.
Copie a pasta SuperPixelSoccer para ms0:/PSP/GAME/ ou abra o EBOOT.PBP no PPSSPP.
