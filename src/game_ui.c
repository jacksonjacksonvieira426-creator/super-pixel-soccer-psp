/* Super Pixel Soccer PSP - pênaltis, campeonato, copa, menus */
#include "game.h"
#include <time.h>

/* ============================================================ PENALTIS */
enum { PH_AIM, PH_POWER, PH_SHOOT, PH_RESULT, PH_END };
typedef struct {
  int team[3], kicks[3], goals[3], hist[3][40];
  int turn, ph, aimZ, st, t, mode;
  float aimY, power, pdir, gkY, shotY, shotZ, gkDive, gkDiveZ;
  char res[40];
  int active;
} Pens;
static Pens PEN;

static void newPens(int t1, int t2, int mode) {
  memset(&PEN, 0, sizeof(PEN));
  PEN.team[1] = t1; PEN.team[2] = t2; PEN.turn = 1; PEN.ph = PH_AIM;
  PEN.aimY = 200; PEN.aimZ = 1; PEN.gkY = FW / 2; PEN.shotY = 200; PEN.shotZ = 6;
  PEN.mode = mode; PEN.active = 1;
  SCENE = SC_PENS;
}

static int pensWinner(void) {
  int k1 = PEN.kicks[1], k2 = PEN.kicks[2], g1 = PEN.goals[1], g2 = PEN.goals[2];
  if (k1 <= 5 && k2 <= 5) {
    if (g1 > g2 + (5 - k2)) return 1;
    if (g2 > g1 + (5 - k1)) return 2;
    if (k1 == 5 && k2 == 5 && g1 != g2) return (g1 > g2) ? 1 : 2;
    return 0;
  }
  if (k1 == k2 && g1 != g2) return (g1 > g2) ? 1 : 2;
  return 0;
}

static const char *penResolve(float shotY, float shotZ, float power, float gkY, float gkZ) {
  if (shotY < GOAL_Y1 + 4 || shotY > GOAL_Y2 - 4) return "FORA!";
  if (power > 0.97f && shotZ > 15) return "NA TRAVE... PRA FORA!";
  int hitY = fabsf(gkY - shotY) < (18 + OPT.diff * 2);
  int hitZ = fabsf(gkZ - shotZ) < 18;
  if (hitY && hitZ && prob() < 0.88f) return "DEFENDEU!";
  if (power < 0.3f && fabsf(gkY - shotY) < 40) return "DEFENDEU!";
  return "GOOOL!";
}

static void penStartShot(void) { PEN.ph = PH_SHOOT; PEN.t = 0; playSnd(S_KICK); }

static void penFinishKick(const char *result) {
  int ti = PEN.turn;
  int gol = (strcmp(result, "GOOOL!") == 0);
  PEN.kicks[ti]++;
  if (gol) { PEN.goals[ti]++; playSnd(S_GOAL); }
  if (PEN.kicks[ti] <= 40) PEN.hist[ti][PEN.kicks[ti] - 1] = gol ? 2 : 1;
  snprintf(PEN.res, sizeof PEN.res, "%s", result);
  PEN.ph = PH_RESULT; PEN.st = 100;
}

static void cupPensEnd(int w);

