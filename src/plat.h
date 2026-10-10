/* Camada de plataforma: PSP (SDL2) ou teste no PC (plat_null.c) */
#ifndef PLAT_H
#define PLAT_H

void plat_init(void);
void plat_quit(void);
int  plat_load_tex(const char *path);          /* devolve id ou -1 */
void plat_blit(int tex, int sx, int sy, int w, int h, int dx, int dy,
               int r, int g, int b, int a);    /* w=h=0: textura inteira */
void plat_rect(int x, int y, int w, int h, int r, int g, int b, int a);
void plat_clear(int r, int g, int b);
void plat_present(void);
/* btn: up down left right cross circle square triangle l r start select */
void plat_input(int btn[12], int *ax, int *ay);
int  plat_snd_load(const char *path);          /* devolve id ou -1 */
void plat_snd_play(int id);
unsigned plat_ticks(void);
void plat_delay(unsigned ms);
int  plat_quit_requested(void);

#endif
