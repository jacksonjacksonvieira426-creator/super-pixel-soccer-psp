/* Super Pixel Soccer PSP - desenho da partida, pausa, taticas e substituicoes */
#include "game.h"

/* ---------------- campo grande (desenhado por codigo) ---------------- */
static void wrect(float x, float y, float w, float h, Col c) {
  float sx, sy; toScreen(x, y, &sx, &sy);
  int x0 = (int)sx, y0 = (int)sy, x1 = (int)(sx + w), y1 = (int)(sy + h);
  if (x0 < 0) x0 = 0;
  if (y0 < 0) y0 = 0;
  if (x1 > SW) x1 = SW;
  if (y1 > SH) y1 = SH;
  if (x1 <= x0 || y1 <= y0) return;
  box(x0, y0, x1 - x0, y1 - y0, c);
}
static void wdot(float x, float y, Col c) { wrect(x - 1, y - 1, 3, 3, c); }
static void wcircle(float cx, float cy, float r, Col c, float a0, float a1) {
  for (int i = 0; i <= 180; i++) {
    float a = a0 + (a1 - a0) * i / 180.0f;
    wdot(cx + r * cosf(a), cy + r * sinf(a), c);
  }
}

typedef struct { Col a, b, out, board, line; int pat, tr, tg, tb; } Theme;
static const Theme TH[14] = {
  { {38,128,48,255}, {45,140,56,255}, {26,96,38,255}, {20,30,70,255}, {240,240,240,255}, 0, 255,255,255 },
  { {18,70,38,255}, {22,80,44,255}, {12,45,28,255}, {10,12,35,255}, {210,215,230,255}, 0, 90,100,150 },
  { {200,215,225,255}, {220,232,240,255}, {170,190,205,255}, {40,60,100,255}, {110,140,200,255}, 2, 220,230,255 },
  { {214,184,120,255}, {224,196,132,255}, {190,160,100,255}, {120,60,20,255}, {255,255,255,255}, 3, 255,230,190 },
  { {176,92,48,255}, {188,102,56,255}, {150,76,40,255}, {30,50,90,255}, {245,245,245,255}, 0, 255,200,170 },
  { {20,95,60,255}, {26,108,70,255}, {14,70,45,255}, {15,25,55,255}, {235,245,240,255}, 1, 190,255,220 },
  { {70,110,50,255}, {78,120,58,255}, {50,80,40,255}, {60,30,70,255}, {255,235,210,255}, 3, 255,170,130 },
  { {44,100,70,255}, {50,110,78,255}, {34,78,56,255}, {20,34,60,255}, {225,235,245,255}, 0, 150,170,200 },
  { {52,150,52,255}, {60,162,58,255}, {36,110,40,255}, {20,30,80,255}, {255,255,255,255}, 0, 170,200,255 },
  { {30,130,50,255}, {38,142,58,255}, {20,95,38,255}, {10,60,30,255}, {255,255,255,255}, 0, 150,255,170 },
  { {44,140,48,255}, {52,152,56,255}, {30,100,36,255}, {70,10,10,255}, {255,255,255,255}, 0, 255,140,140 },
  { {50,140,50,255}, {58,152,58,255}, {34,102,38,255}, {40,40,60,255}, {255,255,255,255}, 0, 255,255,255 },
  { {60,170,50,255}, {40,150,40,255}, {30,110,35,255}, {120,20,20,255}, {255,255,255,255}, 2, 255,120,120 },
  { {50,150,55,255}, {58,162,62,255}, {30,105,40,255}, {20,20,90,255}, {255,255,255,255}, 0, 160,170,255 }
};

static void drawStands(const Theme *th) {
  if (T_STAND < 0) return;
  for (int row = 0; row < 2; row++) {
    float ty = -76.0f - row * 40.0f, by = FW + 28.0f + row * 40.0f;
    float x0 = floorf((M.cx - 240) / 64.0f) * 64.0f;
    for (float x = x0; x < M.cx + 304; x += 64) {
      float sx, sy;
      toScreen(x, ty, &sx, &sy);
      if (sy > -40 && sy < SH) plat_blit(T_STAND, 0, 0, 64, 40, (int)sx, (int)sy, th->tr, th->tg, th->tb, 255);
      toScreen(x, by, &sx, &sy);
      if (sy > -40 && sy < SH) plat_blit(T_STAND, 0, 0, 64, 40, (int)sx, (int)sy, th->tr, th->tg, th->tb, 255);
    }
  }
}