static void scenePens(void) {
  if (!PEN.active) { SCENE = SC_MENU; return; }
  drawBG(T_BG_FIELD);
  int ti = PEN.turn, kicker = (ti == 1);
  float spotX = FL - 110, sx0, sy0, gsx, gsy;
  toScreen(spotX, FW / 2, &sx0, &sy0);
  int kitK = TEAMS[PEN.team[ti]].nkits ? TEAMS[PEN.team[ti]].kits[0] : TEAMS[PEN.team[ti]].kit;

  if (PEN.ph == PH_AIM) {
    if (kicker) {
      if (held(B_UP)) PEN.aimY = fmaxf(125, PEN.aimY - 1.6f);
      if (held(B_DOWN)) PEN.aimY = fminf(275, PEN.aimY + 1.6f);
      if (hit(B_LEFT)) PEN.aimZ = 1;
      if (hit(B_RIGHT)) PEN.aimZ = 2;
      if (hit(B_CROSS)) { PEN.ph = PH_POWER; PEN.power = 0; PEN.pdir = 1; }
      PEN.gkY = FW / 2;
    } else {
      if (held(B_UP)) PEN.aimY = fmaxf(150, PEN.aimY - 1.6f);
      if (held(B_DOWN)) PEN.aimY = fminf(250, PEN.aimY + 1.6f);
      if (hit(B_LEFT)) PEN.aimZ = 1;
      if (hit(B_RIGHT)) PEN.aimZ = 2;
      PEN.gkY = FW / 2;
      if (hit(B_CROSS)) {
        float sy = rndf(155, 245);
        if (prob() < 0.1f) sy = (prob() < 0.5f) ? 135.0f : 265.0f;
        PEN.shotY = sy; PEN.shotZ = (prob() < 0.5f) ? 6.0f : 22.0f;
        PEN.power = rndf(0.6f, 0.95f);
        PEN.gkDive = PEN.aimY; PEN.gkDiveZ = (PEN.aimZ == 2) ? 22.0f : 6.0f;
        penStartShot();
      }
    }
  } else if (PEN.ph == PH_POWER) {
    PEN.power += 0.024f * PEN.pdir;
    if (PEN.power >= 1) { PEN.power = 1; PEN.pdir = -1; }
    if (PEN.power <= 0) { PEN.power = 0; PEN.pdir = 1; }
    if (hit(B_CROSS)) {
      float e = PEN.power - 0.85f;
      PEN.shotY = PEN.aimY + rndf(-1, 1) * fabsf(e) * 70 + rndf(-4, 4);
      PEN.shotZ = (PEN.aimZ == 2) ? 22.0f : 6.0f;
      PEN.gkDive = rndf(158, 242);
      PEN.gkDiveZ = (prob() < 0.5f) ? 6.0f : 22.0f;
      penStartShot();
    }
  } else if (PEN.ph == PH_SHOOT) {
    PEN.t++;
    PEN.gkY = FW / 2 + (PEN.gkDive - FW / 2) * fminf(1.0f, PEN.t / 14.0f);
    if (PEN.t >= 38) penFinishKick(penResolve(PEN.shotY, PEN.shotZ, PEN.power, PEN.gkDive, PEN.gkDiveZ));
  } else if (PEN.ph == PH_RESULT) {
    if (--PEN.st <= 0 || hit(B_CROSS)) {
      if (pensWinner()) { PEN.ph = PH_END; PEN.st = 140; }
      else { PEN.turn = (PEN.turn == 1) ? 2 : 1; PEN.ph = PH_AIM; PEN.aimY = 200; PEN.aimZ = 1; PEN.gkY = FW / 2; }
    }
  } else if (PEN.ph == PH_END) {
    if (--PEN.st <= 0 || hit(B_CROSS)) {
      int w = pensWinner(), mode = PEN.mode;
      PEN.active = 0;
      if (mode == 1) cupPensEnd(w); else SCENE = SC_MENU;
      return;
    }
  }

  /* desenho */
  toScreen(FL - 6, PEN.gkY, &gsx, &gsy);
  drawFrame(FR_SHADOW, gsx, gsy); drawFrame(FR_RING_A, gsx, gsy);
  int diving = (PEN.ph == PH_SHOOT || PEN.ph == PH_RESULT) && fabsf(PEN.gkDive - FW / 2) > 20;
  drawFrame(KFR[KIT_GK][diving ? 1 : 0], gsx, gsy - ((diving && PEN.gkDiveZ > 15) ? 8 : 0));
  drawFrame(FR_SHADOW, sx0 - 14, sy0); drawFrame(FR_RING_H, sx0 - 14, sy0);
  drawFrame(KFR[kitK][(PEN.ph == PH_SHOOT) ? 1 : 0], sx0 - 14, sy0);
  float bx = sx0, by = sy0, bz = 0;
  if (PEN.ph == PH_SHOOT || PEN.ph == PH_RESULT) {
    float k = (PEN.ph == PH_RESULT) ? 1.0f : fminf(1.0f, PEN.t / 38.0f), ex, ey;
    toScreen(FL + 4, PEN.shotY, &ex, &ey);
    bx = sx0 + (ex - sx0) * k; by = sy0 + (ey - sy0) * k;
    bz = PEN.shotZ * sinf(k * 3.14159f * 0.5f) * 0.6f;
  }
  drawFrame(FR_SHADOW, bx, by + 1); drawFrame(FR_BALL, bx, by - bz);
  if (PEN.ph == PH_AIM || PEN.ph == PH_POWER) {
    float mx, my;
    if (kicker) toScreen(FL + 2, PEN.aimY, &mx, &my); else toScreen(FL - 6, PEN.aimY, &mx, &my);
    float hz = (PEN.aimZ == 2) ? 14.0f : 3.0f;
    box((int)mx - 4, (int)(my - hz - 4), 9, 2, C_YELLOW);
    box((int)mx - 1, (int)(my - hz - 7), 2, 9, C_YELLOW);
  }
  char buf[96];
  box(110, 0, 260, 38, COL(0, 0, 0, 170));
  snprintf(buf, sizeof buf, "%s  %d x %d  %s", TEAMS[PEN.team[1]].shortn, PEN.goals[1], PEN.goals[2], TEAMS[PEN.team[2]].shortn);
  ctxt(4, buf, C_YELLOW, 240);
  for (int t = 1; t <= 2; t++) {
    char s[96]; int n = 0;
    n += snprintf(s + n, sizeof s - n, "%s ", TEAMS[PEN.team[t]].shortn);
    int total = PEN.kicks[t] > 5 ? PEN.kicks[t] : 5;
    if (total > 14) total = 14;
    for (int i = 0; i < total && n < 80; i++) {
      int h = (i < PEN.kicks[t]) ? PEN.hist[t][i] : 0;
      n += snprintf(s + n, sizeof s - n, "%c ", h == 0 ? '.' : (h == 2 ? 'O' : 'X'));
    }
    txt(120, 14 + (t - 1) * 11, s, C_WHITE);
  }
  if (PEN.ph == PH_AIM) {
    if (kicker) ctxt(228, "CIMA/BAIXO mira, ESQ/DIR altura, X confirma", C_WHITE, 240);
    else ctxt(228, "DEFESA: CIMA/BAIXO e ESQ/DIR escolhem o canto, X pula", C_WHITE, 240);
  } else if (PEN.ph == PH_POWER) {
    box(170, 236, 140, 14, C_BLACK);
    box(172, 238, (int)(136 * PEN.power), 10, (PEN.power > 0.97f) ? C_RED : ((PEN.power > 0.7f) ? C_GREEN : C_ORANGE));
    ctxt(254, "X para chutar (ideal: perto do fim)", C_WHITE, 240);
  } else if (PEN.ph == PH_RESULT) {
    box(110, 100, 260, 36, C_DIM);
    ctxt(112, PEN.res, C_YELLOW, 240);
  } else if (PEN.ph == PH_END) {
    int w = pensWinner();
    box(100, 96, 280, 50, C_DIM);
    ctxt(104, "VENCEDOR NOS PENALTIS", C_YELLOW, 240);
    ctxt(124, TEAMS[PEN.team[w ? w : 1]].name, C_WHITE, 240);
  }
}

