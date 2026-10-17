#include <stdio.h>
#include <stdlib.h>
#include "../src/game.h"
extern int TEST_BTN[12];
void rec_open(const char *f); void rec_close(void);
void game_debug_set(int what, int v);
static void tap(int b) { for (int i = 0; i < 12; i++) TEST_BTN[i] = 0; game_frame(); TEST_BTN[b] = 1; game_frame(); TEST_BTN[b] = 0; game_frame(); }
static void snap(const char *name) { char p[96]; snprintf(p, sizeof p, "/tmp/ops_%s.txt", name); rec_open(p); for (int i = 0; i < 12; i++) TEST_BTN[i] = 0; game_frame(); rec_close(); }
int main(void) {
  remove("save.txt"); remove("career.dat"); remove("manager.dat"); srand(5);
  game_init(); OPT.half = 2; dbgAllAI = 1;
  OPT.big = 1; newMatch(1, 9, 0, 0, 0, MODE_FRIENDLY); SCENE = SC_MATCH;
  for (int f = 0; f < 60 * 22; f++) { for (int i = 0; i < 12; i++) TEST_BTN[i] = 0; game_frame(); }
  snap("big");
  OPT.big = 0; newMatch(14, 20, 0, 0, 0, MODE_FRIENDLY); SCENE = SC_MATCH;
  for (int f = 0; f < 60 * 14; f++) { for (int i = 0; i < 12; i++) TEST_BTN[i] = 0; game_frame(); }
  snap("small");
  newMatch(0, 22, 1, 0, 0, MODE_FRIENDLY); SCENE = SC_MATCH;
  for (int f = 0; f < 60 * 10; f++) { for (int i = 0; i < 12; i++) TEST_BTN[i] = 0; game_frame(); }
  snap("futsal");
  dbgAllAI = 0;
  OPT.big = 1; OPT.stadium = 2; dbgAllAI = 1; newMatch(1, 9, 0, 0, 0, MODE_FRIENDLY); SCENE = SC_MATCH; for (int f = 0; f < 60 * 8; f++) { for (int i = 0; i < 12; i++) TEST_BTN[i] = 0; game_frame(); } snap("neve");
  OPT.stadium = 6; newMatch(1, 9, 0, 0, 0, MODE_FRIENDLY); for (int f = 0; f < 60 * 8; f++) { for (int i = 0; i < 12; i++) TEST_BTN[i] = 0; game_frame(); } snap("crep");
  OPT.big = 0; OPT.stadium = 7; newMatch(1, 9, 0, 0, 0, MODE_FRIENDLY); for (int f = 0; f < 60 * 6; f++) { for (int i = 0; i < 12; i++) TEST_BTN[i] = 0; game_frame(); } snap("chuva");
  OPT.big = 0; dbgAllAI = 1;
  for (int th = 8; th < 14; th++) { char nm[16]; snprintf(nm, sizeof nm, "st%d", th); OPT.stadium = th; newMatch(1, 9, 0, 0, 0, MODE_FRIENDLY); SCENE = SC_MATCH; for (int f = 0; f < 60 * 9; f++) { for (int i = 0; i < 12; i++) TEST_BTN[i] = 0; game_frame(); } snap(nm); }
  OPT.big = 1; OPT.stadium = 10; newMatch(1, 9, 0, 0, 0, MODE_FRIENDLY); for (int f = 0; f < 60 * 8; f++) { for (int i = 0; i < 12; i++) TEST_BTN[i] = 0; game_frame(); } snap("stbig");
  dbgAllAI = 0; OPT.stadium = 0;
  PRE_dummy: ;
  SCENE = SC_MENU; snap("menu"); SCENE = SC_CHAL; snap("chal"); SCENE = SC_STATS; snap("stats"); SCENE = SC_TRAINSEL; snap("trainsel"); SCENE = SC_PREMATCH; snap("prematch");
  SCENE = SC_TACTICS; snap("tactics");
  SCENE = SC_MENU; game_debug_set(0, 3); tap(B_CROSS); TS_dummy: ;
  snap("teamsel");
  tap(B_CROSS); snap("coach"); tap(B_CROSS); snap("hub");
  MGR.sub = 2; snap("lineup"); MGR.sub = 5; snap("market"); MGR.sub = 1; snap("squad"); MGR.sub = 8; MGR.trophies[0] = 2; MGR.trophies[1] = 3; snap("trophies");
  return 0;
}
