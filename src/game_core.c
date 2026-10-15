/* Super Pixel Soccer PSP - nucleo: utilitarios e motor da partida */
#include "game.h"

Match M;
Opt OPT = { 2, 2, 1, 0, 1, 0, 0, 1, 1 };
Tac UserTac = { 1, 1, 1, 0 };
int SCENE = SC_TITLE, TICK = 0, UserForm = 0, NextRule = 0, NextChal = -1;
int T_ATLAS = -1, T_BG[8] = { -1, -1, -1, -1, -1, -1, -1, -1 }, T_STAND = -1, T_BG_COURT = -1, T_TITLE = -1, T_FONT = -1, T_COACH = -1, T_MENU = -1;
int T_CROWD[4] = { -1, -1, -1, -1 };
int S_KICK = -1, S_WHISTLE = -1, S_GOAL = -1, S_POST = -1, S_CHEER = -1, S_MUSIC = -1, S_AMB = -1;
Result RES;
int dbgFouls = 0, dbgReds = 0, dbgYellows = 0, dbgPens = 0, dbgAllAI = 0, dbgOffside = 0, dbgReplays = 0;
int clampi(int v, int a, int b) { return v < a ? a : (v > b ? b : v); }

Team TEAMS[NTEAMS] = TEAMS_INIT;
static int rhead = 0, rcount = 0;
static int wouldBeOffside(const Player *p, const Player *q);

/* ---------------- formacoes: {rx, ry}, rx 0 = proprio gol ---------------- */
static const float FORMS6[4][6][2] = {
  { {0.04f,0.5f}, {0.22f,0.24f}, {0.22f,0.76f}, {0.45f,0.5f}, {0.68f,0.3f}, {0.68f,0.7f} },
  { {0.04f,0.5f}, {0.22f,0.24f}, {0.22f,0.76f}, {0.5f,0.3f}, {0.5f,0.7f}, {0.7f,0.5f} },
  { {0.04f,0.5f}, {0.2f,0.2f}, {0.2f,0.8f}, {0.5f,0.5f}, {0.2f,0.5f}, {0.7f,0.5f} },
  { {0.04f,0.5f}, {0.2f,0.5f}, {0.45f,0.2f}, {0.45f,0.5f}, {0.45f,0.8f}, {0.7f,0.5f} }
};
static const char *FNAMES6[4] = { "2-1-2", "2-2-1", "3-1-1", "1-3-1" };
static const float FORMS5[3][5][2] = {
  { {0.04f,0.5f}, {0.24f,0.3f}, {0.24f,0.7f}, {0.6f,0.3f}, {0.6f,0.7f} },
  { {0.04f,0.5f}, {0.22f,0.5f}, {0.5f,0.25f}, {0.5f,0.75f}, {0.68f,0.5f} },
  { {0.04f,0.5f}, {0.2f,0.2f}, {0.2f,0.8f}, {0.62f,0.5f}, {0.2f,0.5f} }
};
static const char *FNAMES5[3] = { "2-2", "1-2-1", "3-1" };
static const float FORMS11[4][11][2] = {
  { {0.04f,0.5f}, {0.2f,0.14f}, {0.2f,0.38f}, {0.2f,0.62f}, {0.2f,0.86f}, {0.45f,0.14f}, {0.45f,0.38f}, {0.45f,0.62f}, {0.45f,0.86f}, {0.72f,0.38f}, {0.72f,0.62f} },
  { {0.04f,0.5f}, {0.2f,0.14f}, {0.2f,0.38f}, {0.2f,0.62f}, {0.2f,0.86f}, {0.45f,0.28f}, {0.45f,0.5f}, {0.45f,0.72f}, {0.72f,0.18f}, {0.76f,0.5f}, {0.72f,0.82f} },
  { {0.04f,0.5f}, {0.2f,0.25f}, {0.2f,0.5f}, {0.2f,0.75f}, {0.42f,0.1f}, {0.42f,0.3f}, {0.45f,0.5f}, {0.42f,0.7f}, {0.42f,0.9f}, {0.72f,0.38f}, {0.72f,0.62f} },
  { {0.04f,0.5f}, {0.2f,0.14f}, {0.2f,0.38f}, {0.2f,0.62f}, {0.2f,0.86f}, {0.4f,0.1f}, {0.4f,0.3f}, {0.45f,0.5f}, {0.4f,0.7f}, {0.4f,0.9f}, {0.74f,0.5f} }
};
static const char *FNAMES11[4] = { "4-4-2", "4-3-3", "3-5-2", "4-5-1" };
int formKind(int futsal) { return futsal ? 1 : (OPT.big ? 2 : 0); }
int formCount(int kind) { return kind == 1 ? 3 : 4; }
const char *formName(int kind, int i) {
  if (kind == 1) return FNAMES5[clampi(i, 0, 2)];
  if (kind == 2) return FNAMES11[clampi(i, 0, 3)];
  return FNAMES6[clampi(i, 0, 3)];
}

static const char *NAMES[64] = {
  "BIGU","TICO","NEGO","DUDA","LELO","JUNIOR","CAIO","TUTA","ZECA","MAIKE","RENAN","BETO","PIRES","DEDE","LUCAO","NANDO",
  "TOCHA","FABIO","GUGA","IGOR","JEFF","KAIKE","LEO","MURILO","NENE","OTAVIO","PEDRAO","QUICO","RAFA","SERGIO","TIAGO","UBIRA",
  "VITOR","WALDO","XAVIER","YURI","ZICO JR","ALAN","BRUNO","CESAR","DIOGO","EDU","FELIPE","GUTO","HUGO","ISAAC","JOCA","KLEBER",
  "LUAN","MARCOS","NILTON","OSMAR","PAULO","RUBENS","SAMUEL","TONHO","UBALDO","VINI","WESLEY","XANDE","YAGO","ZEZE","ARTHUR","BENTO"
};
const char *nameOf(int idx) { return NAMES[((idx % 64) + 64) % 64]; }
static unsigned hsh(unsigned a) { a *= 2654435761u; a ^= a >> 15; a *= 2246822519u; a ^= a >> 13; return a; }
const char *playerName(const Player *p) {
  if (M.training && p->t == 1) return "VOCE";
  if (p->gk && p->roster < 0) return "GOLEIRO";
  return nameOf(p->nameIdx);
}
const char *THEME_NAMES[8] = { "DIA", "NOITE", "NEVE", "AREIA", "ARGILA", "ESMERALDA", "CREPUSCULO", "CHUVA" };
int bgForTheme(int theme) {
  static const char *files[8] = { "gfx/bg_field.png", "gfx/bg_night.png", "gfx/bg_snow.png", "gfx/bg_sand.png", "gfx/bg_clay.png", "gfx/bg_emerald.png", "gfx/bg_dusk.png", "gfx/bg_rain.png" };
  theme = clampi(theme, 0, 7);
  if (T_BG[theme] < 0 && theme > 0) T_BG[theme] = plat_load_tex(files[theme]);
  return (T_BG[theme] >= 0) ? T_BG[theme] : T_BG[0];
}
int bgTex(void) { return M.futsal ? T_BG_COURT : bgForTheme(OPT.stadium); }

/* ---------------- utilitarios ---------------- */
static int cur[12], prv[12];
static int AX, AY;