/* ============================================================ CAMPEONATO */
static int simGoals(float mu) {
  if (mu < 0.3f) mu = 0.3f;
  int g = 0;
  for (int i = 0; i < 8; i++) if (prob() < mu / 8.0f) g++;
  return g;
}
static void simMatch(int a, int b, int *ga, int *gb) {
  int sa = TEAMS[a].str, sb = TEAMS[b].str;
  *ga = simGoals(1.3f + (sa - sb) * 0.28f);
  *gb = simGoals(1.3f + (sb - sa) * 0.28f);
}

typedef struct { int p, w, d, l, gf, ga, pts; } Row;
static struct {
  int user, round, done, active;
  Row tb[8];
  int rounds[7][4][2];
} LG;

static void newLeague(int user) {
  memset(&LG, 0, sizeof(LG));
  LG.user = user; LG.round = 1; LG.active = 1;
  int arr[8];
  for (int i = 0; i < 8; i++) arr[i] = i;
  for (int r = 0; r < 7; r++) {
    for (int i = 0; i < 4; i++) { LG.rounds[r][i][0] = arr[i]; LG.rounds[r][i][1] = arr[7 - i]; }
    int na[8]; na[0] = arr[0]; na[1] = arr[7];
    for (int i = 1; i < 7; i++) na[i + 1] = arr[i];
    memcpy(arr, na, sizeof arr);
  }
  SCENE = SC_LEAGUE;
}

static void recordResult(int a, int b, int ga, int gb) {
  Row *A = &LG.tb[a], *B = &LG.tb[b];
  A->p++; B->p++;
  A->gf += ga; A->ga += gb; B->gf += gb; B->ga += ga;
  if (ga > gb) { A->w++; A->pts += 3; B->l++; }
  else if (gb > ga) { B->w++; B->pts += 3; A->l++; }
  else { A->d++; B->d++; A->pts++; B->pts++; }
}