static void drawBigPitch(void) {
  const Theme *th = &TH[clampi(OPT.stadium, 0, 13)];
  const Col line = th->line;
  box(0, 0, SW, SH, th->out);
  drawStands(th);
  if (th->pat == 1) { for (int i = 0; i < 17; i++) for (int j = 0; j < 9; j++) wrect(i * 100.0f, j * 100.0f, 100, 100, ((i + j) % 2) ? th->a : th->b); }
  else if (th->pat >= 2) { for (int j = 0; j < 10; j++) wrect(0, j * 95.0f, FL, 95, (j % 2) ? th->a : th->b); }
  else { for (int i = 0; i < 17; i++) wrect(i * 100.0f, 0, 100, FW, (i % 2) ? th->a : th->b); }
  /* faixa de publicidade */
  wrect(-40, -26, FL + 80, 14, th->board);
  wrect(-40, FW + 12, FL + 80, 14, th->board);
  for (int i = 0; i < 20; i++) { wrect(-30 + i * 90.0f, -23, 40, 8, COL(230, 200, 40, 255)); wrect(-30 + i * 90.0f, FW + 15, 40, 8, COL(230, 200, 40, 255)); }
  /* linhas */
  wrect(0, 0, FL, 3, line); wrect(0, FW - 3, FL, 3, line);
  wrect(0, 0, 3, FW, line); wrect(FL - 3, 0, 3, FW, line);
  wrect(FL / 2 - 1, 0, 3, FW, line);
  wcircle(FL / 2, FW / 2, 120, line, 0, 6.2832f);
  wdot(FL / 2, FW / 2, line); wrect(FL / 2 - 3, FW / 2 - 3, 7, 7, line);
  float pa = 250, pw = FW * 0.3f, ga = 90, gw = (GOAL_Y2 - GOAL_Y1) * 0.65f + 40;
  for (int s = 0; s < 2; s++) {
    float x0 = (s == 0) ? 0 : FL - pa, gx0 = (s == 0) ? 0 : FL - ga;
    wrect(x0, FW / 2 - pw, pa, 3, line); wrect(x0, FW / 2 + pw - 3, pa, 3, line);
    wrect((s == 0) ? pa - 3 : FL - pa, FW / 2 - pw, 3, pw * 2, line);
    wrect(gx0, FW / 2 - gw, ga, 3, line); wrect(gx0, FW / 2 + gw - 3, ga, 3, line);
    wrect((s == 0) ? ga - 3 : FL - ga, FW / 2 - gw, 3, gw * 2, line);
    float spx = (s == 0) ? 180 : FL - 180;
    wrect(spx - 3, FW / 2 - 3, 7, 7, line);
    if (s == 0) wcircle(spx, FW / 2, 120, line, -0.9f, 0.9f); else wcircle(spx, FW / 2, 120, line, 3.14159f - 0.9f, 3.14159f + 0.9f);
    /* gol: rede e traves */
    float nx = (s == 0) ? -46.0f : FL;
    wrect(nx, GOAL_Y1, 46, GOAL_Y2 - GOAL_Y1, COL(220, 220, 220, 110));
    for (int k = 0; k < 4; k++) wrect(nx + k * 12.0f, GOAL_Y1, 1, GOAL_Y2 - GOAL_Y1, COL(255, 255, 255, 160));
    for (int k = 0; k <= 8; k++) wrect(nx, GOAL_Y1 + k * 25.0f, 46, 1, COL(255, 255, 255, 160));
    wrect((s == 0) ? -4.0f : FL, GOAL_Y1 - 3, 4, 6, line); wrect((s == 0) ? -4.0f : FL, GOAL_Y2 - 3, 4, 6, line);
  }
  /* escanteios */
  wcircle(0, 0, 18, line, 0, 1.5708f); wcircle(FL, 0, 18, line, 1.5708f, 3.14159f);
  wcircle(0, FW, 18, line, -1.5708f, 0); wcircle(FL, FW, 18, line, 3.14159f, 4.7124f);
}