void readPad(void) {
  memcpy(prv, cur, sizeof(cur));
  plat_input(cur, &AX, &AY);
}
int hit(int b) { return cur[b] && !prv[b]; }
int held(int b) { return cur[b]; }

float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
float prob(void) { return (float)rand() / (float)RAND_MAX; }
float rndf(float a, float b) { return a + prob() * (b - a); }
float dist(float ax, float ay, float bx, float by) { float dx = ax - bx, dy = ay - by; return sqrtf(dx * dx + dy * dy); }
float normf(float dx, float dy, float *ox, float *oy) {
  float l = sqrtf(dx * dx + dy * dy);
  if (l < 0.0001f) { *ox = 0; *oy = 0; return 0; }
  *ox = dx / l; *oy = dy / l; return l;
}

void dirInput(float *odx, float *ody) {
  float dx = 0, dy = 0;
  if (cur[B_LEFT]) dx -= 1;
  if (cur[B_RIGHT]) dx += 1;
  if (cur[B_UP]) dy -= 1;
  if (cur[B_DOWN]) dy += 1;
  if (abs(AX) > 45) dx = AX / 128.0f;
  if (abs(AY) > 45) dy = AY / 128.0f;
  float l = sqrtf(dx * dx + dy * dy);
  if (l > 1) { dx /= l; dy /= l; }
  *odx = dx; *ody = dy;
}

static void drawText(int x, int y, const char *s, Col c) {
  for (; *s; s++, x += 8) {
    int ch = (unsigned char)*s;
    if (ch < 32 || ch > 127) ch = '?';
    int idx = ch - 32;
    plat_blit(T_FONT, (idx % 16) * 8, (idx / 16) * 11, 8, 11, x, y, c.r, c.g, c.b, c.a);
  }
}
void txt(int x, int y, const char *s, Col c) { drawText(x + 1, y + 1, s, C_BLACK); drawText(x, y, s, c); }
void ctxt(int y, const char *s, Col c, int cx) { txt(cx - (int)strlen(s) * 4, y, s, c); }
void box(int x, int y, int w, int h, Col c) { plat_rect(x, y, w, h, c.r, c.g, c.b, c.a); }
void drawBG(int tex) {
  if (tex >= 0) plat_blit(tex, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255);
  else plat_clear(30, 110, 40);
}
void drawFrame(const short *fr, float x, float y) {
  if (T_ATLAS < 0) return;
  plat_blit(T_ATLAS, fr[0], fr[1], 40, 40, (int)floorf(x - 20), (int)floorf(y - 39), 255, 255, 255, 255);
}
void drawKit(int kit, int fi, float x, float y) {
  if (T_ATLAS < 0) return;
  const short *fr = KFR[kit][fi];
  plat_blit_flip(T_ATLAS, fr[0], fr[1], 40, 40, (int)floorf(x - 20), (int)floorf(y - 39), fi == 2);
}
void playSnd(int id) { if (OPT.sound && id >= 0) plat_snd_play(id); }

void saveOpt(void) {
  FILE *f = fopen("save.txt", "w");
  if (f) {
    fprintf(f, "%d %d %d %d %d %d %d %d %d %d %d %d %d", OPT.diff, OPT.half, OPT.sound, OPT.stadium, OPT.music, OPT.ball, OPT.big,
            OPT.replay, OPT.offside, UserTac.ment, UserTac.press, UserTac.width, UserTac.mark);
    fclose(f);
  }
}
void loadOpt(void) {
  FILE *f = fopen("save.txt", "r");
  if (f) {
    int v[13] = { 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0 };
    int n = fscanf(f, "%d %d %d %d %d %d %d %d %d %d %d %d %d", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9], &v[10], &v[11], &v[12]);
    if (n >= 3 && v[0] >= 1 && v[0] <= 3 && v[1] >= 1 && v[1] <= 3) {
      OPT.diff = v[0]; OPT.half = v[1]; OPT.sound = v[2];
      OPT.stadium = (n >= 4) ? clampi(v[3], 0, 7) : 0; OPT.music = (n >= 5) ? v[4] : 1;
      OPT.ball = (n >= 6) ? clampi(v[5], 0, 3) : 0; OPT.big = (n >= 7) ? (v[6] != 0) : 0;
      if (n >= 13) {
        OPT.replay = (v[7] != 0); OPT.offside = (v[8] != 0);
        UserTac.ment = clampi(v[9], 0, 2); UserTac.press = clampi(v[10], 0, 2); UserTac.width = clampi(v[11], 0, 2); UserTac.mark = clampi(v[12], 0, 1);
      }
    }
    fclose(f);
  }
}

/* ---------------- geometria ---------------- */
void setFieldSize(int big) {
  M.big = big;
  if (big) { M.fl = 1700; M.fw = 900; M.gy1 = 350; M.gy2 = 550; M.sc = 2.1f; M.spd = 1.4f; M.bsp = 1.4f; M.rs = 1.25f; }
  else { M.fl = PFL; M.fw = PFW; M.gy1 = PGY1; M.gy2 = PGY2; M.sc = 1; M.spd = 1; M.bsp = 1; M.rs = 1; }
}

void toScreenSmall(float x, float y, float *sx, float *sy) {
  float u = x / PFL, v = y / PFW;
  float f = (v < 0) ? v : powf(v, 1.14f);
  float l = 58 - 50 * f, r = 422 + 50 * f;
  *sx = l + (r - l) * u;
  *sy = 70 + 174 * f;
}
void toScreen(float x, float y, float *sx, float *sy) {
  if (M.big) { *sx = x - M.cx + 240; *sy = y - M.cy + 136; }
  else toScreenSmall(x, y, sx, sy);
}

static float atkX(int ti) { return M.dir[ti] == 1 ? FL : 0; }
static float ownX(int ti) { return M.dir[ti] == 1 ? 0 : FL; }
static float goalCY(void) { return (GOAL_Y1 + GOAL_Y2) * 0.5f; }

static void homePos(const Player *p, float *hx, float *hy) {
  const float *f = M.form[p->t][p->i - 1];
  const Tac *tc = &M.tac[p->t];
  float rx = f[0], ry = f[1];
  if (p->i > 1) {
    rx += (tc->ment - 1) * 0.07f;
    rx = clampf(rx, 0.08f, 0.95f);
    float wd = (tc->width == 0) ? 0.8f : ((tc->width == 2) ? 1.2f : 1.0f);
    ry = clampf(0.5f + (ry - 0.5f) * wd, 0.06f, 0.94f);
  }
  if (M.dir[p->t] == 1) { *hx = rx * FL; *hy = ry * FW; }
  else { *hx = (1 - rx) * FL; *hy = ry * FW; }
}

static int roleOf(float rx) { return rx < 0.1f ? 0 : (rx < 0.34f ? 1 : (rx < 0.6f ? 2 : 3)); }

int formRole(int kind, int form, int slot) {
  form = clampi(form, 0, formCount(kind) - 1);
  const float *f = (kind == 1) ? FORMS5[form][slot] : ((kind == 2) ? FORMS11[form][slot] : FORMS6[form][slot]);
  return roleOf(f[0]);
}

static int teamKit(int ti, int n) {
  const Team *tm = &TEAMS[M.team[ti]];
  if (n == 1) return tm->gk;
  if (tm->nkits > 0) return tm->kits[(n - 2) % tm->nkits];
  return tm->kit;
}