static void sortedLeague(int ids[8]) {
  for (int i = 0; i < 8; i++) ids[i] = i;
  for (int i = 0; i < 7; i++)
    for (int j = i + 1; j < 8; j++) {
      Row *A = &LG.tb[ids[i]], *B = &LG.tb[ids[j]];
      int swap = 0;
      if (B->pts != A->pts) swap = B->pts > A->pts;
      else if ((B->gf - B->ga) != (A->gf - A->ga)) swap = (B->gf - B->ga) > (A->gf - A->ga);
      else if (B->gf != A->gf) swap = B->gf > A->gf;
      else swap = ids[j] < ids[i];
      if (swap) { int t = ids[i]; ids[i] = ids[j]; ids[j] = t; }
    }
}

static int leagueFixture(void) {
  for (int i = 0; i < 4; i++)
    if (LG.rounds[LG.round - 1][i][0] == LG.user || LG.rounds[LG.round - 1][i][1] == LG.user) return i;
  return 0;
}

static void leaguePlay(void) {
  int i = leagueFixture();
  int a = LG.rounds[LG.round - 1][i][0], b = LG.rounds[LG.round - 1][i][1];
  int opp = (a == LG.user) ? b : a;
  newMatch(LG.user, opp, 0, 0, 0, MODE_LEAGUE);
  SCENE = SC_MATCH;
}

static void leagueMatchEnd(int s1, int s2) {
  int fx = leagueFixture();
  for (int i = 0; i < 4; i++) {
    int a = LG.rounds[LG.round - 1][i][0], b = LG.rounds[LG.round - 1][i][1];
    if (i == fx) {
      if (a == LG.user) recordResult(a, b, s1, s2); else recordResult(a, b, s2, s1);
    } else {
      int x, y; simMatch(a, b, &x, &y); recordResult(a, b, x, y);
    }
  }
  LG.round++;
  if (LG.round > 7) { LG.done = 1; LG.round = 7; }
  SCENE = SC_LEAGUE;
}

static void sceneLeague(void) {
  char buf[128];
  drawBG(T_BG_FIELD); box(0, 0, SW, SH, COL(0, 0, 0, 170));
  ctxt(8, "CAMPEONATO", C_YELLOW, 240);
  int ids[8]; sortedLeague(ids);
  txt(30, 28, "#  TIME", C_GREY);
  txt(250, 28, " J  V  E  D  GP  PTS", C_GREY);
  for (int pos = 1; pos <= 8; pos++) {
    int id = ids[pos - 1]; Row *r = &LG.tb[id];
    int y = 32 + pos * 16;
    if (id == LG.user) box(24, y - 2, 432, 14, COL(255, 200, 40, 70));
    snprintf(buf, sizeof buf, "%d  %s", pos, TEAMS[id].name);
    txt(30, y, buf, (id == LG.user) ? C_YELLOW : C_WHITE);
    snprintf(buf, sizeof buf, "%2d %2d %2d %2d %3d %4d", r->p, r->w, r->d, r->l, r->gf - r->ga, r->pts);
    txt(250, y, buf, C_WHITE);
  }
  if (LG.done) {
    int pos = 1;
    for (int i = 0; i < 8; i++) if (ids[i] == LG.user) pos = i + 1;
    box(90, 205, 300, 50, C_PANEL);
    if (pos == 1) ctxt(212, "VOCE E O CAMPEAO!", C_YELLOW, 240);
    else { snprintf(buf, sizeof buf, "Terminou em %do lugar", pos); ctxt(212, buf, C_YELLOW, 240); }
    ctxt(234, "X para voltar ao menu", C_WHITE, 240);
    if (hit(B_CROSS)) { LG.active = 0; SCENE = SC_MENU; }
  } else {
    int i = leagueFixture();
    int a = LG.rounds[LG.round - 1][i][0], b = LG.rounds[LG.round - 1][i][1];
    int o = (a == LG.user) ? b : a;
    snprintf(buf, sizeof buf, "RODADA %d/7:  %s x %s", LG.round, TEAMS[LG.user].shortn, TEAMS[o].shortn);
    ctxt(214, buf, C_WHITE, 240);
    ctxt(234, "X jogar     TRIANGULO sair", C_GREY, 240);
    if (hit(B_CROSS)) leaguePlay();
    if (hit(B_TRIANGLE)) { LG.active = 0; SCENE = SC_MENU; }
  }
}

