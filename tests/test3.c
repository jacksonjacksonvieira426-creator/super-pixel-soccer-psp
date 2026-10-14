#include <stdio.h>
#include <stdlib.h>
#include "../src/game.h"
extern int TEST_BTN[12];
int game_debug(int what);
void game_debug_set(int what, int v);
static long fr = 0;
static void tick(int pulse) { for (int i = 0; i < 12; i++) TEST_BTN[i] = 0; if (pulse && fr % 4 == 0) TEST_BTN[B_CROSS] = 1; fr++; game_frame(); }
static void tap(int b) { for (int i = 0; i < 12; i++) TEST_BTN[i] = 0; game_frame(); TEST_BTN[b] = 1; game_frame(); TEST_BTN[b] = 0; game_frame(); }
int main(void) {
  remove("career.dat"); remove("manager.dat"); game_init(); OPT.half = 1; srand(9);
  SCENE = SC_MENU; game_debug_set(0, 1); tap(B_CROSS); tap(B_CROSS);      /* CAMPEONATO -> time 0 */
  if (SCENE != SC_LEAGUE) { printf("FALHA: liga nao iniciou (%d)\n", SCENE); return 1; }
  long g = 0;
  while (game_debug(1) < 2 && g < 400000) { tick(1); g++; }
  int round = game_debug(1);
  printf("rodada=%d save=%d\n", round, game_debug(6));
  if (round < 2 || game_debug(6) != 1) return 2;
  game_debug_set(1, 1);                       /* apaga o progresso da memoria */
  SCENE = SC_MENU; game_frame();
  game_debug_set(0, 0); tap(B_CROSS);          /* CONTINUAR */
  printf("continuar: cena=%d rodada=%d\n", SCENE, game_debug(1));
  int ok = (SCENE == SC_LEAGUE && game_debug(1) == round);
  remove("career.dat");
  return ok ? 0 : 3;
}