Player *nearestField(int ti, float x, float y, const Player *skip) {
  Player *best = NULL; float bd = 1e9f;
  for (int k = 0; k < M.np; k++) {
    Player *p = &M.pl[k];
    if (p->t == ti && !p->gk && !p->out && p != skip) {
      float d = dist(p->x, p->y, x, y);
      if (d < bd) { best = p; bd = d; }
    }
  }
  return best;
}

static void startKickoff(int ti);

static void aiTactic(Tac *t, int str) {
  t->ment = (str >= 4) ? ((prob() < 0.6f) ? 2 : 1) : ((str <= 2) ? ((prob() < 0.5f) ? 0 : 1) : 1);
  t->press = rand() % 3; t->width = rand() % 3; t->mark = rand() % 2;
}

void newMatch(int t1, int t2, int futsal, int training, int trainKit, int mode) {
  memset(&M, 0, sizeof(M));
  M.team[1] = t1; M.team[2] = t2; M.mode = mode;
  M.futsal = futsal; M.training = training; M.trainKit = trainKit;
  M.half = 1; M.dir[1] = 1; M.dir[2] = -1; M.active = 1; M.subsLeft = 3;
  M.mgr = (MGR.active && mode == MODE_LEAGUE && t1 == MGR.team);
  M.rule = NextRule; NextRule = 0; M.chal = NextChal; NextChal = -1;
  int big = ((OPT.big || M.mgr) && !futsal && !training);
  setFieldSize(big);
  M.cx = FL / 2; M.cy = FW / 2;
  static const int secs[3] = { 45, 90, 150 };
  M.halfFrames = secs[clampi(OPT.half, 1, 3) - 1] * 60;
  int kind = futsal ? 1 : (big ? 2 : 0);
  M.nform = futsal ? 5 : (big ? 11 : 6);
  for (int ti = 1; ti <= 2; ti++) {
    int fi = (ti == 1) ? (M.mgr ? MGR.form : UserForm) : rand() % formCount(kind);
    fi = clampi(fi, 0, formCount(kind) - 1);
    M.formIdx[ti] = fi;
    M.form[ti] = futsal ? FORMS5[fi] : (big ? FORMS11[fi] : FORMS6[fi]);
  }
  M.tac[1] = M.mgr ? MGR.tac : UserTac;
  aiTactic(&M.tac[2], TEAMS[t2].str);
  M.ball.x = FL / 2; M.ball.y = FW / 2; M.ball.last = 1;
  for (int ti = 1; ti <= 2; ti++) {
    int list[11], n = 0;
    if (training) {
      if (ti == 1) { list[n++] = 4; } else { list[n++] = 1; list[n++] = 2; list[n++] = 3; }
    } else {
      for (int k = 1; k <= M.nform; k++) list[n++] = k;
    }
    for (int k = 0; k < n; k++) {
      Player *p = &M.pl[M.np++];
      memset(p, 0, sizeof(*p));
      int sl = list[k];
      p->t = ti; p->i = sl; p->gk = (sl == 1); p->roster = -1; p->sta = 1.0f;
      p->fx = (ti == 1) ? 1.0f : -1.0f;
      p->kit = teamKit(ti, sl);
      if (training && ti == 1) p->kit = trainKit;
      int tid = M.team[ti], str = TEAMS[tid].str;
      float v1 = (hsh(tid * 31 + sl * 7 + 3) % 100) / 100.0f - 0.5f;
      float v2 = (hsh(tid * 17 + sl * 11 + 5) % 100) / 100.0f - 0.5f;
      float v3 = (hsh(tid * 13 + sl * 19 + 9) % 100) / 100.0f - 0.5f;
      p->pace = 0.94f + 0.03f * str + v1 * 0.10f;
      p->shot = 0.93f + 0.03f * str + v2 * 0.12f;
      p->defn = 1.0f + v3 * 0.20f;
      p->nameIdx = (int)(hsh(tid * 29 + sl * 3 + 1) % 64);
      if (M.mgr && ti == 1) {
        int role = roleOf(M.form[1][sl - 1][0]), nm, ro;
        float pa, sh, de;
        if (mgrAssign(sl - 1, role, &nm, &pa, &sh, &de, &ro)) { p->nameIdx = nm; p->pace = pa; p->shot = sh; p->defn = de; p->roster = ro; }
      }
      if (p->gk) p->pace *= 0.9f;
      if (training && ti == 1) { p->pace = 1.1f; p->shot = 1.1f; p->defn = 1.0f; }
    }
  }
  if (M.chal >= 0) chalSetup(M.chal);
  startKickoff(1);
}

static void startKickoff(int ti) {
  rcount = 0; rhead = 0; M.offQ = NULL;
  M.kickTeam = ti; M.state = ST_KICK; M.st = 80;
  Ball *b = &M.ball;
  b->x = FL / 2; b->y = FW / 2; b->z = b->vx = b->vy = b->vz = 0; b->owner = NULL; b->lastp = NULL;
  Player *kicker = NULL; float bestScore = -1e9f;
  for (int k = 0; k < M.np; k++) {
    Player *p = &M.pl[k];
    float hx, hy; homePos(p, &hx, &hy);
    int d = M.dir[p->t];
    if (M.training) { p->x = hx; p->y = hy; }
    else { p->x = FL / 2 + (hx - FL / 2) * 0.7f; p->y = hy; }
    p->vx = p->vy = 0; p->stun = p->tack = p->kick = p->hold = p->ka = p->da = 0;
    p->fx = (float)d; p->fy = 0;
    if (p->t == ti && !p->gk && !p->out) {
      const float *f = M.form[p->t][p->i - 1];
      float sc = f[0] * 10 - fabsf(f[1] - 0.5f) * 20;
      if (sc > bestScore) { bestScore = sc; kicker = p; }
    }
  }
  if (kicker) {
    kicker->x = FL / 2 - M.dir[ti] * 4; kicker->y = FW / 2;
    b->owner = kicker; b->last = ti; b->lastp = kicker;
  }
  if (M.training) {
    Player *h = NULL;
    for (int k = 0; k < M.np; k++) if (M.pl[k].t == 1) h = &M.pl[k];
    M.ctrl = h;
    if (h) { h->x = 300; h->y = FW / 2; b->x = 310; b->y = FW / 2; b->owner = NULL; }
  } else {
    M.ctrl = nearestField(1, b->x, b->y, NULL);
  }
  M.cx = clampf(b->x, 180, FL - 180); M.cy = clampf(b->y, 120, FW - 120);
}

void applyFormation(int ti, int idx) {
  int kind = M.futsal ? 1 : (M.big ? 2 : 0);
  idx = clampi(idx, 0, formCount(kind) - 1);
  M.formIdx[ti] = idx;
  M.form[ti] = M.futsal ? FORMS5[idx] : (M.big ? FORMS11[idx] : FORMS6[idx]);
}

void doSubstitution(int pi) {
  Player *p = &M.pl[pi];
  if (M.subsLeft <= 0 || p->gk || p->t != 1) return;
  M.subsLeft--;
  p->nameIdx = (int)(hsh((unsigned)(p->nameIdx * 7 + TICK + 13)) % 64);
  int str = TEAMS[M.team[1]].str;
  p->pace = 0.94f + 0.03f * str + rndf(-0.05f, 0.06f);
  p->shot = 0.93f + 0.03f * str + rndf(-0.06f, 0.07f);
  p->defn = 1.0f + rndf(-0.1f, 0.1f);
  p->sta = 1.0f; p->yellow = 0; p->roster = -1;
}

