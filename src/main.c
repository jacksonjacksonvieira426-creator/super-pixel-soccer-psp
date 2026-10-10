/* Super Pixel Soccer PSP - esqueleto em C (PSPSDK + SDL2)
   Etapa 1: confirmar que o build do GitHub Actions funciona e que
   os sprites carregam. Depois portamos os modos do jogo em Lua. */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <pspkernel.h>
#include <math.h>

PSP_MODULE_INFO("SuperPixelSoccer", PSP_MODULE_USER, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

static int running = 1;
static int exit_cb(int a, int b, void *c) { running = 0; return 0; }
static int cb_thread(SceSize args, void *argp) {
  int id = sceKernelCreateCallback("exit", exit_cb, NULL);
  sceKernelRegisterExitCallback(id);
  sceKernelSleepThreadCB();
  return 0;
}
static void setup_callbacks(void) {
  int th = sceKernelCreateThread("cb", cb_thread, 0x11, 0xFA0, 0, NULL);
  if (th >= 0) sceKernelStartThread(th, 0, NULL);
}

/* botoes do SDL2 no PSP */
enum { B_TRI = 0, B_CIR, B_CRO, B_SQU, B_L, B_R, B_DOWN, B_LEFT, B_UP, B_RIGHT, B_SEL, B_START };

#undef main
int main(int argc, char *argv[]) {
  setup_callbacks();
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
  IMG_Init(IMG_INIT_PNG);
  SDL_Window *win = SDL_CreateWindow("SPS", 0, 0, 480, 272, 0);
  SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
  SDL_Joystick *joy = SDL_JoystickOpen(0);

  SDL_Texture *bg = IMG_LoadTexture(ren, "gfx/bg_field.png");
  SDL_Texture *atlas = IMG_LoadTexture(ren, "gfx/atlas.png");
  SDL_Texture *title = IMG_LoadTexture(ren, "gfx/title.png");

  float px = 240, py = 180;
  int anim = 0;
  while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) { if (e.type == SDL_QUIT) running = 0; }
    float dx = 0, dy = 0;
    if (joy) {
      if (SDL_JoystickGetButton(joy, B_LEFT)) dx -= 1;
      if (SDL_JoystickGetButton(joy, B_RIGHT)) dx += 1;
      if (SDL_JoystickGetButton(joy, B_UP)) dy -= 1;
      if (SDL_JoystickGetButton(joy, B_DOWN)) dy += 1;
      if (SDL_JoystickGetButton(joy, B_START)) running = 0;
    }
    px += dx * 2; py += dy * 2;
    if (dx || dy) anim++;
    if (px < 20) px = 20; if (px > 460) px = 460;
    if (py < 80) py = 80; if (py > 250) py = 250;

    SDL_RenderClear(ren);
    if (bg) SDL_RenderCopy(ren, bg, NULL, NULL);
    if (title) { SDL_Rect d = { 10, 10, 460, 100 }; SDL_RenderCopy(ren, title, NULL, &d); }
    if (atlas) {
      /* celula 40x40; coluna 0..11, linha 0: quadro "c1" parado/correndo */
      int col = (dx || dy) ? 1 + ((anim / 7) % 2) : 0;
      SDL_Rect s = { col * 40, 0, 40, 40 };
      SDL_Rect d = { (int)px - 20, (int)py - 39, 40, 40 };
      SDL_RenderCopy(ren, atlas, &s, &d);
    }
    SDL_RenderPresent(ren);
    SDL_Delay(16);
  }
  if (joy) SDL_JoystickClose(joy);
  SDL_Quit();
  sceKernelExitGame();
  return 0;
}