static void drawMinimap(void) {
  int mx = 382, my = 26, mw = 94, mh = 50;
  box(mx - 2, my - 2, mw + 4, mh + 4, COL(0, 0, 0, 150));
  box(mx, my, mw, mh, COL(30, 110, 45, 200));
  box(mx + mw / 2, my, 1, mh, COL(255, 255, 255, 120));
  for (int k = 0; k < M.np; k++) {
    const Player *p = &M.pl[k];
    if (p->out) continue;
    int x = mx + (int)(p->x / FL * mw), y = my + (int)(p->y / FW * mh);
    box(x - 1, y - 1, 3, 3, (p->t == 1) ? COL(90, 170, 255, 255) : COL(255, 90, 90, 255));
  }
  box(mx + (int)(M.ball.x / FL * mw) - 1, my + (int)(M.ball.y / FW * mh) - 1, 3, 3, C_WHITE);
  int cx0 = mx + (int)((M.cx - 240) / FL * mw), cy0 = my + (int)((M.cy - 136) / FW * mh);
  int cw = (int)(480 / FL * mw), ch = (int)(272 / FW * mh);
  box(cx0, cy0, cw, 1, COL(255, 255, 0, 200)); box(cx0, cy0 + ch, cw, 1, COL(255, 255, 0, 200));
  box(cx0, cy0, 1, ch, COL(255, 255, 0, 200)); box(cx0 + cw, cy0, 1, ch, COL(255, 255, 0, 200));
}

/* ---------------- partida ---------------- */
typedef struct { float y; int ball; Player *p; } Item;
static int cmpItem(const void *a, const void *b) {
  float d = ((const Item *)a)->y - ((const Item *)b)->y;
  return (d < 0) ? -1 : (d > 0 ? 1 : 0);
}

void drawHUD(void) {
  char buf[96];
  box(150, 0, 180, 22, COL(0, 0, 0, 170));
  const Team *t1 = &TEAMS[M.team[1]], *t2 = &TEAMS[M.team[2]];
  if (M.training) {
    snprintf(buf, sizeof buf, "TREINO  GOLS: %d", M.score[1]);
    ctxt(3, buf, C_YELLOW, 240);
  } else {
    txt(156, 7, t1->shortn, C_WHITE);
    snprintf(buf, sizeof buf, "%d x %d", M.score[1], M.score[2]);
    ctxt(7, buf, C_YELLOW, 240);
    txt(330 - 8 - 24, 7, t2->shortn, C_WHITE);
    int mins = (int)(M.hf / (float)M.halfFrames * 45) + ((M.half == 2) ? 45 : 0);
    if (mins > 90) mins = 90;
    box(210, 22, 60, 12, COL(0, 0, 0, 150));
    snprintf(buf, sizeof buf, "%02d'", mins);
    ctxt(24, buf, C_WHITE, 240);
  }
  if (M.state == ST_KICK && M.st > 20) ctxt(110, "PREPARAR...", C_WHITE, 240);
  if (M.state == ST_FOUL) {
    box(100, 95, 280, 46, C_DIM);
    ctxt(103, M.foulKind == 1 ? "IMPEDIMENTO!" : (M.foulPen ? "PENALTI!" : "TIRO LIVRE"), C_YELLOW, 240);
    ctxt(122, M.foulKind == 1 ? "tiro livre indireto" : M.msg, (strstr(M.msg, "VERMELHO")) ? C_RED : C_WHITE, 240);
  }
  if (M.state == ST_REPLAY) {
    if ((TICK / 20) % 2 == 0) { box(196, 40, 88, 16, COL(200, 30, 30, 220)); ctxt(43, "REPLAY", C_WHITE, 240); }
    ctxt(246, "X pula", C_WHITE, 240);
  }
  if (M.reds[1] || M.reds[2] || M.yellows[1] || M.yellows[2]) {
    for (int t = 1; t <= 2; t++) {
      int x0 = (t == 1) ? 154 : 304;
      for (int i = 0; i < M.yellows[t] && i < 4; i++) box(x0 + i * 6, 17, 4, 5, COL(255, 220, 40, 255));
      for (int i = 0; i < M.reds[t] && i < 3; i++) box(x0 + 26 + i * 6, 17, 4, 5, COL(230, 40, 40, 255));
    }
  }
  if (M.state == ST_GOAL && M.st > 100 - 1000) {
    if (T_CROWD[0] >= 0) plat_blit(T_CROWD[0], 0, 0, 0, 0, 0, 208, 255, 255, 255, 255);
    box(120, 95, 240, 50, C_DIM);
    ctxt(103, "GOOOL!", C_YELLOW, 240);
    ctxt(122, TEAMS[M.team[M.scorer]].name, C_WHITE, 240);
  }
  if (M.state == ST_HALF) {
    box(120, 95, 240, 56, C_DIM);
    ctxt(103, "FIM DO 1o TEMPO", C_YELLOW, 240);
    snprintf(buf, sizeof buf, "%d x %d   (X continua)", M.score[1], M.score[2]);
    ctxt(122, buf, C_WHITE, 240);
  }
  if (M.state == ST_FULL) {
    box(110, 90, 260, 66, C_DIM);
    ctxt(98, "FIM DE JOGO", C_YELLOW, 240);
    snprintf(buf, sizeof buf, "%s %d x %d %s", t1->name, M.score[1], M.score[2], t2->name);
    ctxt(116, buf, C_WHITE, 240);
    ctxt(134, "X para continuar", C_GREY, 240);
  }
}

