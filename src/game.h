#ifndef GAME_H
#define GAME_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "plat.h"
#include "frames.h"
#include "teams.h"

#define SW 480
#define SH 272
#define FL 800.0f          /* comprimento logico do campo */
#define FW 420.0f          /* largura logica */
#define GOAL_Y1 150.0f
#define GOAL_Y2 250.0f
#define GOAL_H 30.0f
#define GRAV 0.12f
#define MAXP 12

typedef struct { int r, g, b, a; } Col;
#define COL(r, g, b, a) ((Col){ r, g, b, a })
#define C_WHITE  COL(255,255,255,255)
#define C_BLACK  COL(0,0,0,255)
#define C_YELLOW COL(255,230,60,255)
#define C_RED    COL(255,90,90,255)
#define C_GREEN  COL(90,235,130,255)
#define C_GREY   COL(170,170,170,255)
#define C_DIM    COL(0,0,0,150)
#define C_PANEL  COL(10,24,50,215)
#define C_SEL    COL(255,200,40,230)
#define C_ORANGE COL(255,160,40,255)

enum { B_UP, B_DOWN, B_LEFT, B_RIGHT, B_CROSS, B_CIRCLE, B_SQUARE, B_TRIANGLE, B_L, B_R, B_START, B_SELECT };
enum { SC_TITLE, SC_MENU, SC_TEAMSEL, SC_TRAINSEL, SC_OPTS, SC_MATCH, SC_RESULT, SC_PENS, SC_LEAGUE, SC_CUP };
enum { ST_KICK, ST_PLAY, ST_GOAL, ST_HALF, ST_FULL, ST_FOUL };
enum { MODE_FRIENDLY, MODE_LEAGUE, MODE_CUP, MODE_TRAIN };

typedef struct { int diff, half, sound, stadium, music; } Opt;
typedef struct { const char *name, *shortn; int kit; int kits[5]; int nkits; int str; Col col; int gk; } Team;

typedef struct {
  int t, i, gk;
  float x, y, vx, vy, fx, fy;
  int kick, stun, tack, anim, hold, moving, kit;
  int nameIdx, yellow, out; float pace, shot, defn;
} Player;

typedef struct { float x, y, z, vx, vy, vz; Player *owner; int last; } Ball;

typedef struct {
  int team[3];            /* indices em TEAMS para o time 1 e 2 */
  int mode, futsal, training, trainKit;
  int score[3], shots[3], half, hf, dir[3];
  int state, st, np, freeze, halfFrames, scorer, kickTeam, humanShot;
  int paused, pmenu, active;
  float charge;
  int foulT, foulPen, reds[3], yellows[3];
  float foulX, foulY;
  char msg[64];
  const float (*form[3])[2];
  int nform, poss[3], formIdx[3];
  Player pl[MAXP];
  Player *ctrl;
  Ball ball;
} Match;

typedef struct { int s1, s2, t1, t2, mode, shots1, shots2, poss1, poss2; } Result;

/* ---- core ---- */
extern Match M;
extern Opt OPT;
extern Team TEAMS[NTEAMS];
extern int SCENE, TICK, UserForm;
extern int T_ATLAS, T_BG[4], T_BG_COURT, T_TITLE, T_FONT;
#define T_BG_FIELD T_BG[0]
extern int S_KICK, S_WHISTLE, S_GOAL, S_POST, S_CHEER, S_MUSIC;
int bgTex(void);
int formCount(int futsal);
const char *formName(int futsal, int i);
const char *playerName(const Player *p);
extern Result RES;

void readPad(void);
int hit(int b);
int held(int b);
void dirInput(float *dx, float *dy);
float clampf(float v, float a, float b);
int clampi(int v, int a, int b);
float rndf(float a, float b);
float prob(void);
float dist(float ax, float ay, float bx, float by);
float normf(float dx, float dy, float *ox, float *oy);
void txt(int x, int y, const char *s, Col c);
void ctxt(int y, const char *s, Col c, int cx);
void box(int x, int y, int w, int h, Col c);
void drawBG(int tex);
void drawFrame(const short *fr, float x, float y);
void drawKit(int kit, int fi, float x, float y);   /* fi 0 parado, 1 corrida, 2 corrida espelhada */
void toScreen(float x, float y, float *sx, float *sy);
void playSnd(int id);
void saveOpt(void);
void loadOpt(void);

extern int dbgFouls, dbgReds, dbgYellows, dbgPens;
void matchPenaltyDone(int att, int goal);
void startMatchPenalty(int att);
void newMatch(int t1, int t2, int futsal, int training, int trainKit, int mode);
void sceneMatch(void);
void onMatchEnd(int s1, int s2);

/* ---- ui ---- */
void game_init(void);
void game_frame(void);

#endif
