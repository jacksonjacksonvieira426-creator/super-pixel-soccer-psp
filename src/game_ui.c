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
  PEN.aimY = 200; PEN.aimZ = 1; PEN.gkY = PFW / 2; PEN.shotY = 200; PEN.shotZ = 6;
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
  if (shotY < PGY1 + 4 || shotY > PGY2 - 4) return "FORA!";
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

void startMatchPenalty(int att) {
  int t1 = M.team[1], t2 = M.team[2];
  newPens(t1, t2, 2);
  PEN.turn = att;
}

static void scenePens(void) {
  if (!PEN.active) { SCENE = SC_MENU; return; }
  drawBG(T_BG[clampi(OPT.stadium, 0, 3)]);
  int ti = PEN.turn, kicker = (ti == 1);
  float spotX = PFL - 110, sx0, sy0, gsx, gsy;
  toScreenSmall(spotX, PFW / 2, &sx0, &sy0);
  int kitK = TEAMS[PEN.team[ti]].nkits ? TEAMS[PEN.team[ti]].kits[0] : TEAMS[PEN.team[ti]].kit;

  if (PEN.ph == PH_AIM) {
    if (kicker) {
      if (held(B_UP)) PEN.aimY = fmaxf(125, PEN.aimY - 1.6f);
      if (held(B_DOWN)) PEN.aimY = fminf(275, PEN.aimY + 1.6f);
      if (hit(B_LEFT)) PEN.aimZ = 1;
      if (hit(B_RIGHT)) PEN.aimZ = 2;
      if (hit(B_CROSS)) { PEN.ph = PH_POWER; PEN.power = 0; PEN.pdir = 1; }
      PEN.gkY = PFW / 2;
    } else {
      if (held(B_UP)) PEN.aimY = fmaxf(150, PEN.aimY - 1.6f);
      if (held(B_DOWN)) PEN.aimY = fminf(250, PEN.aimY + 1.6f);
      if (hit(B_LEFT)) PEN.aimZ = 1;
      if (hit(B_RIGHT)) PEN.aimZ = 2;
      PEN.gkY = PFW / 2;
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
    PEN.gkY = PFW / 2 + (PEN.gkDive - PFW / 2) * fminf(1.0f, PEN.t / 14.0f);
    if (PEN.t >= 38) penFinishKick(penResolve(PEN.shotY, PEN.shotZ, PEN.power, PEN.gkDive, PEN.gkDiveZ));
  } else if (PEN.ph == PH_RESULT) {
    if (--PEN.st <= 0 || hit(B_CROSS)) {
      if (PEN.mode == 2) {
        int att = PEN.turn, gol = (PEN.goals[att] > 0);
        PEN.active = 0;
        matchPenaltyDone(att, gol);
        return;
      }
      if (pensWinner()) { PEN.ph = PH_END; PEN.st = 140; }
      else { PEN.turn = (PEN.turn == 1) ? 2 : 1; PEN.ph = PH_AIM; PEN.aimY = 200; PEN.aimZ = 1; PEN.gkY = PFW / 2; }
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
  toScreenSmall(PFL - 6, PEN.gkY, &gsx, &gsy);
  drawFrame(FR_SHADOW, gsx, gsy); drawFrame(FR_RING_A, gsx, gsy);
  int diving = (PEN.ph == PH_SHOOT || PEN.ph == PH_RESULT) && fabsf(PEN.gkDive - PFW / 2) > 20;
  drawKit(TEAMS[PEN.team[(ti == 1) ? 2 : 1]].gk, diving ? 3 : 0, gsx, gsy - ((diving && PEN.gkDiveZ > 15) ? 8 : 0));
  drawFrame(FR_SHADOW, sx0 - 14, sy0); drawFrame(FR_RING_H, sx0 - 14, sy0);
  drawKit(kitK, (PEN.ph == PH_SHOOT) ? 3 : 0, sx0 - 14, sy0);
  float bx = sx0, by = sy0, bz = 0;
  if (PEN.ph == PH_SHOOT || PEN.ph == PH_RESULT) {
    float k = (PEN.ph == PH_RESULT) ? 1.0f : fminf(1.0f, PEN.t / 38.0f), ex, ey;
    toScreenSmall(PFL + 4, PEN.shotY, &ex, &ey);
    bx = sx0 + (ex - sx0) * k; by = sy0 + (ey - sy0) * k;
    bz = PEN.shotZ * sinf(k * 3.14159f * 0.5f) * 0.6f;
  }
  drawFrame(FR_SHADOW, bx, by + 1); drawFrame(FR_BALL, bx, by - bz);
  if (PEN.ph == PH_AIM || PEN.ph == PH_POWER) {
    float mx, my;
    if (kicker) toScreenSmall(PFL + 2, PEN.aimY, &mx, &my); else toScreenSmall(PFL - 6, PEN.aimY, &mx, &my);
    float hz = (PEN.aimZ == 2) ? 14.0f : 3.0f;
    box((int)mx - 4, (int)(my - hz - 4), 9, 2, C_YELLOW);
    box((int)mx - 1, (int)(my - hz - 7), 2, 9, C_YELLOW);
  }
  char buf[96];
  if (PEN.mode == 2) ctxt(44, "PENALTI!", C_YELLOW, 240);
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
#define NL 12
typedef struct { int p, w, d, l, gf, ga, pts; } Row;
static struct {
  int user, round, done, active;
  int team[NL];
  Row tb[NL];
  int rounds[NL - 1][NL / 2][2];
} LG;

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

/* ---- carreira (salvar / continuar) ---- */
static struct { int user, round, n, slots[16], alive, champion, active, opp; } CP;
#define SAVE_MAGIC 0x53505333
static int saveKind = 0;
static int careerKind(void) {
  FILE *f = fopen("career.dat", "rb");
  if (!f) return 0;
  int hdr[2] = { 0, 0 };
  int ok = (fread(hdr, sizeof(int), 2, f) == 2 && hdr[0] == SAVE_MAGIC);
  fclose(f);
  return ok ? hdr[1] : 0;
}
static void refreshSave(void) { saveKind = careerKind(); }
static void saveCareer(void) {
  if (MGR.active) return;
  int kind = LG.active ? 1 : (CP.active ? 2 : 0);
  if (!kind) return;
  FILE *f = fopen("career.dat", "wb");
  if (!f) return;
  int hdr[2] = { SAVE_MAGIC, kind };
  fwrite(hdr, sizeof(int), 2, f);
  if (kind == 1) fwrite(&LG, sizeof(LG), 1, f); else fwrite(&CP, sizeof(CP), 1, f);
  fclose(f);
  refreshSave();
}
static void clearCareer(void) { remove("career.dat"); refreshSave(); }
static int loadCareer(void) {
  int kind = careerKind();
  if (!kind) return 0;
  FILE *f = fopen("career.dat", "rb");
  if (!f) return 0;
  int hdr[2];
  int ok = (fread(hdr, sizeof(int), 2, f) == 2);
  if (ok && kind == 1) { ok = (fread(&LG, sizeof(LG), 1, f) == 1); if (ok) { CP.active = 0; LG.active = 1; SCENE = SC_LEAGUE; } }
  else if (ok && kind == 2) { ok = (fread(&CP, sizeof(CP), 1, f) == 1); if (ok) { LG.active = 0; CP.active = 1; SCENE = SC_CUP; } }
  fclose(f);
  return ok;
}

static void newLeague(int user) {
  memset(&LG, 0, sizeof(LG));
  CP.active = 0;
  LG.user = 0; LG.round = 1; LG.active = 1;
  int ids[NCLUBS], n = 0;
  for (int i = 0; i < NCLUBS; i++) if (i != user) ids[n++] = i;
  for (int i = n - 1; i > 0; i--) { int j = rand() % (i + 1); int t = ids[i]; ids[i] = ids[j]; ids[j] = t; }
  LG.team[0] = user;
  for (int i = 1; i < NL; i++) LG.team[i] = ids[i - 1];
  int arr[NL];
  for (int i = 0; i < NL; i++) arr[i] = i;
  for (int r = 0; r < NL - 1; r++) {
    for (int i = 0; i < NL / 2; i++) { LG.rounds[r][i][0] = arr[i]; LG.rounds[r][i][1] = arr[NL - 1 - i]; }
    int na[NL]; na[0] = arr[0]; na[1] = arr[NL - 1];
    for (int i = 1; i < NL - 1; i++) na[i + 1] = arr[i];
    memcpy(arr, na, sizeof arr);
  }
  SCENE = SC_LEAGUE;
  saveCareer();
}

static void recordResult(int a, int b, int ga, int gb) {
  Row *A = &LG.tb[a], *B = &LG.tb[b];
  A->p++; B->p++;
  A->gf += ga; A->ga += gb; B->gf += gb; B->ga += ga;
  if (ga > gb) { A->w++; A->pts += 3; B->l++; }
  else if (gb > ga) { B->w++; B->pts += 3; A->l++; }
  else { A->d++; B->d++; A->pts++; B->pts++; }
}

static void sortedLeague(int ids[NL]) {
  for (int i = 0; i < NL; i++) ids[i] = i;
  for (int i = 0; i < NL - 1; i++)
    for (int j = i + 1; j < NL; j++) {
      Row *A = &LG.tb[ids[i]], *B = &LG.tb[ids[j]];
      int swap;
      if (B->pts != A->pts) swap = B->pts > A->pts;
      else if ((B->gf - B->ga) != (A->gf - A->ga)) swap = (B->gf - B->ga) > (A->gf - A->ga);
      else if (B->gf != A->gf) swap = B->gf > A->gf;
      else swap = ids[j] < ids[i];
      if (swap) { int t = ids[i]; ids[i] = ids[j]; ids[j] = t; }
    }
}

static int leagueFixture(void) {
  for (int i = 0; i < NL / 2; i++)
    if (LG.rounds[LG.round - 1][i][0] == 0 || LG.rounds[LG.round - 1][i][1] == 0) return i;
  return 0;
}

static void leaguePlay(void) {
  int i = leagueFixture();
  int a = LG.rounds[LG.round - 1][i][0], b = LG.rounds[LG.round - 1][i][1];
  int opp = (a == 0) ? b : a;
  newMatch(LG.team[0], LG.team[opp], 0, 0, 0, MODE_LEAGUE);
  SCENE = SC_MATCH;
}

static void leagueMatchEnd(int s1, int s2) {
  int fx = leagueFixture();
  for (int i = 0; i < NL / 2; i++) {
    int a = LG.rounds[LG.round - 1][i][0], b = LG.rounds[LG.round - 1][i][1];
    if (i == fx) {
      if (a == 0) recordResult(a, b, s1, s2); else recordResult(a, b, s2, s1);
    } else {
      int x, y; simMatch(LG.team[a], LG.team[b], &x, &y); recordResult(a, b, x, y);
    }
  }
  LG.round++;
  if (LG.round > NL - 1) { LG.done = 1; LG.round = NL - 1; }
  SCENE = SC_LEAGUE;
  saveCareer();
}

static void sceneLeague(void) {
  char buf[128];
  drawBG(T_BG[0]); box(0, 0, SW, SH, COL(0, 0, 0, 170));
  ctxt(6, "CAMPEONATO", C_YELLOW, 240);
  int ids[NL]; sortedLeague(ids);
  txt(30, 22, "#  TIME", C_GREY);
  txt(250, 22, " J  V  E  D  GP  PTS", C_GREY);
  for (int pos = 1; pos <= NL; pos++) {
    int id = ids[pos - 1]; Row *r = &LG.tb[id];
    int y = 22 + pos * 14;
    if (id == 0) box(24, y - 2, 432, 13, COL(255, 200, 40, 70));
    snprintf(buf, sizeof buf, "%2d %s", pos, TEAMS[LG.team[id]].name);
    txt(30, y, buf, (id == 0) ? C_YELLOW : C_WHITE);
    snprintf(buf, sizeof buf, "%2d %2d %2d %2d %3d %4d", r->p, r->w, r->d, r->l, r->gf - r->ga, r->pts);
    txt(250, y, buf, C_WHITE);
  }
  if (MGR.active && LG.done) { SCENE = SC_MGR; return; }
  if (LG.done) {
    int pos = 1;
    for (int i = 0; i < NL; i++) if (ids[i] == 0) pos = i + 1;
    box(90, 212, 300, 46, C_PANEL);
    if (pos == 1) ctxt(216, "VOCE E O CAMPEAO!", C_YELLOW, 240);
    else { snprintf(buf, sizeof buf, "Terminou em %do lugar", pos); ctxt(216, buf, C_YELLOW, 240); }
    ctxt(240, "X para voltar ao menu", C_WHITE, 240);
    if (hit(B_CROSS)) { LG.active = 0; clearCareer(); SCENE = SC_MENU; }
  } else {
    int i = leagueFixture();
    int a = LG.rounds[LG.round - 1][i][0], b = LG.rounds[LG.round - 1][i][1];
    int o = (a == 0) ? b : a;
    snprintf(buf, sizeof buf, "RODADA %d/%d:  %s x %s", LG.round, NL - 1, TEAMS[LG.team[0]].shortn, TEAMS[LG.team[o]].shortn);
    ctxt(214, buf, C_WHITE, 240);
    ctxt(238, "X jogar    TRIANGULO sair (progresso salvo)", C_GREY, 240);
    if (hit(B_CROSS)) leaguePlay();
    if (hit(B_TRIANGLE)) SCENE = MGR.active ? SC_MGR : SC_MENU;
  }
}

/* ============================================================ COPA */
static void newCup(int user) {
  memset(&CP, 0, sizeof(CP));
  LG.active = 0;
  int ids[NCLUBS], n = 0;
  for (int i = 0; i < NCLUBS; i++) if (i != user) ids[n++] = i;
  for (int i = n - 1; i > 0; i--) { int j = rand() % (i + 1); int t = ids[i]; ids[i] = ids[j]; ids[j] = t; }
  CP.slots[0] = user;
  for (int i = 0; i < 15; i++) CP.slots[i + 1] = ids[i];
  CP.n = 16; CP.user = user; CP.round = 1; CP.alive = 1; CP.active = 1;
  SCENE = SC_CUP;
  saveCareer();
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
  saveCareer();
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
  drawBG(T_BG[0]); box(0, 0, SW, SH, COL(0, 0, 0, 170));
  ctxt(8, "COPA", C_YELLOW, 240);
  if (CP.champion) {
    ctxt(110, "VOCE E O CAMPEAO DA COPA!", C_YELLOW, 240);
    ctxt(140, TEAMS[CP.user].name, C_WHITE, 240);
    ctxt(220, "X para voltar ao menu", C_GREY, 240);
    if (hit(B_CROSS)) { CP.active = 0; clearCareer(); SCENE = SC_MENU; }
    return;
  }
  if (!CP.alive) {
    ctxt(110, "VOCE FOI ELIMINADO", C_RED, 240);
    ctxt(220, "X para voltar ao menu", C_GREY, 240);
    if (hit(B_CROSS)) { CP.active = 0; clearCareer(); SCENE = SC_MENU; }
    return;
  }
  const char *rn = (CP.n == 16) ? "OITAVAS DE FINAL" : (CP.n == 8 ? "QUARTAS DE FINAL" : (CP.n == 4 ? "SEMIFINAL" : "FINAL"));
  ctxt(26, rn, C_WHITE, 240);
  for (int i = 0; i < CP.n; i += 2) {
    int y = 46 + (i / 2) * 23;
    int a = CP.slots[i], b = CP.slots[i + 1];
    int mine = (a == CP.user || b == CP.user);
    snprintf(buf, sizeof buf, "%s  x  %s", TEAMS[a].name, TEAMS[b].name);
    ctxt(y, buf, mine ? C_YELLOW : C_WHITE, 240);
  }
  ctxt(240, "X jogar    TRIANGULO sair (progresso salvo)", C_GREY, 240);
  if (hit(B_CROSS)) cupPlay();
  if (hit(B_TRIANGLE)) SCENE = SC_MENU;
}

/* ============================================================ MENUS */
enum { A_CONT, A_MCONT, A_FRIENDLY, A_LEAGUE, A_CUP, A_MGR, A_PENS, A_FUTSAL, A_TRAIN, A_TACT, A_OPTS, A_QUIT };
static int menuSel = 0;
static struct { int idx, avoid, only, step, a, action; const char *title; } TS;
static int trIdx = 0, opSel = 0, tacSel = 0;
enum { ACT_FRIENDLY, ACT_FUTSAL, ACT_LEAGUE, ACT_CUP, ACT_PENS, ACT_MGR };

static void pickTeam(const char *title, int action, int step, int a, int avoid, int only) {
  TS.title = title; TS.action = action; TS.step = step; TS.a = a; TS.avoid = avoid; TS.only = only;
  TS.idx = (avoid >= 0) ? ((avoid == 0) ? 1 : 0) : 0;
  if (step == 1 && UserForm >= formCount(formKind(action == ACT_FUTSAL))) UserForm = 0;
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
  else if (TS.action == ACT_MGR) mgrNew(idx, 0);
  else if (TS.action == ACT_CUP) newCup(idx);
}

void onMatchEnd(int s1, int s2) {
  M.active = 0;
  if (RES.mode == MODE_LEAGUE && MGR.active) { leagueMatchEnd(s1, s2); mgrAfterMatch(s1, s2); SCENE = SC_MGR; }
  else if (RES.mode == MODE_LEAGUE) leagueMatchEnd(s1, s2);
  else if (RES.mode == MODE_CUP) cupMatchEnd(s1, s2);
  else SCENE = SC_MENU;
}

static void sceneTitle(void) {
  drawBG(T_BG[0]); box(0, 0, SW, SH, COL(0, 0, 0, 90));
  if (T_TITLE >= 0) plat_blit(T_TITLE, 0, 0, 0, 0, 10, 20, 255, 255, 255, 255);
  else ctxt(60, "SUPER PIXEL SOCCER PSP", C_YELLOW, 240);
  static const int kits[5] = { KIT_NEYMAR, KIT_MESSI, KIT_PELE, KIT_CR7, KIT_MARA };
  int t = TICK / 8;
  for (int i = 1; i <= 5; i++) drawKit(kits[i - 1], ((t + i) % 2 == 0) ? 1 : 2, 100 + i * 60, 215);
  if ((TICK / 30) % 2 == 0) ctxt(148, "PRESSIONE START", C_WHITE, 240);
  ctxt(250, "Edicao Definitiva - jogo de fas", C_GREY, 240);
  if (hit(B_START) || hit(B_CROSS)) SCENE = SC_MENU;
}

static void sceneMenu(void) {
  static const char *labels[12] = { "CONTINUAR CARREIRA", "CONTINUAR MANAGER", "AMISTOSO", "CAMPEONATO", "COPA", "MODO MANAGER", "PENALTIS", "FUTSAL", "TREINO", "TATICA", "OPCOES", "SAIR" };
  int acts[12], n = 0;
  if (saveKind) acts[n++] = A_CONT;
  if (mgrSaved()) acts[n++] = A_MCONT;
  for (int a = A_FRIENDLY; a <= A_QUIT; a++) acts[n++] = a;
  if (menuSel >= n) menuSel = 0;
  drawBG(T_MENU >= 0 ? T_MENU : T_BG[0]); box(0, 0, SW, SH, COL(0, 0, 0, 150));
  ctxt(6, "MENU PRINCIPAL", C_YELLOW, 240);
  int step = (n > 10) ? 19 : 22;
  for (int i = 0; i < n; i++) {
    int y = 24 + i * step;
    if (menuSel == i) box(130, y - 3, 220, step - 2, C_SEL);
    ctxt(y, labels[acts[i]], (menuSel == i) ? C_BLACK : C_WHITE, 240);
  }
  if (hit(B_UP)) menuSel = (menuSel > 0) ? menuSel - 1 : n - 1;
  if (hit(B_DOWN)) menuSel = (menuSel < n - 1) ? menuSel + 1 : 0;
  if (hit(B_CROSS)) {
    switch (acts[menuSel]) {
      case A_CONT: if (!loadCareer()) refreshSave(); break;
      case A_MCONT: if (mgrLoad()) SCENE = SC_MGR; break;
      case A_FRIENDLY: pickTeam("SEU TIME", ACT_FRIENDLY, 1, 0, -1, 0); break;
      case A_LEAGUE: pickTeam("SEU TIME", ACT_LEAGUE, 1, 0, -1, 1); break;
      case A_CUP: pickTeam("SEU TIME", ACT_CUP, 1, 0, -1, 1); break;
      case A_MGR: pickTeam("ESCOLHA SEU CLUBE", ACT_MGR, 1, 0, -1, 1); break;
      case A_PENS: pickTeam("SEU TIME", ACT_PENS, 1, 0, -1, 0); break;
      case A_FUTSAL: pickTeam("SEU TIME", ACT_FUTSAL, 1, 0, -1, 0); break;
      case A_TRAIN: trIdx = 0; SCENE = SC_TRAINSEL; break;
      case A_TACT: tacSel = 0; SCENE = SC_TACTICS; break;
      case A_OPTS: opSel = 0; SCENE = SC_OPTS; break;
      case A_QUIT: SCENE = -1; break;
    }
  }
}

static void sceneTeamSel(void) {
  char buf[64];
  drawBG(T_MENU >= 0 ? T_MENU : T_BG[0]); box(0, 0, SW, SH, COL(0, 0, 0, 160));
  ctxt(8, TS.title, C_YELLOW, 240);
  int n = TS.only ? NCLUBS : NTEAMS;
  int top = clampi(TS.idx - 4, 0, n > 9 ? n - 9 : 0);
  for (int i = top; i < n && i < top + 9; i++) {
    int y = 30 + (i - top) * 22;
    int dis = (TS.avoid == i);
    if (TS.idx == i) box(20, y - 3, 190, 19, C_SEL);
    txt(28, y + 1, TEAMS[i].name, (TS.idx == i) ? C_BLACK : (dis ? C_GREY : C_WHITE));
  }
  if (top > 0) txt(196, 22, "^", C_YELLOW);
  if (top + 9 < n) txt(196, 230, "v", C_YELLOW);
  const Team *tm = &TEAMS[TS.idx];
  box(230, 30, 230, 176, C_PANEL);
  ctxt(38, tm->name, C_YELLOW, 345);
  int nk = tm->nkits ? tm->nkits : 3;
  for (int i = 1; i <= nk; i++) {
    int k = tm->nkits ? tm->kits[i - 1] : tm->kit;
    drawKit(k, ((TICK / 10) % 2 == 0) ? 1 : 2, 230 + 230.0f / (nk + 1) * i, 105);
  }
  drawKit(tm->gk, 0, 345, 150);
  char stars[8];
  for (int i = 0; i < 5; i++) stars[i] = (i < tm->str) ? '*' : '-';
  stars[5] = 0;
  snprintf(buf, sizeof buf, "FORCA %s", stars);
  ctxt(124, buf, C_WHITE, 345);
  ctxt(158, tm->shortn, COL(tm->col.r, tm->col.g, tm->col.b, 255), 345);
  if (TS.step == 1 && TS.action != ACT_PENS && TS.action != ACT_MGR) {
    int kind = formKind(TS.action == ACT_FUTSAL);
    snprintf(buf, sizeof buf, "< FORMACAO %s >", formName(kind, UserForm));
    ctxt(180, buf, C_YELLOW, 345);
    if (hit(B_LEFT)) UserForm = (UserForm > 0) ? UserForm - 1 : formCount(kind) - 1;
    if (hit(B_RIGHT)) UserForm = (UserForm < formCount(kind) - 1) ? UserForm + 1 : 0;
  }
  ctxt(248, "X escolher     TRIANGULO voltar", C_GREY, 240);
  if (hit(B_UP)) TS.idx = (TS.idx > 0) ? TS.idx - 1 : n - 1;
  if (hit(B_DOWN)) TS.idx = (TS.idx < n - 1) ? TS.idx + 1 : 0;
  if (hit(B_TRIANGLE)) SCENE = SC_MENU;
  if (hit(B_CROSS) && TS.avoid != TS.idx) teamChosen(TS.idx);
}

static void sceneTactics(void) {
  drawBG(T_MENU >= 0 ? T_MENU : T_BG[0]); box(0, 0, SW, SH, COL(0, 0, 0, 160));
  ctxt(18, "TATICA E FORMACAO", C_YELLOW, 240);
  ctxt(34, OPT.big ? "(campo grande: 11 x 11)" : "(campo pequeno: 6 x 6)", C_GREY, 240);
  if (UserForm >= formCount(formKind(0))) UserForm = 0;
  if (tacEdit(&UserTac, &UserForm, formKind(0), 58, &tacSel)) { saveOpt(); SCENE = SC_MENU; }
  ctxt(228, "Vale para amistoso, campeonato e copa", C_GREY, 240);
  ctxt(244, "Durante o jogo: START > TATICA", C_GREY, 240);
}

static const int STAR_KITS[5] = { KIT_NEYMAR, KIT_MESSI, KIT_PELE, KIT_CR7, KIT_MARA };
static const char *STAR_NAMES[5] = { "NEYMAR", "MESSI", "PELE", "CR7", "CRAQUE" };

static void sceneTrainSel(void) {
  drawBG(T_BG[0]); box(0, 0, SW, SH, COL(0, 0, 0, 150));
  ctxt(10, "TREINO - ESCOLHA O CRAQUE", C_YELLOW, 240);
  for (int i = 0; i < 5; i++) {
    int x = 70 + i * 85;
    if (trIdx == i) box(x - 28, 80, 60, 90, C_SEL);
    int fi = (trIdx == i && (TICK / 8) % 2 == 0) ? 1 : 0;
    drawKit(STAR_KITS[i], fi, x, 140);
    txt(x - (int)strlen(STAR_NAMES[i]) * 4, 152, STAR_NAMES[i], (trIdx == i) ? C_BLACK : C_WHITE);
  }
  ctxt(200, "Faca gols contra goleiro e 2 zagueiros", C_WHITE, 240);
  ctxt(240, "X jogar     TRIANGULO voltar", C_GREY, 240);
  if (hit(B_LEFT)) trIdx = (trIdx > 0) ? trIdx - 1 : 4;
  if (hit(B_RIGHT)) trIdx = (trIdx < 4) ? trIdx + 1 : 0;
  if (hit(B_TRIANGLE)) SCENE = SC_MENU;
  if (hit(B_CROSS)) { newMatch(T_LENDAS, 1, 0, 1, STAR_KITS[trIdx], MODE_TRAIN); SCENE = SC_MATCH; }
}

static void sceneOpts(void) {
  static const char *dn[3] = { "FACIL", "NORMAL", "DIFICIL" };
  static const char *hn[3] = { "CURTO (45s)", "MEDIO (90s)", "LONGO (150s)" };
  static const char *sn[4] = { "DIA", "NOITE", "NEVE", "AREIA" };
  static const char *bn[4] = { "CLASSICA", "BRAZUCA", "INCYTE", "MAXIM" };
  drawBG(T_MENU >= 0 ? T_MENU : T_BG[0]); box(0, 0, SW, SH, COL(0, 0, 0, 160));
  ctxt(8, "OPCOES", C_YELLOW, 240);
  char b[8][48];
  snprintf(b[0], 48, "DIFICULDADE: %s", dn[clampi(OPT.diff, 1, 3) - 1]);
  snprintf(b[1], 48, "TEMPO POR TEMPO: %s", hn[clampi(OPT.half, 1, 3) - 1]);
  snprintf(b[2], 48, "ESTADIO: %s", sn[clampi(OPT.stadium, 0, 3)]);
  snprintf(b[3], 48, "CAMPO: %s", OPT.big ? "GRANDE 11x11" : "PEQUENO 6x6");
  snprintf(b[4], 48, "BOLA: %s", bn[clampi(OPT.ball, 0, 3)]);
  snprintf(b[5], 48, "SOM: %s", OPT.sound ? "LIGADO" : "DESLIGADO");
  snprintf(b[6], 48, "MUSICA: %s", OPT.music ? "LIGADA" : "DESLIGADA");
  snprintf(b[7], 48, "VOLTAR");
  for (int i = 0; i < 8; i++) {
    int y = 24 + i * 23;
    if (opSel == i) box(90, y - 4, 300, 20, C_SEL);
    ctxt(y, b[i], (opSel == i) ? C_BLACK : C_WHITE, 240);
  }
  drawFrame(FR_BALLS[clampi(OPT.ball, 0, 3)], 420, 100);
  ctxt(224, "X passe | O chute (segure) | [] lob | R correr", C_GREY, 240);
  ctxt(240, "O sem bola = carrinho | START pausa", C_GREY, 240);
  if (hit(B_UP)) opSel = (opSel > 0) ? opSel - 1 : 7;
  if (hit(B_DOWN)) opSel = (opSel < 7) ? opSel + 1 : 0;
  int d = 0;
  if (hit(B_LEFT)) d = -1;
  if (hit(B_RIGHT) || hit(B_CROSS)) d = 1;
  if (d) {
    if (opSel == 0) OPT.diff = clampi(OPT.diff + d, 1, 3);
    else if (opSel == 1) OPT.half = clampi(OPT.half + d, 1, 3);
    else if (opSel == 2) OPT.stadium = (OPT.stadium + d + 4) % 4;
    else if (opSel == 3) { OPT.big = !OPT.big; UserForm = 0; }
    else if (opSel == 4) OPT.ball = (OPT.ball + d + 4) % 4;
    else if (opSel == 5) OPT.sound = !OPT.sound;
    else if (opSel == 6) OPT.music = !OPT.music;
    else if (opSel == 7 && hit(B_CROSS)) { saveOpt(); SCENE = SC_MENU; }
  }
  if (hit(B_TRIANGLE)) { saveOpt(); SCENE = SC_MENU; }
}

static void sceneResult(void) {
  char buf[96];
  drawBG(T_BG[0]); box(0, 0, SW, SH, COL(0, 0, 0, 160));
  ctxt(26, "RESULTADO FINAL", C_YELLOW, 240);
  ctxt(70, TEAMS[RES.t1].name, C_WHITE, 240);
  snprintf(buf, sizeof buf, "%d  x  %d", RES.s1, RES.s2);
  ctxt(100, buf, C_YELLOW, 240);
  ctxt(130, TEAMS[RES.t2].name, C_WHITE, 240);
  snprintf(buf, sizeof buf, "Chutes: %d x %d", RES.shots1, RES.shots2);
  ctxt(170, buf, C_GREY, 240);
  snprintf(buf, sizeof buf, "Posse de bola: %d%% x %d%%", RES.poss1, RES.poss2);
  ctxt(188, buf, C_GREY, 240);
  ctxt(212, "X continuar", C_WHITE, 240);
  { int ci = (RES.s1 > RES.s2) ? 0 : ((RES.s1 < RES.s2) ? 2 : 1); static const int ord[3] = { 0, 1, 2 }; if (T_CROWD[ord[ci]] >= 0) plat_blit(T_CROWD[ord[ci]], 0, 0, 0, 0, 0, 208, 255, 255, 255, 255); }
  if (hit(B_CROSS)) onMatchEnd(RES.s1, RES.s2);
}

/* ============================================================ INICIO */
static int musicOn = 0;

void game_init(void) {
  srand((unsigned)time(NULL));
  plat_init();
  plat_clear(0, 0, 0);
  T_FONT = plat_load_tex("gfx/font.png");
  ctxt(130, "Carregando...", C_WHITE, 240);
  plat_present();
  T_ATLAS = plat_load_tex("gfx/atlas.png");
  T_BG[0] = plat_load_tex("gfx/bg_field.png");
  T_BG[1] = plat_load_tex("gfx/bg_night.png");
  T_BG[2] = plat_load_tex("gfx/bg_snow.png");
  T_BG[3] = plat_load_tex("gfx/bg_sand.png");
  for (int i = 1; i < 4; i++) if (T_BG[i] < 0) T_BG[i] = T_BG[0];
  T_BG_COURT = plat_load_tex("gfx/bg_court.png");
  { char p[48]; snprintf(p, sizeof p, "gfx/menu_%d.png", rand() % 9); T_MENU = plat_load_tex(p); }
  T_COACH = plat_load_tex("gfx/coaches.png");
  T_CROWD[0] = plat_load_tex("gfx/crowd_gol.png"); T_CROWD[1] = plat_load_tex("gfx/crowd_quiet.png");
  T_CROWD[2] = plat_load_tex("gfx/crowd_sad.png"); T_CROWD[3] = plat_load_tex("gfx/crowd_fav.png");
  T_TITLE = plat_load_tex("gfx/title.png");
  S_KICK = plat_snd_load("sfx/kick.wav");
  S_WHISTLE = plat_snd_load("sfx/whistle.wav");
  S_GOAL = plat_snd_load("sfx/goal.wav");
  S_POST = plat_snd_load("sfx/post.wav");
  S_CHEER = plat_snd_load("sfx/cheer.wav");
  S_MUSIC = plat_snd_load("sfx/music.wav");
  loadOpt();
  refreshSave();
  SCENE = SC_TITLE; TICK = 0;
}

void game_frame(void) {
  TICK++;
  readPad();
  int wantMusic = OPT.music && S_MUSIC >= 0 && SCENE != SC_MATCH && SCENE != SC_PENS && SCENE != -1;
  if (wantMusic && !musicOn) { plat_snd_loop(S_MUSIC); musicOn = 1; }
  if (!wantMusic && musicOn) { plat_snd_stop(S_MUSIC); musicOn = 0; }
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
    case SC_TACTICS: sceneTactics(); break;
    case SC_MGR: sceneMgr(); break;
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
    case 5: { int s = 0; for (int i = 0; i < NL; i++) s += LG.tb[i].p; return s; }
    case 6: return saveKind;
  }
  return 0;
}

void game_debug_set(int what, int v) {
  if (what == 0) menuSel = v;
  if (what == 1) { LG.round = v; LG.active = 0; }
}

/* ---- ganchos usados pelo modo manager ---- */
void leagueStart(int team) { newLeague(team); }
void leaguePlayMatch(void) { leaguePlay(); }
int leagueIsDone(void) { return LG.done; }
int leagueRoundNo(void) { return LG.round; }
int leagueUserPos(void) {
  int ids[NL]; sortedLeague(ids);
  for (int i = 0; i < NL; i++) if (ids[i] == 0) return i + 1;
  return NL;
}
int leagueGoalDiff(void) { return LG.tb[0].gf - LG.tb[0].ga; }
int leagueLosses(void) { return LG.tb[0].l; }
int leagueNextOpp(void) {
  if (LG.done) return 0;
  int i = leagueFixture();
  int a = LG.rounds[LG.round - 1][i][0], b = LG.rounds[LG.round - 1][i][1];
  return LG.team[(a == 0) ? b : a];
}
void leagueSimUser(void) {
  int opp = leagueNextOpp(), g1, g2;
  simMatch(MGR.team, opp, &g1, &g2);
  RES.mode = MODE_LEAGUE; RES.s1 = g1; RES.s2 = g2; RES.t1 = MGR.team; RES.t2 = opp;
  leagueMatchEnd(g1, g2);
  mgrAfterMatch(g1, g2);
  SCENE = SC_MGR;
}
void leagueWrite(FILE *f) { fwrite(&LG, sizeof(LG), 1, f); }
int leagueRead(FILE *f) { int ok = (fread(&LG, sizeof(LG), 1, f) == 1); if (ok) LG.active = 1; return ok; }
int leagueSortedId(int pos) { int ids[NL]; sortedLeague(ids); return LG.team[ids[clampi(pos, 0, NL - 1)]]; }
