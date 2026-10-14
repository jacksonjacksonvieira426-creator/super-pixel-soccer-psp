/* manager: temporada completa passando pelos menus + compra/venda + salvar/continuar */
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
  remove("career.dat"); remove("manager.dat"); game_init(); OPT.half = 1; srand(21);
  /* menu: sem saves -> itens: 0 AMISTOSO 1 CAMPEONATO 2 COPA 3 MODO MANAGER */
  SCENE = SC_MENU; game_debug_set(0, 3); tap(B_CROSS);
  if (SCENE != SC_TEAMSEL) { printf("FALHA: teamsel (%d)\n", SCENE); return 1; }
  tap(B_CROSS);
  if (SCENE != SC_MGR) { printf("FALHA: manager nao abriu (%d)\n", SCENE); return 2; }
  tap(B_CROSS);                         /* confirma treinador */
  if (!MGR.active || MGR.sub != 0) { printf("FALHA: sub=%d\n", MGR.sub); return 3; }
  /* mercado: comprar e vender */
  int n0 = MGR.nsq, money0 = MGR.money;
  MGR.sub = 5;
  for (int k = 0; k < 6; k++) tap(B_CROSS);                   /* tenta comprar */
  printf("elenco %d -> %d, caixa %d -> %d\n", n0, MGR.nsq, money0, MGR.money);
  tap(B_RIGHT);                                               /* aba vender */
  for (int k = 0; k < 4; k++) tap(B_CROSS);
  MGR.sub = 0;
  if (MGR.nsq < 14) { printf("FALHA: elenco minimo violado\n"); return 4; }
  /* escalacao: troca de jogadores */
  MGR.sub = 2; tap(B_CROSS); tap(B_DOWN); tap(B_CROSS); tap(B_SQUARE); MGR.sub = 0;
  /* tatica e treino */
  MGR.sub = 3; tap(B_RIGHT); tap(B_DOWN); tap(B_RIGHT); tap(B_TRIANGLE); MGR.sub = 4; tap(B_DOWN); tap(B_CROSS); tap(B_TRIANGLE);
  /* joga a temporada: metade simulando, metade jogando */
  long g = 0; int rounds = 0;
  while (!game_debug(0) && g < 3000000 && rounds < 30) {
    SCENE = SC_MGR; MGR.sub = 0;
    if (rounds % 2 == 0) { game_debug_set(0, 0); MGR.sub = 0; tap(B_DOWN); if (0) {} }
    /* escolhe item pelo indice: 0 jogar, 1 simular */
    extern int hubSelDebug;
    hubSelDebug = (rounds % 2 == 0) ? 0 : 1;
    tap(B_CROSS);
    g = 0;
    while ((SCENE == SC_MATCH || SCENE == SC_RESULT || SCENE == SC_PENS) && g < 200000) { tick(1); g++; }
    rounds++;
  }
  printf("temporada: rodadas=%d done=%d pos=%d money=%d\n", rounds, game_debug(0), leagueUserPos(), MGR.money);
  if (!game_debug(0)) return 5;
  SCENE = SC_MGR; tap(B_CROSS); tap(B_CROSS);                  /* tela de fim + nova temporada */
  printf("nova temporada: %d  titulos=%d\n", MGR.season, MGR.trophies[0]);
  if (MGR.season != 2) return 6;
  /* salvar e continuar */
  mgrSave(); int money = MGR.money; MGR.active = 0; MGR.money = -1;
  if (!mgrLoad() || MGR.money != money) { printf("FALHA: load\n"); return 7; }
  remove("manager.dat");
  return 0;
}
