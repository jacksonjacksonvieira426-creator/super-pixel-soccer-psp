TARGET = SuperPixelSoccer
OBJS = src/game_core.o src/game_ui.o src/plat_sdl.o

CFLAGS = -O2 -G0 -Wall -D_PSP_FW_VERSION=600 -D_GNU_SOURCE=1 -Dmain=SDL_main
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = -lSDL2main -lSDL2_image -lSDL2 -lpng -ljpeg -lz -lm -lGL -lGLU -lpspvfpu -lpsphprm -lpspvram -lpspaudio -lpspgu -lpspge -lpspdisplay -lpspctrl -lpsppower -lpspdebug -lpspsdk

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Super Pixel Soccer

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
