/* Super Pixel Soccer PSP - nucleo: utilitarios e motor da partida */
#include "game.h"

Match M;
Opt OPT = { 2, 2, 1 };
int SCENE = SC_TITLE, TICK = 0, NCLUBS = 8;
int T_ATLAS = -1, T_BG_FIELD = -1, T_BG_COURT = -1, T_TITLE = -1, T_FONT = -1;
int S_KICK = -1, S_WHISTLE = -1, S_GOAL = -1, S_POST = -1;
Result RES;

Team TEAMS[9] = {
  { "TRICOLOR",  "TRI", KIT_C1, {0}, 0, 4, { 150, 30, 60, 255 } },
  { "VERDAO",    "VRD", KIT_C2, {0}, 0, 5, { 20, 130, 60, 255 } },
  { "CRUZ MALT", "CMA", KIT_C3, {0}, 0, 3, { 40, 40, 40, 255 } },
  { "PEIXE",     "PXE", KIT_C4, {0}, 0, 4, { 230, 230, 230, 255 } },
  { "MACACA",    "MAC", KIT_C5, {0}, 0, 2, { 220, 220, 220, 255 } },
  { "ALVINEGRO", "ALV", KIT_C6, {0}, 0, 3, { 90, 90, 90, 255 } },
  { "LEAO",      "LEA", KIT_C7, {0}, 0, 3, { 200, 30, 30, 255 } },
  { "ESMERALDA", "ESM", KIT_C8, {0}, 0, 2, { 30, 150, 80, 255 } },
  { "LENDAS",    "LEN", KIT_C4, { KIT_PELE, KIT_MESSI, KIT_NEYMAR, KIT_CR7, KIT_MARA }, 5, 5, { 255, 215, 60, 255 } }
};

static const float FORM6[6][2] = { {0.04f,0.5f}, {0.22f,0.24f}, {0.22f,0.76f}, {0.45f,0.5f}, {0.68f,0.3f}, {0.68f,0.7f} };
static const float FORM5[5][2] = { {0.04f,0.5f}, {0.24f,0.3f}, {0.24f,0.7f}, {0.6f,0.3f}, {0.6f,0.7f} };

/* ---------------- utilitarios ---------------- */
static int cur[12], prv[12];
static int AX, AY;

void readPad(void) {
  memcpy(prv, cur, sizeof(cur));
  plat_input(cur, &AX, &AY);
}
int hit(int b) { return cur[b] && !prv[b]; }
int held(int b) { return cur[b]; }

int clampi(int v, int a, int b) { return v < a ? a : (v > b ? b : v); }
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
void playSnd(int id) { if (OPT.sound && id >= 0) plat_snd_play(id); }

void saveOpt(void) {
  FILE *f = fopen("save.txt", "w");
  if (f) { fprintf(f, "%d %d %d", OPT.diff, OPT.half, OPT.sound); fclose(f); }
}
void loadOpt(void) {
  FILE *f = fopen("save.txt", "r");
  if (f) {
    int a, b, c;
    if (fscanf(f, "%d %d %d", &a, &b, &c) == 3 && a >= 1 && a <= 3 && b >= 1 && b <= 3) { OPT.diff = a; OPT.half = b; OPT.sound = c; }
    fclose(f);
  }
}

/* campo logico -> tela */
void toScreen(float x, float y, float *sx, float *sy) {
  float u = x / FL, v = y / FW;
  float f = (v < 0) ? v : powf(v, 1.14f);
  float l = 58 - 50 * f, r = 422 + 50 * f;
  *sx = l + (r - l) * u;
  *sy = 70 + 174 * f;
}

static float atkX(int ti) { return M.dir[ti] == 1 ? FL : 0; }
static float ownX(int ti) { return M.dir[ti] == 1 ? 0 : FL; }

static void homePos(const Player *p, float *hx, float *hy) {
  const float *f = M.form[p->i - 1];
  if (M.dir[p->t] == 1) { *hx = f[0] * FL; *hy = f[1] * FW; }
  else { *hx = (1 - f[0]) * FL; *hy = f[1] * FW; }
}

static int teamKit(int ti, int n) {
  const Team *tm = &TEAMS[M.team[ti]];
  if (n == 1) return KIT_GK;
  if (tm->nkits > 0) return tm->kits[(n - 2) % tm->nkits];
  return tm->kit;
}

