CC = gcc

CFLAGS = -g -Wall

#CFLAGS = -g -O5
#CFLAGS = -pg -g -a -Wall

LDFLAGS = -lz -ljpeg -lmad -lpng -L/usr/local/lib

DATE = `date +%Y%m%d`
OS = shell uname -s

ifeq ("$(shell uname)", "Darwin")
  CFLAGS += -DDD_PLATFORM_MAC_OS_X=1 -DDD_BIG_ENDIAN=1
  LDFLAGS += -framework Carbon
else
ifeq ("$(shell uname)", "Linux")
  CFLAGS += -DDD_PLATFORM_LINUX=1 -DDD_LITTLE_ENDIAN=1
else
ifeq ("$(shell uname)", "FreeBSD")
  CFLAGS += -I/usr/local/include -DDD_PLATFORM_LINUX=1 -DDD_LITTLE_ENDIAN=1 -O2  -I/usr/local/include
endif
endif
endif

ACTIONFILES = \
	src/action/action.o \
	src/action/class.o \
	src/action/context.o \
	src/action/event.o \
	src/action/function.o \
	src/action/hash.o \
	src/action/locals.o \
	src/action/object.o \
	src/action/opcodes.o \
	src/action/stack.o \
	src/action/value.o

ACTIONCLASSFILES = \
	src/action/classes/array_class.o \
	src/action/classes/color_class.o \
	src/action/classes/date_class.o \
	src/action/classes/function_class.o \
	src/action/classes/key_class.o \
	src/action/classes/math_class.o \
	src/action/classes/movieclip_class.o \
	src/action/classes/object_class.o \
	src/action/classes/string_class.o

FORMATFILES = \
	src/format/read.o \
	src/format/readaction.o \
	src/format/readbitmap.o \
	src/format/readbutton.o \
	src/format/readfont.o \
	src/format/readjpeg.o \
	src/format/readplace.o \
	src/format/readshape.o \
	src/format/readsound.o \
	src/format/readsprite.o \
	src/format/readtext.o \
	src/format/readthread.o

RENDERFILES = \
	src/render/cxform.o \
	src/render/edgelist.o \
	src/render/fill.o \
	src/render/gradient.o \
	src/render/image.o \
	src/render/matrix.o \
	src/render/raster.o \
	src/render/raster_bitmap.o \
	src/render/raster_grad.o \
	src/render/raster_solid.o \
	src/render/rect.o \
	src/render/shape.o \
	src/render/shapeinstance.o \
	src/render/updatelist.o

PLAYERFILES = \
	src/player/button.o \
	src/player/character.o \
	src/player/charlist.o \
	src/player/displaylist.o \
	src/player/drawable.o \
	src/player/drawbutton.o \
	src/player/drawclip.o \
	src/player/drawmorph.o \
	src/player/drawshape.o \
	src/player/drawtext.o \
	src/player/drawtextfield.o \
	src/player/font.o \
	src/player/morphshape.o \
	src/player/movieclip.o \
	src/player/player.o \
	src/player/sound.o \
	src/player/text.o \
	src/player/textfield.o \
	src/player/timeline.o

SOUNDFILES = \
	src/sound/mixer.o \
	src/sound/mp3decoder.o

OFILES = src/libdd.o src/types.o \
	 ${PLAYERFILES} ${RENDERFILES} ${FORMATFILES} ${ACTIONFILES} \
	 ${ACTIONCLASSFILES} ${SOUNDFILES}

ARCHIVEFILES = \
	Makefile README LICENSE \
	test.c exttest.c imageext.c exttest.swf \
	flirt.h flirt-debug.h \
	docs src ex Mac\ OS\ X

libflirt: ${OFILES}
	ar r libflirt.a ${OFILES}
	ranlib libflirt.a

%.o: %.c
	${CC} ${CFLAGS} ${DEFINES} -c $< -o $@

test: libflirt test.o
	${CC} test.o libflirt.a -o test -lm ${LDFLAGS}

exttest: libflirt exttest.o imageext.o
	${CC} exttest.o imageext.o libflirt.a -o exttest -lm ${LDFLAGS}

clean:
	rm -f ${OFILES} test.o test imageext.o exttest.o exttest \
		exttest.exe libflirt.a core test.exe test.png
	rm -rf Mac\ OS\ X/build

archive: clean
	mkdir flirt-${DATE}
	cp -r ${ARCHIVEFILES} flirt-${DATE}
	find flirt-${DATE} -name CVS -or -name .cvsignore -or -name .DS_Store -print0 | xargs -0 rm -rf
	tar zcvf flirt-${DATE}.tgz flirt-${DATE}
	rm -rf flirt-${DATE}

# These are leftover from early days, left here so I remember how to 
# build if I ever try this on cygwin or linux again..

sdltest: libflirt sdltest.o
	${CC} -Dmain=SDL_main sdltest.o ${OFILES} \
	  /usr/local/cross-tools/lib/libz.a \
	  /usr/local/cross-tools/lib/libjpeg.a \
	  -o test.exe -lmingw32 -lm -lSDLmain -lSDL -L. -mwindows

cygtest: libflirt sdltest.o
	${CC} -Dmain=SDL_main sdltest.o ${OFILES} \
	  /usr/lib/libz.a /usr/lib/libjpeg.a \
	  -o test.exe -lm -lSDL -L.

puretest: ${OFILES} test.o
	purify ${CC} -g -Wall test.o ${OFILES} -o test -lm -lz
