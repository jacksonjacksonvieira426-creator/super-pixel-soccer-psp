TARGET = SuperPixelSoccer
OBJS = src/main.o

PSPSDK = $(shell psp-config --pspsdk-path)
PSPDEV = $(shell psp-config --pspdev-path)

INCDIR = $(PSPDEV)/psp/include/SDL2 $(PSPDEV)/psp/include/SDL $(PSPDEV)/psp/include/libpng16
CFLAGS = -O2 -G0 -Wall -D_PSP_FW_VERSION=600 -D_GNU_SOURCE=1 -Dmain=SDL_main
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS = -lSDL2main -lSDL2_image -lSDL2 -lpng -ljpeg -lz -lm -lGL -lGLU -lpspvfpu -lpsphprm -lpspvram -lpspaudio -lpspgu -lpspge -lpspdisplay -lpspctrl -lpsppower -lpspdebug -lpspsdk -lpspnet -lpspnet_apctl

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Super Pixel Soccer

include $(PSPSDK)/lib/build.mak