static Player *nearestField(int ti, float x, float y, const Player *skip) {
  Player *best = NULL; float bd = 1e9f;
  for (int k = 0; k < M.np; k++) {
    Player *p = &M.pl[k];
    if (p->t == ti && !p->gk && p != skip) {
      float d = dist(p->x, p->y, x, y);
      if (d < bd) { best = p; bd = d; }
    }
  }
  return best;
}

static void startKickoff(int ti);

void newMatch(int t1, int t2, int futsal, int training, int trainKit, int mode) {
  memset(&M, 0, sizeof(M));
  M.team[1] = t1; M.team[2] = t2; M.mode = mode;
  M.futsal = futsal; M.training = training; M.trainKit = trainKit;
  M.half = 1; M.dir[1] = 1; M.dir[2] = -1; M.active = 1;
  static const int secs[3] = { 45, 90, 150 };
  M.halfFrames = secs[clampi(OPT.half, 1, 3) - 1] * 60;
  if (futsal) { M.form = FORM5; M.nform = 5; } else { M.form = FORM6; M.nform = 6; }
  M.ball.x = FL / 2; M.ball.y = FW / 2; M.ball.last = 1;
  for (int ti = 1; ti <= 2; ti++) {
    int list[6], n = 0;
    if (training) {
      if (ti == 1) { list[n++] = 4; } else { list[n++] = 1; list[n++] = 2; list[n++] = 3; }
    } else {
      for (int k = 1; k <= M.nform; k++) list[n++] = k;
    }
    for (int k = 0; k < n; k++) {
      Player *p = &M.pl[M.np++];
      memset(p, 0, sizeof(*p));
      p->t = ti; p->i = list[k]; p->gk = (list[k] == 1);
      p->fx = (ti == 1) ? 1.0f : -1.0f;
      p->kit = teamKit(ti, list[k]);
      if (training && ti == 1) p->kit = trainKit;
    }
  }
  startKickoff(1);
}

static void startKickoff(int ti) {
  M.kickTeam = ti; M.state = ST_KICK; M.st = 80;
  Ball *b = &M.ball;
  b->x = FL / 2; b->y = FW / 2; b->z = b->vx = b->vy = b->vz = 0; b->owner = NULL;
  Player *kicker = NULL;
  for (int k = 0; k < M.np; k++) {
    Player *p = &M.pl[k];
    float hx, hy; homePos(p, &hx, &hy);
    int d = M.dir[p->t];
    if (M.training) { p->x = hx; p->y = hy; }
    else { p->x = FL / 2 + (hx - FL / 2) * 0.7f; p->y = hy; }
    p->vx = p->vy = 0; p->stun = p->tack = p->kick = p->hold = 0;
    p->fx = (float)d; p->fy = 0;
    if (p->t == ti && p->i == 4) kicker = p;
  }
  if (kicker) {
    kicker->x = FL / 2 - M.dir[ti] * 4; kicker->y = FW / 2;
    b->owner = kicker; b->last = ti;
  }
  if (M.training) {
    Player *h = NULL;
    for (int k = 0; k < M.np; k++) if (M.pl[k].t == 1) h = &M.pl[k];
    M.ctrl = h;
    if (h) { h->x = 300; h->y = FW / 2; b->x = 310; b->y = FW / 2; b->owner = NULL; }
  } else {
    M.ctrl = nearestField(1, b->x, b->y, NULL);
  }
}

/* ---------------- chutes e passes ---------------- */
static void releaseBall(Player *p) {
  Ball *b = &M.ball;
  b->owner = NULL; b->last = p->t;
  p->kick = 22; p->hold = 0;
  b->x = p->x + p->fx * 8; b->y = p->y + p->fy * 8;
}

static void doShoot(Player *p, float power, int hasAim, float ax, float ay) {
  Ball *b = &M.ball;
  releaseBall(p);
  float dx, dy;
  normf(atkX(p->t) - p->x, FW / 2 - p->y, &dx, &dy);
  if (hasAim) {
    float mx = ax * 0.7f + dx * 0.3f, my = ay * 0.7f + dy * 0.3f;
    normf(mx, my, &dx, &dy);
  }
  float err = (1.15f - power) * 0.10f + 0.015f;
  if (p->t == 2 || !M.humanShot) err += (3 - OPT.diff) * 0.012f;
  float ang = atan2f(dy, dx) + rndf(-err, err);
  float sp = 6.5f + power * 8;
  b->vx = cosf(ang) * sp; b->vy = sinf(ang) * sp;
  b->vz = 0.7f + power * 1.7f + rndf(0, 0.5f);
  M.shots[p->t]++;
  playSnd(S_KICK);
}