/* ---------------- chutes e passes ---------------- */
static void releaseBall(Player *p) {
  Ball *b = &M.ball;
  b->owner = NULL; b->last = p->t; b->lastp = p;
  p->kick = 22; p->hold = 0; p->ka = 12;
  b->x = p->x + p->fx * 8; b->y = p->y + p->fy * 8;
}

static void doShoot(Player *p, float power, int hasAim, float ax, float ay) {
  Ball *b = &M.ball;
  M.offQ = NULL;
  releaseBall(p);
  float dx, dy;
  normf(atkX(p->t) - p->x, goalCY() - p->y, &dx, &dy);
  if (hasAim) {
    float mx = ax * 0.7f + dx * 0.3f, my = ay * 0.7f + dy * 0.3f;
    normf(mx, my, &dx, &dy);
  }
  float err = ((1.15f - power) * 0.10f + 0.015f) / clampf(p->shot, 0.85f, 1.15f);
  err *= (0.75f + 0.25f * p->sta);
  if (p->t == 2 || !M.humanShot) err += (3 - OPT.diff) * 0.012f;
  float ang = atan2f(dy, dx) + rndf(-err, err);
  float sp = (6.5f + power * 8) * clampf(p->shot, 0.85f, 1.15f) * M.bsp;
  b->vx = cosf(ang) * sp; b->vy = sinf(ang) * sp;
  b->vz = 0.7f + power * 1.7f + rndf(0, 0.5f);
  M.shots[p->t]++;
  playSnd(S_KICK);
}

static int countBlockers(float x0, float y0, float x1, float y1, int myT, float rad) {
  int n = 0;
  float dx = x1 - x0, dy = y1 - y0, L2 = dx * dx + dy * dy;
  if (L2 < 1) return 0;
  for (int k = 0; k < M.np; k++) {
    Player *o = &M.pl[k];
    if (o->t == myT || o->out) continue;
    float t = ((o->x - x0) * dx + (o->y - y0) * dy) / L2;
    if (t < 0.05f || t > 1.0f) continue;
    if (dist(o->x, o->y, x0 + t * dx, y0 + t * dy) < rad) n++;
  }
  return n;
}

static float nearestOpp(float x, float y, int myT, Player **who) {
  float bd = 1e9f; Player *bw = NULL;
  for (int k = 0; k < M.np; k++) {
    Player *o = &M.pl[k];
    if (o->t == myT || o->out) continue;
    float d = dist(x, y, o->x, o->y);
    if (d < bd) { bd = d; bw = o; }
  }
  if (who) *who = bw;
  return bd;
}

/* passe da IA: escolhe pelo espaco livre, linha de passe e progresso */
static Player *pickMateAI(Player *p, float *outScore) {
  int dir = M.dir[p->t];
  Player *best = NULL; float bs = -1e9f;
  for (int k = 0; k < M.np; k++) {
    Player *q = &M.pl[k];
    if (q->t != p->t || q == p || q->out) continue;
    float d = dist(p->x, p->y, q->x, q->y);
    if (d < 45 * M.rs || d > 520 * SCL) continue;
    if (wouldBeOffside(p, q)) continue;
    float open = fminf(1.0f, nearestOpp(q->x, q->y, p->t, NULL) / (140 * M.rs));
    int blk = countBlockers(p->x, p->y, q->x, q->y, p->t, 16 * M.rs);
    float prog = ((q->x - p->x) * dir) / (300 * SCL);
    float s = prog * 1.1f + open - blk * 0.9f - d / (900 * SCL);
    if (q->gk) s -= 1.2f;
    if (s > bs) { bs = s; best = q; }
  }
  if (outScore) *outScore = bs;
  return best;
}

/* passe do jogador: escolhe pela direcao apontada */
static Player *pickMate(Player *p, float fxv, float fyv) {
  Player *best = NULL; float bs = -1e9f;
  for (int k = 0; k < M.np; k++) {
    Player *q = &M.pl[k];
    if (q->t == p->t && q != p && !q->out) {
      float dx, dy, d = normf(q->x - p->x, q->y - p->y, &dx, &dy);
      if (d > 20) {
        float dot = dx * fxv + dy * fyv;
        float s = dot - d / (700.0f * SCL);
        if (q->gk) s -= 0.6f;
        if (dot > 0.25f && s > bs) { best = q; bs = s; }
      }
    }
  }
  if (!best) {
    float bd = 1e9f;
    for (int k = 0; k < M.np; k++) {
      Player *q = &M.pl[k];
      if (q->t == p->t && q != p && !q->gk && !q->out) {
        float d = dist(p->x, p->y, q->x, q->y);
        if (d < bd) { best = q; bd = d; }
      }
    }
  }
  return best;
}

static int wouldBeOffside(const Player *p, const Player *q);
static void doPassTo(Player *p, Player *q, int lob, int human) {
  Ball *b = &M.ball;
  M.offQ = (q && wouldBeOffside(p, q)) ? q : NULL; M.offT = 150;
  releaseBall(p);
  if (!q) { b->vx = p->fx * 6 * M.bsp; b->vy = p->fy * 6 * M.bsp; playSnd(S_KICK); return; }
  float d = dist(p->x, p->y, q->x, q->y);
  float tx = q->x + q->vx * 8, ty = q->y + q->vy * 8;
  float dx, dy; normf(tx - p->x, ty - p->y, &dx, &dy);
  if (lob) {
    float vz = clampf(d * 0.008f / SCL + 1.8f, 2.0f, 4.6f);
    float T = 2 * vz / GRAV;
    float sp = clampf(d / T, 2, 11 * M.bsp);
    b->vx = dx * sp; b->vy = dy * sp; b->vz = vz;
  } else {
    float sp = clampf((3.6f + d * 0.012f / SCL) * M.bsp, 4 * M.bsp, 10 * M.bsp);
    b->vx = dx * sp; b->vy = dy * sp; b->vz = 0;
  }
  playSnd(S_KICK);
  if (human && p->t == 1 && !q->gk) M.ctrl = q;
}

static void doPass(Player *p, int lob, float fxv, float fyv, int human) {
  doPassTo(p, pickMate(p, fxv, fyv), lob, human);
}

static void callFoul(Player *fouler, Player *victim);
static int attemptTackle(Player *p, Player *o) {
  Ball *b = &M.ball;
  p->tack = 40;
  float chance = 0.42f + (TEAMS[M.team[p->t]].str - TEAMS[M.team[o->t]].str) * 0.04f;
  if (p->t == 2) chance += (OPT.diff - 2) * 0.05f;
  chance += (p->defn - 1.0f) * 0.4f;
  if (prob() < chance) {
    o->stun = 30; b->owner = p; b->last = p->t; b->lastp = p; p->hold = 0;
    if (p->t == 1 && !p->gk) M.ctrl = p;
    return 1;
  }
  p->stun = 22;
  if (!M.training && M.rule != 3 && M.state == ST_PLAY && !p->gk && prob() < 0.40f) callFoul(p, o);
  return 0;
}

