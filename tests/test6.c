
#include <stdio.h>
#include <stdlib.h>
#include "../src/game.h"
extern int TEST_BTN[12];
void game_debug_set(int what, int v);
static long fr = 0;
static void tick(int pulse) { for (int i = 0; i < 12; i++) TEST_BTN[i] = 0; if (pulse && fr % 4 == 0) TEST_BTN[B_CROSS] = 1; fr++; game_frame(); }
int main(void) {
  remove("extras.txt"); remove("save.txt"); game_init(); OPT.half = 1; dbgAllAI = 1;
  int rules_done = 0;
  for (int rule = 0; rule < 4; rule++) {
    srand(40 + rule); NextRule = rule; newMatch(0, 3, 0, 0, 0, MODE_FRIENDLY); SCENE = SC_MATCH;
    long f = 0;
    while (SCENE == SC_MATCH && f < 60 * 400) { f++; tick(M.state == ST_HALF || M.state == ST_FULL); }
    if (SCENE == SC_RESULT) { rules_done++; printf("regra %d: %d x %d (min %d)\n", rule, RES.s1, RES.s2, RES.minute); tick(1); }
    SCENE = SC_MENU;
  }
  if (rules_done != 4) { printf("FALHA regras\n"); return 1; }
  int stars = 0;
  for (int id = 0; id < 6; id++) {
    srand(70 + id); NextChal = id; newMatch(0, 2, 0, 0, 0, MODE_FRIENDLY); SCENE = SC_MATCH;
    long f = 0;
    while (SCENE == SC_MATCH && f < 60 * 400) { f++; tick(M.state == ST_HALF || M.state == ST_FULL); }
    if (SCENE != SC_RESULT) { printf("FALHA desafio %d (%d)\n", id, SCENE); return 2; }
    printf("desafio %d: %d x %d -> %d estrelas\n", id, RES.s1, RES.s2, RES.stars); stars += RES.stars;
    for (int k = 0; k < 6; k++) tick(1);
    SCENE = SC_MENU;
  }
  /* cenas novas */
  for (int sc = SC_PREMATCH; sc <= SC_STATS; sc++) { SCENE = sc; for (int k = 0; k < 40; k++) tick(0); }
  for (int th = 0; th < 8; th++) { OPT.stadium = th; OPT.big = 1; newMatch(1, 9, 0, 0, 0, MODE_FRIENDLY); SCENE = SC_MATCH; for (int k = 0; k < 200; k++) tick(0); OPT.big = 0; newMatch(1, 9, 0, 0, 0, MODE_FRIENDLY); for (int k = 0; k < 100; k++) tick(0); }
  remove("extras.txt");
  return 0;
}
