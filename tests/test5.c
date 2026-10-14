/* equilibrio: IA contra IA em campo pequeno e grande */
#include <stdio.h>
#include <stdlib.h>
#include "../src/game.h"
extern int TEST_BTN[12];
int main(void) {
  remove("save.txt"); game_init(); dbgAllAI = 1; OPT.half = 2;
  for (int big = 0; big <= 1; big++) {
    for (int diff = 2; diff <= 2; diff += 2) {
      OPT.big = big; OPT.diff = diff;
      long g1 = 0, g2 = 0, s1 = 0, s2 = 0, p1 = 0, f0 = dbgFouls; int n = 16;
      for (int seed = 1; seed <= n; seed++) {
        srand(seed * 17 + big); newMatch(1, 2, 0, 0, 0, MODE_FRIENDLY); SCENE = SC_MATCH;
        long f = 0;
        while (SCENE == SC_MATCH && f < 60 * 400) {
          f++;
          for (int i = 0; i < 12; i++) TEST_BTN[i] = 0;
          if (M.state == ST_HALF || M.state == ST_FULL) TEST_BTN[B_CROSS] = (f % 4 == 0);
          game_frame();
        }
        g1 += RES.s1; g2 += RES.s2; s1 += RES.shots1; s2 += RES.shots2; p1 += RES.poss1;
      }
      printf("%s dif%d: gols %.1f x %.1f | chutes %.1f x %.1f | posse %ld%% | faltas/jogo %.1f\n", big ? "GRANDE" : "pequeno", diff,
             g1 / (float)n, g2 / (float)n, s1 / (float)n, s2 / (float)n, p1 / n, (dbgFouls - f0) / (float)n);
    }
  }
  return 0;
}