static Player *pickMate(Player *p, float fxv, float fyv) {
  Player *best = NULL; float bs = -1e9f;
  for (int k = 0; k < M.np; k++) {
    Player *q = &M.pl[k];
    if (q->t == p->t && q != p) {
      float dx, dy, d = normf(q->x - p->x, q->y - p->y, &dx, &dy);
      if (d > 20) {
        float dot = dx * fxv + dy * fyv;
        float s = dot - d / 700.0f;
        if (q->gk) s -= 0.6f;
        if (dot > 0.25f && s > bs) { best = q; bs = s; }
      }
    }
  }
  if (!best) {
    float bd = 1e9f;
    for (int k = 0; k < M.np; k++) {
      Player *q = &M.pl[k];
      if (q->t == p->t && q != p && !q->gk) {
        float d = dist(p->x, p->y, q->x, q->y);
        if (d < bd) { best = q; bd = d; }
      }
    }
  }
  return best;
}

static void doPass(Player *p, int lob, float fxv, float fyv, int human) {
  Ball *b = &M.ball;
  Player *q = pickMate(p, fxv, fyv);
  releaseBall(p);
  if (!q) { b->vx = p->fx * 6; b->vy = p->fy * 6; playSnd(S_KICK); return; }
  float d = dist(p->x, p->y, q->x, q->y);
  float tx = q->x + q->vx * 8, ty = q->y + q->vy * 8;
  float dx, dy; normf(tx - p->x, ty - p->y, &dx, &dy);
  if (lob) {
    float vz = clampf(d * 0.008f + 1.8f, 2.0f, 4.6f);
    float T = 2 * vz / GRAV;
    float sp = clampf(d / T, 2, 11);
    b->vx = dx * sp; b->vy = dy * sp; b->vz = vz;
  } else {
    float sp = clampf(3.6f + d * 0.012f, 4, 10);
    b->vx = dx * sp; b->vy = dy * sp; b->vz = 0;
  }
  playSnd(S_KICK);
  if (human && p->t == 1 && !q->gk) M.ctrl = q;
}

static int attemptTackle(Player *p, Player *o) {
  Ball *b = &M.ball;
  p->tack = 40;
  float chance = 0.42f + (TEAMS[M.team[p->t]].str - TEAMS[M.team[o->t]].str) * 0.04f;
  if (p->t == 2) chance += (OPT.diff - 2) * 0.05f;
  if (prob() < chance) {
    o->stun = 30; b->owner = p; b->last = p->t; p->hold = 0;
    if (p->t == 1 && !p->gk) M.ctrl = p;
    return 1;
  }
  p->stun = 22;
  return 0;
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
  int dir = M.dir[p->t];
  float ax = atkX(p->t);
  p->hold++;
  float dg = dist(p->x, p->y, ax, FW / 2);
  if (p->gk) {
    if (p->hold > 55) doPass(p, prob() < 0.4f, (float)dir, rndf(-0.4f, 0.4f), 0);
    return (Tgt){ p->x, p->y, 0 };
  }
  float range = M.futsal ? 250.0f : 290.0f;
  if (dg < range && p->hold > 8 && prob() < 0.045f + 0.01f * TEAMS[M.team[p->t]].str) {
    M.humanShot = 0;
    doShoot(p, rndf(0.6f, 1.0f), 0, 0, 0);
    return (Tgt){ p->x, p->y, 0 };
  }
  float nd = 1e9f; Player *no = NULL;
  for (int k = 0; k < M.np; k++) {
    Player *o = &M.pl[k];
    if (o->t != p->t) { float d = dist(p->x, p->y, o->x, o->y); if (d < nd) { nd = d; no = o; } }
  }
  if (nd < 22 && prob() < 0.05f) {
    doPass(p, prob() < 0.25f, p->fx, p->fy, 0);
    return (Tgt){ p->x, p->y, 0 };
  }
  float ty = FW / 2 + sinf(M.hf / 45.0f + p->i) * 70;
  if (no && nd < 60 && (no->x - p->x) * dir > 0) ty = (no->y > p->y) ? p->y - 70 : p->y + 70;
  return (Tgt){ ax, clampf(ty, 20, FW - 20), 1 };
}