/* ============================================================ COPA */
static struct { int user, round, n, slots[8], alive, champion, active, opp; } CP;
static const char *CUPNAMES[3] = { "QUARTAS DE FINAL", "SEMIFINAL", "FINAL" };

static void newCup(int user) {
  memset(&CP, 0, sizeof(CP));
  int ids[8], n = 0;
  for (int i = 0; i < 8; i++) if (i != user) ids[n++] = i;
  for (int i = n - 1; i > 0; i--) { int j = rand() % (i + 1); int t = ids[i]; ids[i] = ids[j]; ids[j] = t; }
  CP.slots[0] = user;
  for (int i = 0; i < n; i++) CP.slots[i + 1] = ids[i];
  CP.n = 8; CP.user = user; CP.round = 1; CP.alive = 1; CP.active = 1;
  SCENE = SC_CUP;
}

static void cupAdvance(int userWon) {
  int nxt[8], m = 0;
  for (int i = 0; i < CP.n; i += 2) {
    int a = CP.slots[i], b = CP.slots[i + 1], w;
    if (a == CP.user || b == CP.user) w = userWon ? CP.user : ((a == CP.user) ? b : a);
    else {
      int x, y; simMatch(a, b, &x, &y);
      if (x == y) { if (prob() < 0.5f + (TEAMS[a].str - TEAMS[b].str) * 0.06f) x++; else y++; }
      w = (x > y) ? a : b;
    }
    nxt[m++] = w;
  }
  memcpy(CP.slots, nxt, sizeof(int) * m); CP.n = m;
  if (!userWon) CP.alive = 0;
  else if (m == 1) CP.champion = 1;
  else CP.round++;
  SCENE = SC_CUP;
}

static void cupPensEnd(int w) { cupAdvance(w == 1); }

static void cupPlay(void) {
  int opp = 0;
  for (int i = 0; i < CP.n; i += 2) {
    if (CP.slots[i] == CP.user) opp = CP.slots[i + 1];
    else if (CP.slots[i + 1] == CP.user) opp = CP.slots[i];
  }
  CP.opp = opp;
  newMatch(CP.user, opp, 0, 0, 0, MODE_CUP);
  SCENE = SC_MATCH;
}

static void cupMatchEnd(int s1, int s2) {
  if (s1 != s2) cupAdvance(s1 > s2);
  else newPens(CP.user, CP.opp, 1);
}

static void sceneCup(void) {
  char buf[128];
  drawBG(T_BG_FIELD); box(0, 0, SW, SH, COL(0, 0, 0, 170));
  ctxt(10, "COPA", C_YELLOW, 240);
  if (CP.champion) {
    ctxt(110, "VOCE E O CAMPEAO DA COPA!", C_YELLOW, 240);
    ctxt(140, TEAMS[CP.user].name, C_WHITE, 240);
    ctxt(220, "X para voltar ao menu", C_GREY, 240);
    if (hit(B_CROSS)) { CP.active = 0; SCENE = SC_MENU; }
    return;
  }
  if (!CP.alive) {
    ctxt(110, "VOCE FOI ELIMINADO", C_RED, 240);
    ctxt(220, "X para voltar ao menu", C_GREY, 240);
    if (hit(B_CROSS)) { CP.active = 0; SCENE = SC_MENU; }
    return;
  }
  ctxt(34, CUPNAMES[CP.round - 1 > 2 ? 2 : CP.round - 1], C_WHITE, 240);
  for (int i = 0; i < CP.n; i += 2) {
    int y = 64 + (i / 2) * 32;
    int a = CP.slots[i], b = CP.slots[i + 1];
    int mine = (a == CP.user || b == CP.user);
    snprintf(buf, sizeof buf, "%s  x  %s", TEAMS[a].name, TEAMS[b].name);
    ctxt(y, buf, mine ? C_YELLOW : C_WHITE, 240);
  }
  ctxt(220, "X jogar     TRIANGULO sair", C_GREY, 240);
  if (hit(B_CROSS)) cupPlay();
  if (hit(B_TRIANGLE)) { CP.active = 0; SCENE = SC_MENU; }
}

/* ============================================================ MENUS */
static int menuSel = 0;
static const char *MENU_ITEMS[8] = { "AMISTOSO", "CAMPEONATO", "COPA", "PENALTIS", "FUTSAL", "TREINO", "OPCOES", "SAIR" };
static struct { int idx, avoid, only, step, a, action; const char *title; } TS;
static int trIdx = 0, opSel = 0;
enum { ACT_FRIENDLY, ACT_FUTSAL, ACT_LEAGUE, ACT_CUP, ACT_PENS };