static void callFoul(Player *fouler, Player *victim) {
  Ball *b = &M.ball;
  dbgFouls++;
  M.state = ST_FOUL; M.st = 100; M.foulKind = 0; M.offQ = NULL;
  M.foulT = victim->t;
  M.foulX = clampf(victim->x, 10, FL - 10); M.foulY = clampf(victim->y, 10, FW - 10);
  float gx = (M.dir[fouler->t] == 1) ? 0.0f : FL;
  M.foulPen = (fabsf(M.foulX - gx) < 120 * SCL && M.foulY > FW * 0.214f && M.foulY < FW * 0.786f);
  b->owner = NULL; b->vx = b->vy = b->vz = 0; b->z = 0; b->x = M.foulX; b->y = M.foulY;
  float r = prob();
  int red = 0, yel = 0;
  if (r < 0.05f) red = 1; else if (r < 0.30f) yel = 1;
  if (yel) { fouler->yellow++; dbgYellows++; M.yellows[fouler->t]++; if (fouler->yellow >= 2) { red = 1; yel = 0; } }
  if (red) {
    fouler->out = 1; M.reds[fouler->t]++; dbgReds++;
    if (M.ctrl == fouler) M.ctrl = NULL;
    snprintf(M.msg, sizeof M.msg, "CARTAO VERMELHO: %s", playerName(fouler));
  } else if (yel) snprintf(M.msg, sizeof M.msg, "CARTAO AMARELO: %s", playerName(fouler));
  else snprintf(M.msg, sizeof M.msg, "FALTA!");
  if (M.foulPen) dbgPens++;
  playSnd(S_WHISTLE);
}

static void onGoal(int ti);
static void giveGoalKick(int defT) {
  Ball *b = &M.ball;
  Player *gk = NULL;
  for (int k = 0; k < M.np; k++) if (M.pl[k].t == defT && M.pl[k].gk) gk = &M.pl[k];
  if (!gk) return;
  b->x = ((M.dir[defT] == 1) ? 0.0f : FL) + M.dir[defT] * 20 * M.rs; b->y = FW / 2; b->z = b->vx = b->vy = b->vz = 0;
  gk->x = b->x - M.dir[defT] * 4; gk->y = b->y;
  b->owner = gk; b->last = defT; b->lastp = gk; gk->hold = 0; gk->kick = 0;
}

/* ---------------- replay dos gols ---------------- */
#define RBUF 300
typedef struct {
  float px[MAXP], py[MAXP], bx, by, bz, cx, cy;
  short anim[MAXP], ka[MAXP], da[MAXP];
  unsigned char fl[MAXP];
} RFrame;
static RFrame rbuf[RBUF];
static Match rbackup;

static void replayRecord(void) {
  RFrame *r = &rbuf[rhead];
  for (int k = 0; k < M.np; k++) {
    const Player *p = &M.pl[k];
    r->px[k] = p->x; r->py[k] = p->y; r->anim[k] = (short)p->anim; r->ka[k] = (short)p->ka; r->da[k] = (short)p->da;
    r->fl[k] = (unsigned char)((p->moving ? 1 : 0) | (p->out ? 2 : 0) | (p->stun > 0 ? 4 : 0));
  }
  r->bx = M.ball.x; r->by = M.ball.y; r->bz = M.ball.z; r->cx = M.cx; r->cy = M.cy;
  rhead = (rhead + 1) % RBUF;
  if (rcount < RBUF) rcount++;
}
int replayAvailable(void) { return rcount >= 90; }
static void replayStart(void) {
  memcpy(&rbackup, &M, sizeof(M));
  M.state = ST_REPLAY; M.rpos = 0; M.rtick = 0;
  dbgReplays++;
}
static void replayStep(void) {
  int n = rcount < 240 ? rcount : 240;
  M.rtick++;
  if (M.rtick % 2 == 0) M.rpos++;
  if (M.rpos >= n || hit(B_CROSS) || hit(B_START)) {
    memcpy(&M, &rbackup, sizeof(M));
    M.state = ST_GOAL; M.st = 60;
    return;
  }
  int idx = ((rhead - n + M.rpos) % RBUF + RBUF) % RBUF;
  const RFrame *r = &rbuf[idx];
  for (int k = 0; k < M.np; k++) {
    Player *p = &M.pl[k];
    p->x = r->px[k]; p->y = r->py[k]; p->anim = r->anim[k]; p->ka = r->ka[k]; p->da = r->da[k];
    p->moving = (r->fl[k] & 1) != 0; p->out = (r->fl[k] & 2) != 0; p->stun = (r->fl[k] & 4) ? 1 : 0;
  }
  M.ball.x = r->bx; M.ball.y = r->by; M.ball.z = r->bz; M.ball.owner = NULL;
  M.cx = r->cx; M.cy = r->cy;
}

/* ---------------- impedimento ---------------- */
static int wouldBeOffside(const Player *p, const Player *q) {
  if (!(M.big && OPT.offside) || q->gk || q->out) return 0;
  int d = M.dir[p->t];
  float qa = q->x * d, pa = p->x * d;
  if (qa <= pa) return 0;
  if ((q->x - FL / 2) * d <= 0) return 0;
  float a1 = -1e9f, a2 = -1e9f;
  for (int k = 0; k < M.np; k++) {
    const Player *o = &M.pl[k];
    if (o->t == p->t || o->out) continue;
    float a = o->x * d;
    if (a > a1) { a2 = a1; a1 = a; } else if (a > a2) a2 = a;
  }
  return qa > a2 + 3;
}

static void callOffside(Player *q) {
  Ball *b = &M.ball;
  dbgOffside++;
  M.state = ST_FOUL; M.st = 90;
  M.foulT = (q->t == 1) ? 2 : 1;
  M.foulX = clampf(q->x, 10, FL - 10); M.foulY = clampf(q->y, 10, FW - 10);
  M.foulPen = 0; M.foulKind = 1;
  b->owner = NULL; b->vx = b->vy = b->vz = 0; b->z = 0; b->x = M.foulX; b->y = M.foulY;
  snprintf(M.msg, sizeof M.msg, "IMPEDIMENTO!");
  M.offQ = NULL;
  playSnd(S_WHISTLE);
}

void matchPenaltyDone(int att, int goal) {
  SCENE = SC_MATCH;
  M.paused = 0;
  if (goal) { onGoal(att); }
  else { M.state = ST_PLAY; M.freeze = 40; giveGoalKick((att == 1) ? 2 : 1); }
}

/* ---------------- IA ---------------- */
typedef struct { float x, y, sf; } Tgt;

static float diffMul(const Player *p) {
  static const float dm[3] = { 0.84f, 0.94f, 1.03f };
  float m = 1.0f;
  if (p->t == 2) m = dm[clampi(OPT.diff, 1, 3) - 1];
  return m * (0.94f + 0.03f * TEAMS[M.team[p->t]].str);
}