static Tgt aiPlayer(Player *p) {
  Ball *b = &M.ball;
  int ti = p->t, dir = M.dir[ti];
  float hx, hy; homePos(p, &hx, &hy);
  float own = ownX(ti);
  if (p->gk) {
    if (b->owner == p) return aiWithBall(p);
    float tx = own + dir * (10 + clampf(fabsf(b->x - own) * 0.1f, 0, 28));
    float ty = FW / 2 + (b->y - FW / 2) * 0.42f;
    if (!b->owner && fabsf(b->x - own) < 120 && b->z < 20 && fabsf(b->vx) < 3.5f && dist(p->x, p->y, b->x, b->y) < 80)
      return (Tgt){ b->x, b->y, 1 };
    return (Tgt){ tx, clampf(ty, 165, 235), 1 };
  }
  if (b->owner == p) return aiWithBall(p);
  Player *o = b->owner;
  Player *ch = nearestField(ti, b->x, b->y, (ti == 1) ? M.ctrl : NULL);
  if (o && o->t == ti) return (Tgt){ clampf(hx + dir * 90, 40, FL - 40), hy, 1 };
  if (ch == p) {
    if (o) {
      float d = dist(p->x, p->y, o->x, o->y);
      if (d < 14 && p->tack <= 0 && prob() < 0.03f + 0.012f * TEAMS[M.team[ti]].str) attemptTackle(p, o);
      return (Tgt){ o->x + o->vx * 6, o->y + o->vy * 6, 1 };
    }
    return (Tgt){ b->x + b->vx * 4, b->y + b->vy * 4, 1 };
  }
  if (!o) {
    float d1 = dist(p->x, p->y, b->x, b->y);
    if (d1 < 130) return (Tgt){ b->x, b->y, 0.95f };
  }
  return (Tgt){ hx * 0.55f + b->x * 0.45f, hy * 0.7f + b->y * 0.3f, 0.9f };
}

/* ---------------- movimento ---------------- */
static void movePlayer(Player *p, float tx, float ty, float sf) {
  float sp = 1.5f * sf * diffMul(p);
  if (p->stun > 0) sp = 0;
  float dx, dy, d = normf(tx - p->x, ty - p->y, &dx, &dy);
  if (d < 3) { dx = 0; dy = 0; }
  float lim = fminf(1.0f, d / 14.0f);
  float wvx = dx * sp * lim, wvy = dy * sp * lim;
  if (p->tack > 28) { wvx = p->fx * 2.5f; wvy = p->fy * 2.5f; }
  p->vx += (wvx - p->vx) * 0.28f;
  p->vy += (wvy - p->vy) * 0.28f;
  p->x += p->vx; p->y += p->vy;
  if (fabsf(p->vx) + fabsf(p->vy) > 0.25f) {
    p->moving = 1; p->anim++;
    float fx, fy; normf(p->vx, p->vy, &fx, &fy);
    if (fx != 0 || fy != 0) { p->fx = fx; p->fy = fy; }
  } else p->moving = 0;
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
    if (p->tack > 28 && b->owner && b->owner->t != p->t && dist(p->x, p->y, b->owner->x, b->owner->y) < 15) {
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
  b->owner = q; b->last = ti; q->hold = 0; q->kick = 0;
  for (int k = 0; k < M.np; k++) { M.pl[k].vx = 0; M.pl[k].vy = 0; }
  M.freeze = 28;
  if (ti == 1 && !q->gk) M.ctrl = q;
}

static void onGoal(int ti) {
  M.score[ti]++; M.state = ST_GOAL; M.st = 170; M.scorer = ti;
  playSnd(S_GOAL);
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
      } else {
        Player *gk = NULL;
        for (int k = 0; k < M.np; k++) if (M.pl[k].t == defT && M.pl[k].gk) gk = &M.pl[k];
        if (gk) {
          b->x = ownX(defT) + M.dir[defT] * 20; b->y = FW / 2; b->z = b->vx = b->vy = b->vz = 0;
          gk->x = b->x - M.dir[defT] * 4; gk->y = b->y;
          b->owner = gk; b->last = defT; gk->hold = 0; gk->kick = 0;
        }
      }
    }
  }
}