static void pickTeam(const char *title, int action, int step, int a, int avoid, int only) {
  TS.title = title; TS.action = action; TS.step = step; TS.a = a; TS.avoid = avoid; TS.only = only;
  TS.idx = (avoid >= 0) ? ((avoid == 0) ? 1 : 0) : 0;
  SCENE = SC_TEAMSEL;
}

static void teamChosen(int idx) {
  if (TS.step == 1 && (TS.action == ACT_FRIENDLY || TS.action == ACT_FUTSAL || TS.action == ACT_PENS)) {
    pickTeam("ADVERSARIO", TS.action, 2, idx, idx, 0);
    return;
  }
  if (TS.action == ACT_FRIENDLY || TS.action == ACT_FUTSAL) {
    newMatch(TS.a, idx, TS.action == ACT_FUTSAL, 0, 0, MODE_FRIENDLY); SCENE = SC_MATCH;
  } else if (TS.action == ACT_PENS) {
    newPens(TS.a, idx, 0);
  } else if (TS.action == ACT_LEAGUE) newLeague(idx);
  else if (TS.action == ACT_CUP) newCup(idx);
}

void onMatchEnd(int s1, int s2) {
  M.active = 0;
  if (RES.mode == MODE_LEAGUE) leagueMatchEnd(s1, s2);
  else if (RES.mode == MODE_CUP) cupMatchEnd(s1, s2);
  else SCENE = SC_MENU;
}

static void sceneTitle(void) {
  drawBG(T_BG_FIELD); box(0, 0, SW, SH, COL(0, 0, 0, 90));
  if (T_TITLE >= 0) plat_blit(T_TITLE, 0, 0, 0, 0, 10, 20, 255, 255, 255, 255);
  else ctxt(60, "SUPER PIXEL SOCCER PSP", C_YELLOW, 240);
  static const int kits[5] = { KIT_NEYMAR, KIT_MESSI, KIT_PELE, KIT_CR7, KIT_MARA };
  int t = TICK / 8;
  for (int i = 1; i <= 5; i++) drawFrame(KFR[kits[i - 1]][((t + i) % 2 == 0) ? 1 : 2], 100 + i * 60, 215);
  if ((TICK / 30) % 2 == 0) ctxt(148, "PRESSIONE START", C_WHITE, 240);
  ctxt(250, "Jogo de fas - nao oficial", C_GREY, 240);
  if (hit(B_START) || hit(B_CROSS)) SCENE = SC_MENU;
}

static void sceneMenu(void) {
  drawBG(T_BG_FIELD); box(0, 0, SW, SH, COL(0, 0, 0, 140));
  ctxt(14, "MENU PRINCIPAL", C_YELLOW, 240);
  for (int i = 0; i < 8; i++) {
    int y = 40 + i * 24;
    if (menuSel == i) box(150, y - 4, 180, 20, C_SEL);
    ctxt(y, MENU_ITEMS[i], (menuSel == i) ? C_BLACK : C_WHITE, 240);
  }
  if (hit(B_UP)) menuSel = (menuSel > 0) ? menuSel - 1 : 7;
  if (hit(B_DOWN)) menuSel = (menuSel < 7) ? menuSel + 1 : 0;
  if (hit(B_CROSS)) {
    switch (menuSel) {
      case 0: pickTeam("SEU TIME", ACT_FRIENDLY, 1, 0, -1, 0); break;
      case 1: pickTeam("SEU TIME", ACT_LEAGUE, 1, 0, -1, 1); break;
      case 2: pickTeam("SEU TIME", ACT_CUP, 1, 0, -1, 1); break;
      case 3: pickTeam("SEU TIME", ACT_PENS, 1, 0, -1, 0); break;
      case 4: pickTeam("SEU TIME", ACT_FUTSAL, 1, 0, -1, 0); break;
      case 5: trIdx = 0; SCENE = SC_TRAINSEL; break;
      case 6: opSel = 0; SCENE = SC_OPTS; break;
      case 7: SCENE = -1; break;
    }
  }
}

