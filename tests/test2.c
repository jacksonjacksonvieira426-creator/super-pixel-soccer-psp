#include <stdio.h>
#include <stdlib.h>
#include "../src/game.h"
extern int TEST_BTN[12]; extern int TEST_AX, TEST_AY;
int game_debug(int what);
static long fr = 0;
static void tick(int pulse) {
  for (int i = 0; i < 12; i++) TEST_BTN[i] = 0;
  if (pulse && (fr % 4 == 0)) TEST_BTN[B_CROSS] = 1;
  fr++; game_frame();
}
static void tap(int b) { for (int i = 0; i < 12; i++) TEST_BTN[i] = 0; game_frame(); TEST_BTN[b] = 1; game_frame(); TEST_BTN[b] = 0; game_frame(); }
static int mpos = 0;
static void gotoItem(int i) { SCENE = SC_MENU; int n = (i - mpos + 8) % 8; for (int k = 0; k < n; k++) tap(B_DOWN); mpos = i; tap(B_CROSS); }
int main(int argc, char **argv) {
  remove("career.dat"); remove("manager.dat"); game_init(); OPT.half = 1; OPT.big = (argc > 1);
  /* liga completa com humano passivo */
  srand(3); SCENE = SC_MENU;
  newMatch(0, 1, 0, 0, 0, MODE_LEAGUE); /* garante que newMatch funciona */
  SCENE = SC_MENU;
  gotoItem(1);
  tap(B_CROSS);                              /* time 0 */
  if (SCENE != SC_LEAGUE) { printf("FALHA: nao entrou na liga (%d)\n", SCENE); return 1; }
  long guard = 0;
  while (!game_debug(0) && guard < 2000000) { tick(1); guard++; }
  printf("liga: done=%d rodada=%d jogos=%d frames=%ld\n", game_debug(0), game_debug(1), game_debug(5), guard);
  if (!game_debug(0) || game_debug(5) != 132) return 2;
  tap(B_CROSS);
  if (SCENE != SC_MENU) { printf("FALHA: liga nao voltou ao menu (%d)\n", SCENE); return 3; }
  /* copas */
  int champ = 0, elim = 0;
  for (int seed = 1; seed <= 12; seed++) {
    srand(seed);
    gotoItem(2); tap(B_CROSS);
    if (SCENE != SC_CUP) { printf("FALHA: copa nao iniciou (%d)\n", SCENE); return 4; }
    guard = 0;
    while (!game_debug(2) && game_debug(3) && guard < 3000000) { tick(1); guard++; }
    if (game_debug(2)) champ++; else elim++;
    if (guard >= 3000000) { printf("FALHA: copa travou seed %d cena %d\n", seed, SCENE); return 5; }
    tap(B_CROSS);
    if (SCENE != SC_MENU) { printf("FALHA: copa nao voltou (%d)\n", SCENE); return 6; }
  }
  printf("copa: campeao=%d eliminado=%d\n", champ, elim);
  /* penaltis puros, 10 vezes */
  for (int k = 0; k < 10; k++) {
    srand(100 + k);
    gotoItem(4); tap(B_CROSS);
    if (SCENE != SC_TEAMSEL) { printf("FALHA: escolha do adversario (%d)\n", SCENE); return 7; }
    tap(B_CROSS);
    if (SCENE != SC_PENS) { printf("FALHA: penaltis nao iniciou (%d)\n", SCENE); return 8; }
    guard = 0;
    while (SCENE == SC_PENS && guard < 200000) { tick(1); guard++; }
    if (SCENE != SC_MENU) { printf("FALHA: penaltis nao terminou (%d)\n", SCENE); return 9; }
  }
  printf("penaltis ok\n");
  return 0;
}