static void updatePickup(void) {
  Ball *b = &M.ball;
  if (b->owner || M.state != ST_PLAY || M.freeze > 0) return;
  Player *best = NULL; float bd = 1e9f;
  for (int k = 0; k < M.np; k++) {
    Player *p = &M.pl[k];
    float reach = p->gk ? 15.0f : 9.0f, hz = p->gk ? 34.0f : 13.0f;
    if (p->kick <= 0 && p->stun <= 0 && b->z <= hz) {
      float d = dist(p->x, p->y, b->x, b->y);
      if (d < reach && d < bd) { best = p; bd = d; }
    }
  }
  if (!best) return;
  float sp = sqrtf(b->vx * b->vx + b->vy * b->vy);
  if (best->gk) {
    static const float ccs[3] = { 0.40f, 0.52f, 0.64f };
    float cc = ccs[clampi(OPT.diff, 1, 3) - 1];
    if (best->t == 1) cc = 0.62f;
    cc += TEAMS[M.team[best->t]].str * 0.02f;
    if (sp > 7 && prob() > cc) {
      b->vx = -b->vx * 0.45f + rndf(-1.2f, 1.2f);
      b->vy = b->vy * 0.5f + rndf(-3, 3);
      b->vz = rndf(0.6f, 1.8f);
      best->kick = 25; b->last = best->t;
      playSnd(S_POST);
      return;
    }
  } else if (sp > 11) {
    b->vx *= 0.5f; b->vy *= 0.5f; best->kick = 12;
    return;
  }
  b->owner = best; b->last = best->t; best->hold = 0;
  b->vx = b->vy = b->vz = 0;
  if (best->t == 1 && !best->gk && !M.training) M.ctrl = best;
}