static Tgt aiWithBall(Player *p) {
  const Tac *tc = &M.tac[p->t];
  int dir = M.dir[p->t];
  float ax = atkX(p->t), gyc = goalCY(), rs = M.rs;
  p->hold++;
  if (p->gk) {
    if (p->hold > 55) {
      float sc; Player *q = pickMateAI(p, &sc);
      doPassTo(p, q, prob() < 0.45f, 0);
    }
    return (Tgt){ p->x, p->y, 0 };
  }
  float dg = dist(p->x, p->y, ax, gyc);
  float range = (M.futsal ? 250.0f : 290.0f) * SCL * (0.9f + 0.12f * tc->ment);
  int blk = countBlockers(p->x, p->y, ax, gyc, p->t, 22 * rs);
  float shootP = (0.05f + 0.012f * TEAMS[M.team[p->t]].str) * ((tc->ment == 0) ? 0.8f : ((tc->ment == 2) ? 1.3f : 1.0f));
  shootP *= (blk == 0) ? 1.6f : 0.5f;
  if (dg < range && p->hold > 8 && prob() < shootP) {
    float gm = (GOAL_Y2 - GOAL_Y1) * rndf(-0.40f, 0.40f);
    float ay = (gyc + gm) - p->y, axv = ax - p->x, nx, ny;
    normf(axv, ay, &nx, &ny);
    M.humanShot = 0;
    doShoot(p, rndf(0.6f, 1.0f), 1, nx, ny);
    return (Tgt){ p->x, p->y, 0 };
  }
  Player *no = NULL;
  float nd = nearestOpp(p->x, p->y, p->t, &no);
  float sc = -9; Player *q = pickMateAI(p, &sc);
  if (q) {
    int press = (nd < 24 * rs && prob() < 0.10f);
    int better = (sc > 1.3f && prob() < 0.02f + 0.01f * tc->ment);
    int stale = (p->hold > 150 && prob() < 0.05f);
    if (press || better || stale) { doPassTo(p, q, (sc > 1.0f && prob() < 0.2f) ? 1 : 0, 0); return (Tgt){ p->x, p->y, 0 }; }
  }
  float ty = gyc + sinf(M.hf / 45.0f + p->i) * 70 * SCL;
  if (no && nd < 70 * rs && (no->x - p->x) * dir > 0) ty = (no->y > p->y) ? p->y - 70 * SCL : p->y + 70 * SCL;
  return (Tgt){ ax, clampf(ty, 20, FW - 20), 1 };
}

static Tgt aiPlayer(Player *p) {
  Ball *b = &M.ball;
  int ti = p->t, dir = M.dir[ti];
  const Tac *tc = &M.tac[ti];
  float hx, hy; homePos(p, &hx, &hy);
  float own = ownX(ti), rs = M.rs, gyc = goalCY();
  if (p->gk) {
    if (b->owner == p) return aiWithBall(p);
    float mouth = GOAL_Y2 - GOAL_Y1;
    float depth = 10 * sqrtf(SCL) + clampf(fabsf(b->x - own) * 0.09f, 0, 26 * SCL);
    float ty = gyc + clampf((b->y - gyc) * 0.32f, -mouth * 0.42f, mouth * 0.42f);
    if (!b->owner && fabsf(b->x - own) < 130 * SCL && b->z < 20 && fabsf(b->vx) < 3.5f * M.bsp && dist(p->x, p->y, b->x, b->y) < 85 * SCL)
      return (Tgt){ b->x, b->y, 1 };
    if (b->owner && b->owner->t != ti && fabsf(b->owner->x - own) < 150 * SCL && dist(p->x, p->y, b->owner->x, b->owner->y) < 100 * SCL)
      return (Tgt){ own + dir * (depth + 30 * SCL), ty, 1 };
    return (Tgt){ own + dir * depth, ty, 1 };
  }
  if (b->owner == p) return aiWithBall(p);
  Player *o = b->owner;
  const float *f = M.form[ti][p->i - 1];
  int rank = 0; float dp = dist(p->x, p->y, b->x, b->y);
  for (int k = 0; k < M.np; k++) {
    Player *q = &M.pl[k];
    if (q->t != ti || q->gk || q->out || q == p) continue;
    if (ti == 1 && q == M.ctrl) continue;
    if (dist(q->x, q->y, b->x, b->y) < dp) rank++;
  }
  if (o && o->t == ti) {
    float reach = (f[0] > 0.5f) ? 100.0f : ((f[0] > 0.34f) ? 60.0f : 25.0f);
    reach *= (0.8f + 0.25f * tc->ment) * SCL;
    float x = clampf(hx + dir * reach, 40, FL - 40), y = hy;
    Player *op = NULL;
    float d = nearestOpp(x, y, ti, &op);
    if (op && d < 80 * rs) y += ((hy >= op->y) ? 1 : -1) * 35 * SCL;
    return (Tgt){ x, clampf(y, 30, FW - 30), 1 };
  }
  int npress = 1 + tc->press;
  if (o) {
    if (rank < npress) {
      float d = dist(p->x, p->y, o->x, o->y);
      if (rank == 0 && d < 14 * rs && p->tack <= 0 && prob() < 0.03f + 0.012f * TEAMS[M.team[ti]].str) attemptTackle(p, o);
      if (rank == 0) return (Tgt){ o->x + o->vx * 6, o->y + o->vy * 6, 1 };
      return (Tgt){ o->x + (own - o->x) * 0.2f, o->y + (hy - o->y) * 0.2f, 1 };
    }
    if (tc->mark == 1) {
      Player *tgt = NULL; float bd = 1e9f;
      for (int k = 0; k < M.np; k++) {
        Player *q = &M.pl[k];
        if (q->t == ti || q->gk || q->out) continue;
        float dd = dist(q->x, q->y, hx, hy);
        if (dd < bd && dd < 220 * SCL) { bd = dd; tgt = q; }
      }
      if (tgt) return (Tgt){ tgt->x - dir * 18 * rs, tgt->y, 0.95f };
    }
    float k = 0.30f + 0.12f * tc->press;
    return (Tgt){ hx * (1 - k) + b->x * k, hy * 0.7f + b->y * 0.3f, 0.9f };
  }
  if (rank < 1 || (rank < 2 && dp < 150 * SCL)) return (Tgt){ b->x + b->vx * 4, b->y + b->vy * 4, 1 };
  return (Tgt){ hx * 0.55f + b->x * 0.45f, hy * 0.7f + b->y * 0.3f, 0.9f };
}

/* ---------------- movimento ---------------- */
static void movePlayer(Player *p, float tx, float ty, float sf) {
  float stam = 0.82f + 0.18f * p->sta;
  float sp = 1.5f * M.spd * sf * diffMul(p) * clampf(p->pace, 0.85f, 1.15f) * stam;
  if (p->stun > 0) sp = 0;
  float dx, dy, d = normf(tx - p->x, ty - p->y, &dx, &dy);
  if (d < 3) { dx = 0; dy = 0; }
  float lim = fminf(1.0f, d / 14.0f);
  float wvx = dx * sp * lim, wvy = dy * sp * lim;
  if (p->tack > 28) { wvx = p->fx * 2.5f * M.spd; wvy = p->fy * 2.5f * M.spd; }
  p->vx += (wvx - p->vx) * 0.28f;
  p->vy += (wvy - p->vy) * 0.28f;
  p->x += p->vx; p->y += p->vy;
  if (fabsf(p->vx) + fabsf(p->vy) > 0.25f * M.spd) {
    p->moving = 1; p->anim++;
    float fx, fy; normf(p->vx, p->vy, &fx, &fy);
    if (fx != 0 || fy != 0) { p->fx = fx; p->fy = fy; }
  } else p->moving = 0;
  if (sf > 1.05f) p->sta = fmaxf(0, p->sta - 0.0011f * sf);
  else if (sf < 0.3f) p->sta = fminf(1, p->sta + 0.0010f);
  else p->sta = fmaxf(0, p->sta - 0.00025f);
  p->x = clampf(p->x, 4, FL - 4);
  p->y = clampf(p->y, 4, FW - 4);
}