void drawMatchView(void) {
  if (M.big) drawBigPitch(); else drawBG(bgTex());
  if (M.rule == 1) txt(4, 40, "GOL DE OURO", C_YELLOW); else if (M.rule == 2) txt(4, 40, "1o A 3 GOLS", C_YELLOW); else if (M.rule == 3) txt(4, 40, "SEM FALTAS", C_YELLOW);
  Item items[MAXP + 1]; int n = 0;
  for (int k = 0; k < M.np; k++) { if (M.pl[k].out) continue; items[n].y = M.pl[k].y; items[n].ball = 0; items[n].p = &M.pl[k]; n++; }
  items[n].y = M.ball.y + 0.5f; items[n].ball = 1; items[n].p = NULL; n++;
  qsort(items, n, sizeof(Item), cmpItem);
  for (int k = 0; k < n; k++) {
    float sx, sy;
    if (items[k].ball) {
      Ball *b = &M.ball;
      toScreen(b->x, b->y, &sx, &sy);
      if (sx < -20 || sx > SW + 20 || sy < -20 || sy > SH + 60) continue;
      drawFrame(FR_SHADOW, sx, sy + 1);
      drawFrame(FR_BALL, sx, sy - b->z * 0.55f);
    } else {
      Player *p = items[k].p;
      toScreen(p->x, p->y, &sx, &sy);
      if (sx < -30 || sx > SW + 30 || sy < -10 || sy > SH + 60) continue;
      drawFrame(FR_SHADOW, sx, sy);
      drawFrame((p->t == 1) ? FR_RING_H : FR_RING_A, sx, sy);
      int fi = 0, oy = 0;
      if (p->moving) { int ph = (p->anim / 7) % 2; fi = ph == 0 ? 1 : 2; oy = -ph; }
      if (p->gk) { if (p->da > 0) { fi = 3; oy = 0; } }
      else if (p->ka > 0) fi = 3;
      if (p->stun > 0) { fi = 0; oy = 1; }
      drawKit(p->kit, fi, sx, sy + oy);
      if (p == M.ctrl && M.state != ST_GOAL) drawFrame(FR_ARROW, sx, sy - 5 + (M.hf / 8) % 2);
    }
  }
  if (M.ctrl && M.charge > 0) {
    float sx, sy; toScreen(M.ctrl->x, M.ctrl->y, &sx, &sy);
    box((int)sx - 15, (int)sy + 3, 30, 4, C_BLACK);
    box((int)sx - 14, (int)sy + 4, (int)(28 * M.charge), 2, (M.charge > 0.85f) ? C_RED : C_GREEN);
  }
  if (M.ctrl && M.state != ST_GOAL) {
    char nb[48];
    snprintf(nb, sizeof nb, "#%d %s", M.ctrl->i, playerName(M.ctrl));
    int w = (int)strlen(nb) * 8 + 8;
    box(4, 252, w, 19, COL(0, 0, 0, 150));
    txt(8, 253, nb, C_WHITE);
    box(8, 265, w - 8, 3, C_BLACK);
    box(8, 265, (int)((w - 8) * M.ctrl->sta), 3, (M.ctrl->sta > 0.4f) ? C_GREEN : C_RED);
  }
  if (M.big) drawMinimap();
  drawHUD();
}

