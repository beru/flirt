CC = gcc
CFLAGS = -g -Wall -I.. -I/usr/local/include `sdl-config --cflags`

LDFLAGS = -L.. -lz -ljpeg -lpng -lmad -lflirt `sdl-config --libs`
OS = shell uname -s

ifeq ("$(shell uname)", "Darwin")
  LDFLAGS += -framework Carbon -framework Cocoa
endif

OFILES = sdltest.o

sdltest: ${OFILES} ../libflirt.a
	${CC} ${OFILES} -o sdltest ${LDFLAGS}

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

clean:
	rm sdltest ${OFILES}
