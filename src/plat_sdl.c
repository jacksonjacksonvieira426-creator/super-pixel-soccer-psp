/* Backend PSP: SDL2 + SDL2_image (audio via SDL puro) */
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <pspkernel.h>
#include "plat.h"
#include "game.h"

PSP_MODULE_INFO("SuperPixelSoccer", PSP_MODULE_USER, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

static int running = 1;
static SDL_Window *win;
static SDL_Renderer *ren;
static SDL_Joystick *joy;
static SDL_Texture *texs[16];
static int ntex = 0;

#define NSND 8
typedef struct { Uint8 *buf; Uint32 len; Uint32 pos; int playing; } Snd;
static Snd snds[NSND];
static int nsnd = 0;
static SDL_AudioDeviceID adev = 0;

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

static void audio_cb(void *ud, Uint8 *stream, int len) {
  memset(stream, 0, len);
  for (int i = 0; i < nsnd; i++) {
    Snd *s = &snds[i];
    if (!s->playing) continue;
    Uint32 n = s->len - s->pos;
    if (n > (Uint32)len) n = (Uint32)len;
    SDL_MixAudioFormat(stream, s->buf + s->pos, AUDIO_S16, n, SDL_MIX_MAXVOLUME / 2);
    s->pos += n;
    if (s->pos >= s->len) s->playing = 0;
  }
}

void plat_init(void) {
  setup_callbacks();
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO);
  IMG_Init(IMG_INIT_PNG);
  win = SDL_CreateWindow("SPS", 0, 0, 480, 272, 0);
  ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
  SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
  joy = SDL_JoystickOpen(0);
}
void plat_quit(void) {
  if (adev) SDL_CloseAudioDevice(adev);
  if (joy) SDL_JoystickClose(joy);
  SDL_Quit();
  sceKernelExitGame();
}
int plat_load_tex(const char *path) {
  if (ntex >= 16) return -1;
  SDL_Texture *t = IMG_LoadTexture(ren, path);
  if (!t) return -1;
  SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
  texs[ntex] = t;
  return ntex++;
}
void plat_blit(int tex, int sx, int sy, int w, int h, int dx, int dy, int r, int g, int b, int a) {
  if (tex < 0) return;
  SDL_Texture *t = texs[tex];
  SDL_SetTextureColorMod(t, r, g, b);
  SDL_SetTextureAlphaMod(t, a);
  if (w == 0 || h == 0) {
    int tw, th; SDL_QueryTexture(t, NULL, NULL, &tw, &th);
    SDL_Rect d = { dx, dy, tw, th };
    SDL_RenderCopy(ren, t, NULL, &d);
  } else {
    SDL_Rect s = { sx, sy, w, h }, d = { dx, dy, w, h };
    SDL_RenderCopy(ren, t, &s, &d);
  }
}
void plat_rect(int x, int y, int w, int h, int r, int g, int b, int a) {
  SDL_SetRenderDrawColor(ren, r, g, b, a);
  SDL_Rect rc = { x, y, w, h };
  SDL_RenderFillRect(ren, &rc);
}
void plat_clear(int r, int g, int b) { SDL_SetRenderDrawColor(ren, r, g, b, 255); SDL_RenderClear(ren); }
void plat_present(void) { SDL_RenderPresent(ren); }

/* ordem dos botoes do SDL2 no PSP */
enum { J_TRI = 0, J_CIR, J_CRO, J_SQU, J_L, J_R, J_DOWN, J_LEFT, J_UP, J_RIGHT, J_SEL, J_START };
void plat_input(int btn[12], int *ax, int *ay) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) { if (e.type == SDL_QUIT) running = 0; }
  memset(btn, 0, sizeof(int) * 12);
  *ax = *ay = 0;
  if (!joy) return;
  btn[B_UP] = SDL_JoystickGetButton(joy, J_UP);
  btn[B_DOWN] = SDL_JoystickGetButton(joy, J_DOWN);
  btn[B_LEFT] = SDL_JoystickGetButton(joy, J_LEFT);
  btn[B_RIGHT] = SDL_JoystickGetButton(joy, J_RIGHT);
  btn[B_CROSS] = SDL_JoystickGetButton(joy, J_CRO);
  btn[B_CIRCLE] = SDL_JoystickGetButton(joy, J_CIR);
  btn[B_SQUARE] = SDL_JoystickGetButton(joy, J_SQU);
  btn[B_TRIANGLE] = SDL_JoystickGetButton(joy, J_TRI);
  btn[B_L] = SDL_JoystickGetButton(joy, J_L);
  btn[B_R] = SDL_JoystickGetButton(joy, J_R);
  btn[B_START] = SDL_JoystickGetButton(joy, J_START);
  btn[B_SELECT] = SDL_JoystickGetButton(joy, J_SEL);
  *ax = SDL_JoystickGetAxis(joy, 0) / 256;
  *ay = SDL_JoystickGetAxis(joy, 1) / 256;
}

int plat_snd_load(const char *path) {
  if (nsnd >= NSND) return -1;
  SDL_AudioSpec spec; Uint8 *buf; Uint32 len;
  if (!SDL_LoadWAV(path, &spec, &buf, &len)) return -1;
  if (!adev) {
    SDL_AudioSpec want = spec, have;
    want.callback = audio_cb; want.userdata = NULL; want.samples = 1024;
    adev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (!adev) { SDL_FreeWAV(buf); return -1; }
    SDL_PauseAudioDevice(adev, 0);
  }
  snds[nsnd].buf = buf; snds[nsnd].len = len; snds[nsnd].pos = 0; snds[nsnd].playing = 0;
  return nsnd++;
}
void plat_snd_play(int id) {
  if (id < 0 || id >= nsnd || !adev) return;
  SDL_LockAudioDevice(adev);
  snds[id].pos = 0; snds[id].playing = 1;
  SDL_UnlockAudioDevice(adev);
}
unsigned plat_ticks(void) { return SDL_GetTicks(); }
void plat_delay(unsigned ms) { SDL_Delay(ms); }
int plat_quit_requested(void) { return !running || SCENE == -1; }

int main(int argc, char *argv[]) {
  game_init();
  while (!plat_quit_requested()) {
    unsigned t0 = plat_ticks();
    game_frame();
    plat_present();
    unsigned dt = plat_ticks() - t0;
    if (dt < 16) plat_delay(16 - dt);
  }
  plat_quit();
  return 0;
}