static void separatePlayers(void) {
  for (int i = 0; i < M.np - 1; i++) {
    Player *a = &M.pl[i];
    for (int j = i + 1; j < M.np; j++) {
      Player *c = &M.pl[j];
      float dx = c->x - a->x, dy = c->y - a->y;
      float d = sqrtf(dx * dx + dy * dy);
      if (d < 9 && d > 0.01f) {
        float push = (9 - d) * 0.5f, ux = dx / d, uy = dy / d;
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
  SCENE = SC_RESULT;
}

static void updateMatch(void) {
  Ball *b = &M.ball;
  if (hit(B_START) && (M.state == ST_PLAY || M.state == ST_KICK)) { M.paused = 1; M.pmenu = 1; return; }
  for (int k = 0; k < M.np; k++) {
    Player *p = &M.pl[k];
    if (p->kick > 0) p->kick--;
    if (p->stun > 0) p->stun--;
    if (p->tack > 0) p->tack--;
  }
  if (M.freeze > 0) M.freeze--;

  if (M.state == ST_KICK) {
    if (--M.st == 40) playSnd(S_WHISTLE);
    if (M.st <= 0) M.state = ST_PLAY;
    if (M.ctrl) humanControl(M.ctrl);
    updateBall();
    return;
  } else if (M.state == ST_GOAL) {
    M.st--;
    updateBall();
    b->x = clampf(b->x, -30, FL + 30);
    for (int k = 0; k < M.np; k++) { M.pl[k].vx *= 0.8f; M.pl[k].vy *= 0.8f; M.pl[k].moving = 0; }
    if (M.st <= 0) startKickoff(M.training ? 1 : ((M.scorer == 1) ? 2 : 1));
    return;
  } else if (M.state == ST_HALF) {
    if (--M.st <= 0 || hit(B_CROSS)) {
      M.half = 2; M.hf = 0; M.dir[1] = -1; M.dir[2] = 1;
      startKickoff(2);
    }
    return;
  } else if (M.state == ST_FULL) {
    if (hit(B_CROSS) || hit(B_START)) finishMatch();
    return;
  }

  if (!M.training) {
    M.hf++;
    if (M.hf >= M.halfFrames) {
      playSnd(S_WHISTLE);
      if (M.half == 1) { M.state = ST_HALF; M.st = 240; } else M.state = ST_FULL;
      return;
    }
  }
  if (!M.ctrl || M.ctrl->gk) {
    if (M.training) { for (int k = 0; k < M.np; k++) if (M.pl[k].t == 1) M.ctrl = &M.pl[k]; }
    else M.ctrl = nearestField(1, b->x, b->y, NULL);
  }
  for (int k = 0; k < M.np; k++) {
    Player *p = &M.pl[k];
    Tgt t = (p == M.ctrl) ? humanControl(p) : aiPlayer(p);
    if (M.freeze > 0) t.sf = 0;
    movePlayer(p, t.x, t.y, t.sf);
  }
  separatePlayers();
  updatePickup();
  updateBall();
}

/* ---------------- desenho ---------------- */
typedef struct { float y; int ball; Player *p; } Item;
static int cmpItem(const void *a, const void *b) {
  float d = ((const Item *)a)->y - ((const Item *)b)->y;
  return (d < 0) ? -1 : (d > 0 ? 1 : 0);
}

static void drawHUD(void) {
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
  if (M.state == ST_GOAL) {
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

static void drawMatch(void) {
  drawBG(M.futsal ? T_BG_COURT : T_BG_FIELD);
  Item items[MAXP + 1]; int n = 0;
  for (int k = 0; k < M.np; k++) { items[n].y = M.pl[k].y; items[n].ball = 0; items[n].p = &M.pl[k]; n++; }
  items[n].y = M.ball.y + 0.5f; items[n].ball = 1; items[n].p = NULL; n++;
  qsort(items, n, sizeof(Item), cmpItem);
  for (int k = 0; k < n; k++) {
    float sx, sy;
    if (items[k].ball) {
      Ball *b = &M.ball;
      toScreen(b->x, b->y, &sx, &sy);
      drawFrame(FR_SHADOW, sx, sy + 1);
      drawFrame(FR_BALL, sx, sy - b->z * 0.55f);
    } else {
      Player *p = items[k].p;
      toScreen(p->x, p->y, &sx, &sy);
      drawFrame(FR_SHADOW, sx, sy);
      drawFrame((p->t == 1) ? FR_RING_H : FR_RING_A, sx, sy);
      int fi = 0, oy = 0;
      if (p->moving) { int ph = (p->anim / 7) % 2; fi = ph == 0 ? 1 : 2; oy = -ph; }
      if (p->stun > 0) { fi = 0; oy = 1; }
      drawFrame(KFR[p->kit][fi], sx, sy + oy);
      if (p == M.ctrl && M.state != ST_GOAL) drawFrame(FR_ARROW, sx, sy - 5 + (M.hf / 8) % 2);
    }
  }
  if (M.ctrl && M.charge > 0) {
    float sx, sy; toScreen(M.ctrl->x, M.ctrl->y, &sx, &sy);
    box((int)sx - 15, (int)sy + 3, 30, 4, C_BLACK);
    box((int)sx - 14, (int)sy + 4, (int)(28 * M.charge), 2, (M.charge > 0.85f) ? C_RED : C_GREEN);
  }
  drawHUD();
}

static void drawPause(void) {
  box(150, 70, 180, 112, C_PANEL);
  ctxt(78, "PAUSA", C_YELLOW, 240);
  static const char *items[3] = { "CONTINUAR", "REINICIAR PARTIDA", "SAIR PARA O MENU" };
  for (int i = 1; i <= 3; i++) ctxt(104 + (i - 1) * 20, items[i - 1], (M.pmenu == i) ? C_SEL : C_WHITE, 240);
}

void sceneMatch(void) {
  if (!M.active) { SCENE = SC_MENU; return; }
  if (M.paused) {
    drawMatch(); drawPause();
    if (hit(B_UP)) M.pmenu = (M.pmenu > 1) ? M.pmenu - 1 : 1;
    if (hit(B_DOWN)) M.pmenu = (M.pmenu < 3) ? M.pmenu + 1 : 3;
    if (hit(B_START) || hit(B_TRIANGLE)) M.paused = 0;
    if (hit(B_CROSS)) {
      if (M.pmenu == 1) M.paused = 0;
      else if (M.pmenu == 2) newMatch(M.team[1], M.team[2], M.futsal, M.training, M.trainKit, M.mode);
      else { M.active = 0; SCENE = SC_MENU; }
    }
    return;
  }
  updateMatch();
  if (M.active && SCENE == SC_MATCH) drawMatch();
}
