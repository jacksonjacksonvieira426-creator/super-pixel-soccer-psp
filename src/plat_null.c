/* Backend de teste no PC: sem tela, entrada injetada pelo teste */
#include <time.h>
#include "plat.h"
int TEST_BTN[12]; int TEST_AX, TEST_AY; long TEST_BLITS, TEST_RECTS;
extern int SCENE;
void plat_init(void) {}
void plat_quit(void) {}
int plat_load_tex(const char *p) { static int n = 0; return n++; }
int plat_load_tex_fast(const char *p) { return plat_load_tex(p); }
void plat_blit(int tex, int sx, int sy, int w, int h, int dx, int dy, int r, int g, int b, int a) {
  TEST_BLITS++;
  if (tex < 0) { *(volatile int *)0 = 1; }
}
void plat_blit_flip(int tex, int sx, int sy, int w, int h, int dx, int dy, int flip) { TEST_BLITS++; if (tex < 0) { *(volatile int *)0 = 1; } }
void plat_rect(int x, int y, int w, int h, int r, int g, int b, int a) { TEST_RECTS++; }
void plat_clear(int r, int g, int b) {}
void plat_present(void) {}
void plat_input(int btn[12], int *ax, int *ay) { for (int i = 0; i < 12; i++) btn[i] = TEST_BTN[i]; *ax = TEST_AX; *ay = TEST_AY; }
int plat_snd_load(const char *p) { return 0; }
void plat_snd_play(int id) {}
void plat_snd_loop(int id, int vol) {}
void plat_snd_stop(int id) {}
unsigned plat_ticks(void) { return 0; }
void plat_delay(unsigned ms) {}
int plat_quit_requested(void) { return SCENE == -1; }