static Tgt humanControl(Player *p) {
  Ball *b = &M.ball;
  float dx, dy; dirInput(&dx, &dy);
  float sf = 1.0f;
  if (held(B_R)) sf = 1.35f;
  int has = (b->owner == p);
  if (p->stun > 0 || M.state != ST_PLAY) { dx = 0; dy = 0; }
  float fx = p->fx, fy = p->fy;
  if (dx != 0 || dy != 0) normf(dx, dy, &fx, &fy);
  if (has) {
    if (hit(B_CROSS)) { doPass(p, 0, fx, fy, 1); M.charge = 0; }
    else if (hit(B_SQUARE)) { doPass(p, 1, fx, fy, 1); M.charge = 0; }
    if (b->owner == p) {
      if (held(B_CIRCLE)) M.charge = fminf(1.0f, M.charge + 1.0f / 40.0f);
      else if (M.charge > 0) {
        M.humanShot = 1;
        doShoot(p, fmaxf(0.25f, M.charge), 1, fx, fy);
        M.charge = 0;
      }
    }
  } else {
    M.charge = 0;
    if (hit(B_CROSS)) { Player *n = nearestField(1, b->x, b->y, p); if (n) M.ctrl = n; }
    if (hit(B_CIRCLE) && p->tack <= 0 && p->stun <= 0) p->tack = 36;
    if (p->tack > 28 && b->owner && b->owner->t != p->t && dist(p->x, p->y, b->owner->x, b->owner->y) < 15 * M.rs) {
      attemptTackle(p, b->owner); p->tack = 26;
    }
  }
  return (Tgt){ p->x + dx * 40, p->y + dy * 40, (dx != 0 || dy != 0) ? sf : 0 };
}

/* ---------------- bola ---------------- */
static void setPiece(int ti, float x, float y) {
  Ball *b = &M.ball;
  Player *q = nearestField(ti, x, y, NULL);
  if (!q) q = &M.pl[0];
  b->x = x; b->y = y; b->z = b->vx = b->vy = b->vz = 0;
  q->x = x - M.dir[ti] * 4; q->y = y;
  if (y < 8) q->y = y + 6; else if (y > FW - 8) q->y = y - 6;
  b->owner = q; b->last = ti; b->lastp = q; q->hold = 0; q->kick = 0;
  for (int k = 0; k < M.np; k++) { M.pl[k].vx = 0; M.pl[k].vy = 0; }
  M.freeze = 28; q->hold = 110;
  int corner = ((x <= 10 || x >= FL - 10) && (y <= 10 || y >= FW - 10));
  if (corner) {
    float gx = (x <= 10) ? 0.0f : FL, inw = (x <= 10) ? 1.0f : -1.0f;
    static const float sp[4][2] = { {70, -60}, {110, 0}, {60, 55}, {150, -95} };
    int usedA[MAXP] = { 0 }, usedD[MAXP] = { 0 };
    for (int s = 0; s < 4; s++) {
      Player *a = NULL, *d = NULL; float ba = -1e9f, bd = 1e9f;
      for (int k = 0; k < M.np; k++) {
        Player *p = &M.pl[k];
        if (p->gk || p->out || p == q) continue;
        float rxv = M.form[p->t][p->i - 1][0];
        if (p->t == ti && !usedA[k] && rxv > ba) { ba = rxv; a = p; }
        if (p->t != ti && !usedD[k] && rxv < bd) { bd = rxv; d = p; }
      }
      float sx = gx + inw * sp[s][0] * SCL, sy = FW / 2 + sp[s][1] * SCL;
      if (a) { a->x = sx; a->y = sy; usedA[a - M.pl] = 1; }
      if (d) { d->x = sx + inw * 12; d->y = sy + 14; usedD[d - M.pl] = 1; }
    }
    M.freeze = 75;
  }
  if (ti == 1 && !q->gk) M.ctrl = q;
}

static void onGoal(int ti) {
  M.score[ti]++; M.state = ST_GOAL; M.st = 170; M.scorer = ti;
  Player *lp = M.ball.lastp;
  if (lp && lp->t == ti && ti == 1 && M.mgr && lp->roster >= 0) mgrCredit(lp->roster);
  if (M.gnc[ti] < 16) M.gn[ti][M.gnc[ti]++] = (lp && lp->t == ti) ? lp->nameIdx : rand() % 64;
  playSnd(S_GOAL); playSnd(S_CHEER);
}

static void updateBall(void) {
  Ball *b = &M.ball;
  if (b->owner) {
    Player *p = b->owner;
    b->x = p->x + p->fx * 6; b->y = p->y + p->fy * 6 + 1;
    b->z = 0; b->vx = p->vx; b->vy = p->vy; b->vz = 0;
    return;
  }
  b->x += b->vx; b->y += b->vy;
  b->z += b->vz; b->vz -= GRAV;
  if (b->z <= 0) {
    b->z = 0;
    if (b->vz < -1.2f) b->vz = -b->vz * 0.5f; else b->vz = 0;
    b->vx *= 0.985f; b->vy *= 0.985f;
  } else { b->vx *= 0.997f; b->vy *= 0.997f; }
  if (fabsf(b->vx) < 0.02f) b->vx = 0;
  if (fabsf(b->vy) < 0.02f) b->vy = 0;
  if (M.state != ST_PLAY) return;
  if (b->y < 0 || b->y > FW) {
    if (M.futsal) { b->y = clampf(b->y, 0, FW); b->vy = -b->vy * 0.8f; }
    else {
      int ti = (b->last == 1) ? 2 : 1;
      setPiece(ti, clampf(b->x, 12, FL - 12), (b->y < 0) ? 3 : FW - 3);
      return;
    }
  }
  if (b->x < 0 || b->x > FL) {
    int left = (b->x < 0);
    if (b->y > GOAL_Y1 && b->y < GOAL_Y2 && b->z < GOAL_H) {
      int scorer = (M.dir[1] == (left ? -1 : 1)) ? 1 : 2;
      onGoal(scorer);
      return;
    }
    if (M.futsal) {
      b->x = clampf(b->x, 0, FL); b->vx = -b->vx * 0.75f;
      if (b->y > GOAL_Y1 - 8 && b->y < GOAL_Y2 + 8) playSnd(S_POST);
    } else {
      int defT = left ? ((M.dir[1] == 1) ? 1 : 2) : ((M.dir[1] == 1) ? 2 : 1);
      if (b->last == defT) {
        int attT = (defT == 1) ? 2 : 1;
        setPiece(attT, left ? 6 : FL - 6, (b->y < FW / 2) ? 6 : FW - 6);
      } else giveGoalKick(defT);
    }
  }
}