static void sceneTeamSel(void) {
  char buf[64];
  drawBG(T_BG_FIELD); box(0, 0, SW, SH, COL(0, 0, 0, 150));
  ctxt(8, TS.title, C_YELLOW, 240);
  int n = TS.only ? 8 : 9;
  for (int i = 0; i < n; i++) {
    int y = 30 + i * 22;
    int dis = (TS.avoid == i);
    if (TS.idx == i) box(20, y - 3, 190, 19, C_SEL);
    txt(28, y + 1, TEAMS[i].name, (TS.idx == i) ? C_BLACK : (dis ? C_GREY : C_WHITE));
  }
  const Team *tm = &TEAMS[TS.idx];
  box(230, 30, 230, 150, C_PANEL);
  ctxt(38, tm->name, C_YELLOW, 345);
  int nk = tm->nkits ? tm->nkits : 3;
  for (int i = 1; i <= nk; i++) {
    int k = tm->nkits ? tm->kits[i - 1] : tm->kit;
    drawFrame(KFR[k][((TICK / 10) % 2 == 0) ? 1 : 2], 230 + 230.0f / (nk + 1) * i, 110);
  }
  char stars[8];
  for (int i = 0; i < 5; i++) stars[i] = (i < tm->str) ? '*' : '-';
  stars[5] = 0;
  snprintf(buf, sizeof buf, "FORCA %s", stars);
  ctxt(130, buf, C_WHITE, 345);
  ctxt(150, tm->shortn, COL(tm->col.r, tm->col.g, tm->col.b, 255), 345);
  ctxt(240, "X escolher     TRIANGULO voltar", C_GREY, 240);
  if (hit(B_UP)) TS.idx = (TS.idx > 0) ? TS.idx - 1 : n - 1;
  if (hit(B_DOWN)) TS.idx = (TS.idx < n - 1) ? TS.idx + 1 : 0;
  if (hit(B_TRIANGLE)) SCENE = SC_MENU;
  if (hit(B_CROSS) && TS.avoid != TS.idx) teamChosen(TS.idx);
}

static const int STAR_KITS[5] = { KIT_NEYMAR, KIT_MESSI, KIT_PELE, KIT_CR7, KIT_MARA };
static const char *STAR_NAMES[5] = { "NEYMAR", "MESSI", "PELE", "CR7", "CRAQUE" };

static void sceneTrainSel(void) {
  drawBG(T_BG_FIELD); box(0, 0, SW, SH, COL(0, 0, 0, 150));
  ctxt(10, "TREINO - ESCOLHA O CRAQUE", C_YELLOW, 240);
  for (int i = 0; i < 5; i++) {
    int x = 70 + i * 85;
    if (trIdx == i) box(x - 28, 80, 60, 90, C_SEL);
    int fi = (trIdx == i && (TICK / 8) % 2 == 0) ? 1 : 0;
    drawFrame(KFR[STAR_KITS[i]][fi], x, 140);
    txt(x - (int)strlen(STAR_NAMES[i]) * 4, 152, STAR_NAMES[i], (trIdx == i) ? C_BLACK : C_WHITE);
  }
  ctxt(200, "Faca gols contra goleiro e 2 zagueiros", C_WHITE, 240);
  ctxt(240, "X jogar     TRIANGULO voltar", C_GREY, 240);
  if (hit(B_LEFT)) trIdx = (trIdx > 0) ? trIdx - 1 : 4;
  if (hit(B_RIGHT)) trIdx = (trIdx < 4) ? trIdx + 1 : 0;
  if (hit(B_TRIANGLE)) SCENE = SC_MENU;
  if (hit(B_CROSS)) { newMatch(8, 1, 0, 1, STAR_KITS[trIdx], MODE_TRAIN); SCENE = SC_MATCH; }
}