/* ---------------- taticas ---------------- */
static const char *MENT[3] = { "DEFENSIVA", "EQUILIBRADA", "OFENSIVA" };
static const char *PRESS[3] = { "BAIXA", "MEDIA", "ALTA" };
static const char *WIDTH[3] = { "ESTREITA", "NORMAL", "LARGA" };
static const char *MARK[2] = { "POR ZONA", "HOMEM A HOMEM" };

/* desenha e edita; devolve 1 quando o usuario sai (triangulo ou OK) */
int tacEdit(Tac *t, int *formIdx, int kind, int y0, int *sel) {
  char b[64];
  const char *lab[6]; char vals[6][32];
  snprintf(vals[0], 32, "%s", formName(kind, *formIdx));
  snprintf(vals[1], 32, "%s", MENT[clampi(t->ment, 0, 2)]);
  snprintf(vals[2], 32, "%s", PRESS[clampi(t->press, 0, 2)]);
  snprintf(vals[3], 32, "%s", WIDTH[clampi(t->width, 0, 2)]);
  snprintf(vals[4], 32, "%s", MARK[clampi(t->mark, 0, 1)]);
  snprintf(vals[5], 32, "OK");
  lab[0] = "FORMACAO"; lab[1] = "MENTALIDADE"; lab[2] = "PRESSAO"; lab[3] = "LARGURA"; lab[4] = "MARCACAO"; lab[5] = "";
  for (int i = 0; i < 6; i++) {
    int y = y0 + i * 22;
    if (*sel == i) box(60, y - 4, 360, 20, C_SEL);
    Col c = (*sel == i) ? C_BLACK : C_WHITE;
    if (i < 5) {
      txt(70, y, lab[i], c);
      snprintf(b, sizeof b, "< %s >", vals[i]);
      txt(410 - (int)strlen(b) * 8, y, b, c);
    } else ctxt(y, "OK", c, 240);
  }
  if (hit(B_UP)) *sel = (*sel > 0) ? *sel - 1 : 5;
  if (hit(B_DOWN)) *sel = (*sel < 5) ? *sel + 1 : 0;
  int d = 0;
  if (hit(B_LEFT)) d = -1;
  if (hit(B_RIGHT)) d = 1;
  if (d) {
    int n;
    switch (*sel) {
      case 0: n = formCount(kind); *formIdx = (*formIdx + d + n) % n; break;
      case 1: t->ment = (t->ment + d + 3) % 3; break;
      case 2: t->press = (t->press + d + 3) % 3; break;
      case 3: t->width = (t->width + d + 3) % 3; break;
      case 4: t->mark = (t->mark + d + 2) % 2; break;
    }
  }
  if (hit(B_TRIANGLE)) return 1;
  if (hit(B_CROSS) && *sel == 5) return 1;
  return 0;
}

/* ---------------- pausa ---------------- */
static void drawPauseMain(void) {
  char b[48];
  box(140, 56, 200, 136, C_PANEL);
  ctxt(62, "PAUSA", C_YELLOW, 240);
  static const char *items[5] = { "CONTINUAR", "TATICA", "SUBSTITUICAO", "REINICIAR PARTIDA", "SAIR PARA O MENU" };
  for (int i = 0; i < 5; i++) {
    if (i == 2) snprintf(b, sizeof b, "SUBSTITUICAO (%d)", M.subsLeft); else snprintf(b, sizeof b, "%s", items[i]);
    ctxt(84 + i * 20, b, (M.pmenu == i) ? C_SEL : C_WHITE, 240);
  }
}