static void updatePickup(void) {
  Ball *b = &M.ball;
  if (b->owner || M.state != ST_PLAY || M.freeze > 0) return;
  Player *best = NULL; float bd = 1e9f;
  for (int k = 0; k < M.np; k++) {
    Player *p = &M.pl[k];
    float reach = (p->gk ? 19.0f * (1.0f + 0.9f * (SCL - 1.0f)) : 9.0f * M.rs), hz = p->gk ? 34.0f : 13.0f;
    if (!p->out && p->kick <= 0 && p->stun <= 0 && b->z <= hz) {
      float d = dist(p->x, p->y, b->x, b->y);
      if (d < reach && d < bd) { best = p; bd = d; }
    }
  }
  if (!best) return;
  if (M.offQ && best == M.offQ && M.offT > 0 && !M.futsal) { callOffside(best); return; }
  M.offQ = NULL;
  float sp = sqrtf(b->vx * b->vx + b->vy * b->vy);
  if (best->gk) {
    static const float ccs[3] = { 0.40f, 0.52f, 0.64f };
    float cc = ccs[clampi(OPT.diff, 1, 3) - 1];
    if (best->t == 1) cc = 0.62f;
    cc += TEAMS[M.team[best->t]].str * 0.02f;
    if (sp > 6 * M.bsp) best->da = 24;
    if (sp > 7 * M.bsp && prob() > cc) {
      b->vx = -b->vx * 0.45f + rndf(-1.2f, 1.2f);
      b->vy = b->vy * 0.5f + rndf(-3, 3);
      b->vz = rndf(0.6f, 1.8f);
      best->kick = 25; b->last = best->t; b->lastp = best;
      playSnd(S_POST);
      return;
    }
  } else if (sp > 11 * M.bsp) {
    b->vx *= 0.5f; b->vy *= 0.5f; best->kick = 12;
    return;
  }
  b->owner = best; b->last = best->t; b->lastp = best; best->hold = 0;
  b->vx = b->vy = b->vz = 0;
  if (best->t == 1 && !best->gk && !M.training) M.ctrl = best;
}

static void separatePlayers(void) {
  float rad = 9 * M.rs;
  for (int i = 0; i < M.np - 1; i++) {
    Player *a = &M.pl[i];
    if (a->out) continue;
    for (int j = i + 1; j < M.np; j++) {
      Player *c = &M.pl[j];
      if (c->out) continue;
      float dx = c->x - a->x, dy = c->y - a->y;
      float d = sqrtf(dx * dx + dy * dy);
      if (d < rad && d > 0.01f) {
        float push = (rad - d) * 0.5f, ux = dx / d, uy = dy / d;
        a->x -= ux * push; a->y -= uy * push;
        c->x += ux * push; c->y += uy * push;
      }
    }
  }
}

static void finishMatch(void) {
  RES.s1 = M.score[1]; RES.s2 = M.score[2];
  RES.t1 = M.team[1]; RES.t2 = M.team[2]; RES.mode = M.mode;
  RES.shots1 = M.shots[1]; RES.shots2 = M.shots[2];
  RES.chal = M.chal; RES.minute = (int)(M.hf / (float)M.halfFrames * 45) + ((M.half == 2) ? 45 : 0); RES.stars = 0;
  if (M.chal >= 0) RES.stars = chalStars(M.chal, M.score[1], M.score[2], RES.minute);
  { int tp = M.poss[1] + M.poss[2]; RES.poss1 = tp ? (100 * M.poss[1] / tp) : 50; RES.poss2 = 100 - RES.poss1; }
  SCENE = SC_RESULT;
}

static void updateCamera(void) {
  if (!M.big) return;
  Ball *b = &M.ball;
  float tx = b->x + b->vx * 14, ty = b->y + b->vy * 8;
  M.cx += (tx - M.cx) * 0.10f;
  M.cy += (ty - M.cy) * 0.10f;
  M.cx = clampf(M.cx, 180, FL - 180);
  M.cy = clampf(M.cy, 100, FW - 100);
}

void updateMatch(void) {
  Ball *b = &M.ball;
  if (hit(B_START) && (M.state == ST_PLAY || M.state == ST_KICK)) { M.paused = 1; M.pmenu = 0; M.pkind = PM_MAIN; return; }
  for (int k = 0; k < M.np; k++) {
    Player *p = &M.pl[k];
    if (p->kick > 0) p->kick--;
    if (p->stun > 0) p->stun--;
    if (p->tack > 0) p->tack--;
    if (p->ka > 0) p->ka--;
    if (p->da > 0) p->da--;
  }
  if (M.freeze > 0) M.freeze--;

  if (M.state == ST_KICK) {
    if (--M.st == 40) playSnd(S_WHISTLE);
    if (M.st <= 0) M.state = ST_PLAY;
    if (M.ctrl) humanControl(M.ctrl);
    updateBall(); updateCamera();
    return;
  } else if (M.state == ST_REPLAY) {
    replayStep();
    return;
  } else if (M.state == ST_GOAL) {
    M.st--;
    if (M.st == 100 && OPT.replay && replayAvailable() && !M.training) { replayStart(); return; }
    updateBall();
    b->x = clampf(b->x, -30, FL + 30);
    for (int k = 0; k < M.np; k++) { M.pl[k].vx *= 0.8f; M.pl[k].vy *= 0.8f; M.pl[k].moving = 0; }
    if (M.st <= 0) {
      int end = (M.rule == 1) || (M.rule == 2 && (M.score[1] >= 3 || M.score[2] >= 3));
      if (end && !M.training) { M.state = ST_FULL; return; }
      startKickoff(M.training ? 1 : ((M.scorer == 1) ? 2 : 1));
    }
    updateCamera();
    return;
  } else if (M.state == ST_FOUL) {
    b->vx = b->vy = b->vz = 0;
    for (int k = 0; k < M.np; k++) { M.pl[k].vx *= 0.7f; M.pl[k].vy *= 0.7f; M.pl[k].moving = 0; }
    if (--M.st <= 0) {
      M.state = ST_PLAY;
      if (M.foulPen) startMatchPenalty(M.foulT);
      else setPiece(M.foulT, M.foulX, M.foulY);
    }
    updateCamera();
    return;
  } else if (M.state == ST_HALF) {
    if (--M.st <= 0 || hit(B_CROSS)) {
      M.half = 2; M.hf = 0; M.dir[1] = -1; M.dir[2] = 1;
      for (int k = 0; k < M.np; k++) M.pl[k].sta = fminf(1.0f, M.pl[k].sta + 0.35f);
      startKickoff(2);
    }
    return;
  } else if (M.state == ST_FULL) {
    if (hit(B_CROSS) || hit(B_START)) finishMatch();
    return;
  }

  if (b->owner) M.poss[b->owner->t]++;
  if (!M.training) {
    M.hf++;
    if (M.hf >= M.halfFrames) {
      playSnd(S_WHISTLE);
      if (M.half == 1 && !M.single) { M.state = ST_HALF; M.st = 240; } else M.state = ST_FULL;
      return;
    }
  }
  if (dbgAllAI) M.ctrl = NULL;
  else if (!M.ctrl || M.ctrl->gk || M.ctrl->out) {
    if (M.training) { for (int k = 0; k < M.np; k++) if (M.pl[k].t == 1) M.ctrl = &M.pl[k]; }
    else M.ctrl = nearestField(1, b->x, b->y, NULL);
  }
  for (int k = 0; k < M.np; k++) {
    Player *p = &M.pl[k];
    if (p->out) continue;
    Tgt t = (p == M.ctrl) ? humanControl(p) : aiPlayer(p);
    if (M.freeze > 0) t.sf = 0;
    movePlayer(p, t.x, t.y, t.sf);
  }
  separatePlayers();
  updatePickup();
  updateBall();
  updateCamera();
  if (M.offT > 0) { M.offT--; if (M.offT == 0) M.offQ = NULL; }
  replayRecord();
}