static void sceneOpts(void) {
  char buf[64];
  static const char *dn[3] = { "FACIL", "NORMAL", "DIFICIL" };
  static const char *hn[3] = { "CURTO (45s)", "MEDIO (90s)", "LONGO (150s)" };
  drawBG(T_BG_FIELD); box(0, 0, SW, SH, COL(0, 0, 0, 150));
  ctxt(14, "OPCOES", C_YELLOW, 240);
  const char *items[4]; char b0[48], b1[48], b2[48];
  snprintf(b0, sizeof b0, "DIFICULDADE: %s", dn[OPT.diff - 1]);
  snprintf(b1, sizeof b1, "TEMPO POR TEMPO: %s", hn[OPT.half - 1]);
  snprintf(b2, sizeof b2, "SOM: %s", OPT.sound ? "LIGADO" : "DESLIGADO");
  items[0] = b0; items[1] = b1; items[2] = b2; items[3] = "VOLTAR";
  for (int i = 0; i < 4; i++) {
    int y = 60 + i * 30;
    if (opSel == i) box(100, y - 5, 280, 22, C_SEL);
    ctxt(y, items[i], (opSel == i) ? C_BLACK : C_WHITE, 240);
  }
  (void)buf;
  ctxt(200, "Controles: X passe | O chute (segure) | [] lob", C_GREY, 240);
  ctxt(214, "R correr | O sem bola = carrinho | START pausa", C_GREY, 240);
  if (hit(B_UP)) opSel = (opSel > 0) ? opSel - 1 : 3;
  if (hit(B_DOWN)) opSel = (opSel < 3) ? opSel + 1 : 0;
  int d = 0;
  if (hit(B_LEFT)) d = -1;
  if (hit(B_RIGHT) || hit(B_CROSS)) d = 1;
  if (d) {
    if (opSel == 0) OPT.diff = clampi(OPT.diff + d, 1, 3);
    else if (opSel == 1) OPT.half = clampi(OPT.half + d, 1, 3);
    else if (opSel == 2) OPT.sound = !OPT.sound;
    else if (opSel == 3 && hit(B_CROSS)) { saveOpt(); SCENE = SC_MENU; }
  }
  if (hit(B_TRIANGLE)) { saveOpt(); SCENE = SC_MENU; }
}

static void sceneResult(void) {
  char buf[96];
  drawBG(T_BG_FIELD); box(0, 0, SW, SH, COL(0, 0, 0, 160));
  ctxt(30, "RESULTADO FINAL", C_YELLOW, 240);
  ctxt(80, TEAMS[RES.t1].name, C_WHITE, 240);
  snprintf(buf, sizeof buf, "%d  x  %d", RES.s1, RES.s2);
  ctxt(110, buf, C_YELLOW, 240);
  ctxt(140, TEAMS[RES.t2].name, C_WHITE, 240);
  snprintf(buf, sizeof buf, "Chutes: %d x %d", RES.shots1, RES.shots2);
  ctxt(180, buf, C_GREY, 240);
  ctxt(230, "X continuar", C_WHITE, 240);
  if (hit(B_CROSS)) onMatchEnd(RES.s1, RES.s2);
}

/* ============================================================ INICIO */
void game_init(void) {
  srand((unsigned)time(NULL));
  plat_init();
  plat_clear(0, 0, 0);
  T_FONT = plat_load_tex("gfx/font.png");
  ctxt(130, "Carregando...", C_WHITE, 240);
  plat_present();
  T_ATLAS = plat_load_tex("gfx/atlas.png");
  T_BG_FIELD = plat_load_tex("gfx/bg_field.png");
  T_BG_COURT = plat_load_tex("gfx/bg_court.png");
  T_TITLE = plat_load_tex("gfx/title.png");
  S_KICK = plat_snd_load("sfx/kick.wav");
  S_WHISTLE = plat_snd_load("sfx/whistle.wav");
  S_GOAL = plat_snd_load("sfx/goal.wav");
  S_POST = plat_snd_load("sfx/post.wav");
  loadOpt();
  SCENE = SC_TITLE; TICK = 0;
}

void game_frame(void) {
  TICK++;
  readPad();
  switch (SCENE) {
    case SC_TITLE: sceneTitle(); break;
    case SC_MENU: sceneMenu(); break;
    case SC_TEAMSEL: sceneTeamSel(); break;
    case SC_TRAINSEL: sceneTrainSel(); break;
    case SC_OPTS: sceneOpts(); break;
    case SC_MATCH: sceneMatch(); break;
    case SC_RESULT: sceneResult(); break;
    case SC_PENS: scenePens(); break;
    case SC_LEAGUE: sceneLeague(); break;
    case SC_CUP: sceneCup(); break;
    default: break;
  }
}

/* usado apenas pelos testes no PC */
int game_debug(int what) {
  switch (what) {
    case 0: return LG.done;
    case 1: return LG.round;
    case 2: return CP.champion;
    case 3: return CP.alive;
    case 4: return PEN.active;
    case 5: return LG.tb[0].p + LG.tb[1].p + LG.tb[2].p + LG.tb[3].p + LG.tb[4].p + LG.tb[5].p + LG.tb[6].p + LG.tb[7].p;
  }
  return 0;
}
