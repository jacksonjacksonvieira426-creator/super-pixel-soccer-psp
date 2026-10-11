# Super Pixel Soccer PSP - Edicao Definitiva
Jogo de futebol 2D de fas, nao oficial, em C (PSPSDK + SDL2).

## Novidades
- 21 times: 8 clubes dos sprites originais, 12 times novos com jogadores e uniformes desenhados do zero, e o time LENDAS
- Jogadores com nome, velocidade, chute e marcacao diferentes
- 4 formacoes no 6x6 e 3 no futsal (escolha na tela do time)
- Campeonato de 12 times (11 rodadas) e Copa de 16 times, com CONTINUAR (progresso salvo)
- 4 estadios: dia, noite, neve e areia (Opcoes)
- Faltas, cartoes amarelo e vermelho (2 amarelos = vermelho), tiro livre e penalti na partida
- Posse de bola e chutes no resultado, musica em loop e torcida
- Faltas, cartoes amarelo e vermelho, tiro livre e penalti marcado dentro da area
- Sprites novos enviados pelo autor (parado e correndo); 12 times usam esses sprites recoloridos, cada um com 2 jogadores

## Modos
Amistoso, Campeonato, Copa, Penaltis, Futsal, Treino, Opcoes.
Controles: direcional/analogico move | X passe | O chute (segure) | [] lob | R correr | START pausa.

## Pastas
- `src/`    codigo (game_core.c = partida, game_ui.c = menus e modos, plat_sdl.c = PSP)
- `gfx/` `sfx/`  sprites, fundos e sons (gerados por script, sem material de terceiros nos times novos)
- `lua/`    versao antiga para Lua Player
- `tests/`  testes no PC (`sh tests/run_tests.sh`)
- `.github/workflows/build.yml`  compila o EBOOT.PBP no GitHub Actions

Baixe em Actions > ultimo build > Artifacts > SuperPixelSoccer-PSP.
Copie a pasta SuperPixelSoccer para ms0:/PSP/GAME/ ou abra o EBOOT.PBP no PPSSPP.
