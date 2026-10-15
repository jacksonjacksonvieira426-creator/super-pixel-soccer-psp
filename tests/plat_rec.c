/* backend de captura: grava as chamadas de desenho do ultimo quadro para renderizar prévias */
#include <stdio.h>
#include <string.h>
#include "../src/plat.h"
int TEST_BTN[12]; int TEST_AX, TEST_AY;
extern int SCENE;
static char paths[64][64]; static int np = 0;
static FILE *out = NULL;
void rec_open(const char *f) { if (out) fclose(out); out = fopen(f, "w"); for (int i = 0; i < np; i++) fprintf(out, "T %d %s\n", i, paths[i]); }
void rec_close(void) { if (out) fclose(out); out = NULL; }
void plat_init(void) {}
void plat_quit(void) {}
int plat_load_tex(const char *p) { strncpy(paths[np], p, 63); return np++; }
void plat_blit(int t, int sx, int sy, int w, int h, int dx, int dy, int r, int g, int b, int a) { if (out) fprintf(out, "B %d %d %d %d %d %d %d %d %d %d %d\n", t, sx, sy, w, h, dx, dy, r, g, b, a); }
void plat_blit_flip(int t, int sx, int sy, int w, int h, int dx, int dy, int fl) { if (out) fprintf(out, "F %d %d %d %d %d %d %d %d\n", t, sx, sy, w, h, dx, dy, fl); }
void plat_rect(int x, int y, int w, int h, int r, int g, int b, int a) { if (out) fprintf(out, "R %d %d %d %d %d %d %d %d\n", x, y, w, h, r, g, b, a); }
void plat_clear(int r, int g, int b) { if (out) fprintf(out, "C %d %d %d\n", r, g, b); }
void plat_present(void) {}
void plat_input(int btn[12], int *ax, int *ay) { for (int i = 0; i < 12; i++) btn[i] = TEST_BTN[i]; *ax = TEST_AX; *ay = TEST_AY; }
int plat_snd_load(const char *p) { return 0; }
void plat_snd_play(int id) {}
void plat_snd_loop(int id, int vol) {}
void plat_snd_stop(int id) {}
unsigned plat_ticks(void) { return 0; }
void plat_delay(unsigned ms) {}
int plat_quit_requested(void) { return 0; }