static void drawPauseSubs(void) {
  char b[64];
  box(80, 30, 320, 210, C_PANEL);
  snprintf(b, sizeof b, "SUBSTITUICAO  (restam %d)", M.subsLeft);
  ctxt(36, b, C_YELLOW, 240);
  int row = 0;
  for (int k = 0; k < M.np; k++) {
    Player *p = &M.pl[k];
    if (p->t != 1 || p->gk || p->out) continue;
    int y = 58 + row * 16;
    if (row == M.psel) box(88, y - 3, 304, 15, C_SEL);
    Col c = (row == M.psel) ? C_BLACK : C_WHITE;
    snprintf(b, sizeof b, "#%-2d %s", p->i, playerName(p));
    txt(94, y, b, c);
    box(300, y + 2, 80, 6, C_BLACK);
    box(301, y + 3, (int)(78 * p->sta), 4, (p->sta > 0.4f) ? C_GREEN : C_RED);
    row++;
  }
  ctxt(226, "X substitui    TRIANGULO volta", C_GREY, 240);
}

static int subRows(void) {
  int n = 0;
  for (int k = 0; k < M.np; k++) if (M.pl[k].t == 1 && !M.pl[k].gk && !M.pl[k].out) n++;
  return n;
}

void sceneMatch(void) {
  if (!M.active) { SCENE = SC_MENU; return; }
  if (M.paused) {
    drawMatchView();
    if (M.pkind == PM_MAIN) {
      drawPauseMain();
      if (hit(B_UP)) M.pmenu = (M.pmenu > 0) ? M.pmenu - 1 : 0;
      if (hit(B_DOWN)) M.pmenu = (M.pmenu < 4) ? M.pmenu + 1 : 4;
      if (hit(B_START) || hit(B_TRIANGLE)) M.paused = 0;
      if (hit(B_CROSS)) {
        if (M.pmenu == 0) M.paused = 0;
        else if (M.pmenu == 1) { M.pkind = PM_TACTIC; M.psel = 0; }
        else if (M.pmenu == 2) { M.pkind = PM_SUBS; M.psel = 0; }
        else if (M.pmenu == 3) newMatch(M.team[1], M.team[2], M.futsal, M.training, M.trainKit, M.mode);
        else { M.active = 0; SCENE = SC_MENU; }
      }
    } else if (M.pkind == PM_TACTIC) {
      box(50, 40, 380, 190, C_PANEL);
      ctxt(46, "TATICA DA EQUIPE", C_YELLOW, 240);
      int kind = M.futsal ? 1 : (M.big ? 2 : 0);
      int fi = M.formIdx[1];
      if (tacEdit(&M.tac[1], &fi, kind, 74, &M.psel)) {
        if (!M.mgr) { UserTac = M.tac[1]; UserForm = M.formIdx[1]; }
        else { MGR.tac = M.tac[1]; MGR.form = M.formIdx[1]; }
        M.pkind = PM_MAIN;
      }
      if (fi != M.formIdx[1]) applyFormation(1, fi);
    } else {
      drawPauseSubs();
      int rows = subRows();
      if (hit(B_UP)) M.psel = (M.psel > 0) ? M.psel - 1 : rows - 1;
      if (hit(B_DOWN)) M.psel = (M.psel < rows - 1) ? M.psel + 1 : 0;
      if (hit(B_TRIANGLE)) M.pkind = PM_MAIN;
      if (hit(B_CROSS) && M.subsLeft > 0) {
        int row = 0;
        for (int k = 0; k < M.np; k++) {
          Player *p = &M.pl[k];
          if (p->t != 1 || p->gk || p->out) continue;
          if (row == M.psel) { doSubstitution(k); break; }
          row++;
        }
      }
    }
    return;
  }
  updateMatch();
  if (M.active && SCENE == SC_MATCH) drawMatchView();
}
