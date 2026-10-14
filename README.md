# Super Pixel Soccer PSP - Edicao Definitiva
Jogo de futebol 2D de fas, nao oficial, em C (PSPSDK + SDL2).

## Novidades desta versao
- **Campo grande 11x11** com camera acompanhando a bola e minimapa (Opcoes > CAMPO)
- **Taticas**: formacao, mentalidade, pressao, largura e marcacao (menu TATICA ou START > TATICA no jogo)
- **IA melhor**: passes pelo espaco livre, apoio por funcao, goleiro por angulo, pressao em grupo
- **Resistencia** dos jogadores e **substituicoes** (3 por jogo) no menu de pausa
- **Modo Manager**: elenco, escalacao, treino, mercado, financas, metas da diretoria, temporadas, sala de trofeus e salvar/continuar
- Animacoes novas: chute, defesa do goleiro, torcida reagindo, 4 bolas, estadios
- 25 times (clubes dos sprites enviados + 12 times recoloridos + LENDAS)

## Modos
Amistoso, Campeonato (12 times), Copa (16 times), Modo Manager, Penaltis, Futsal, Treino, Tatica, Opcoes.
Controles: direcional/analogico move | X passe | O chute (segure) | [] lob | R correr | START pausa.

## Pastas
- `src/`    codigo (game_core.c = partida, game_draw.c = desenho/pausa, game_mgr.c = manager, game_ui.c = menus, plat_sdl.c = PSP)
- `gfx/` `sfx/`  sprites, fundos e sons
- `lua/`    versao antiga para Lua Player
- `tests/`  testes no PC (`sh tests/run_tests.sh`)
- `.github/workflows/build.yml`  compila o EBOOT.PBP no GitHub Actions

Baixe em Actions > ultimo build > Artifacts > SuperPixelSoccer-PSP.
Copie a pasta SuperPixelSoccer para ms0:/PSP/GAME/ ou abra o EBOOT.PBP no PPSSPP.
