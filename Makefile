TARGET = SuperPixelSoccer
OBJS = src/game_core.o src/game_ui.o src/plat_sdl.o

CFLAGS = -O2 -G0 -Wall $(shell psp-pkg-config --cflags sdl2 SDL_image 2>/dev/null || echo "-I$(shell psp-config --psp-prefix)/include/SDL2")
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = -lSDL2main -lSDL2_image -lSDL2 -lpng -ljpeg -lz -lm -lGL -lGLU \
       -lpspvfpu -lpsphprm -lpspvram -lpspaudio -lpspgu -lpspge -lpspdisplay -lpspctrl -lpsppower -lpspdebug -lpspsdk

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Super Pixel Soccer

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
