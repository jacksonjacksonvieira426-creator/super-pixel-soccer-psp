#include <stdio.h>
#include <stdlib.h>
#include "../src/game.h"
extern int TEST_BTN[12]; extern int TEST_AX, TEST_AY; extern long TEST_BLITS, TEST_RECTS;
static void frame(void) { game_frame(); }
int main(void) {
  game_init();
  long frames = 0; int seen[16] = {0};
  for (int seed = 1; seed <= 6; seed++) {
    srand(seed); SCENE = SC_TITLE; OPT.big = (seed % 2 == 0);
    for (long f = 0; f < 250000; f++) {
      if (f % 6 == 0) {
        for (int i = 0; i < 12; i++) TEST_BTN[i] = 0;
        int k = rand() % 16; if (k < 12) TEST_BTN[k] = 1;
        if (rand() % 3 == 0) TEST_BTN[rand() % 12] = 1;
        TEST_AX = (rand() % 3 == 0) ? (rand() % 256 - 128) : 0; TEST_AY = (rand() % 3 == 0) ? (rand() % 256 - 128) : 0;
        TEST_BTN[B_START] = (rand() % 400 == 0);
      }
      if (SCENE == -1) SCENE = SC_MENU;
      seen[SCENE]++;
      frame(); frames++;
    }
  }
  printf("faltas=%d amarelos=%d vermelhos=%d penaltis=%d\n", dbgFouls, dbgYellows, dbgReds, dbgPens);
  printf("frames=%ld blits=%ld rects=%ld\n", frames, TEST_BLITS, TEST_RECTS);
  for (int i = 0; i < 12; i++) printf("cena%d=%d ", i, seen[i]);
  printf("\n");
  return 0;
}
