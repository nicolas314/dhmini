# st7789 Makefile

CC      = gcc
CFLAGS  = -O3 -fpic
IFLAGS  = -I./include -I./stb -I/usr/include/freetype2
LDFLAGS = -L. -ldhmini -lgpiod -lm -lfreetype

AR      = ar
ARFLAGS = rcs
RM      = rm -f

# implicit rules
SUFFIXES = .o .c .h .a

COMPILE.c=$(CC) $(CFLAGS) $(IFLAGS) -c
.c.o:
	$(COMPILE.c) -o $@ $<

default: libdhmini.a progs

clean:
	$(RM) libdhmini.a src/*.o $(PROGS)

SRCS =   src/dhmini.c \
         src/buttons.c \
         src/console.c \
         src/httpget.c \
         src/image.c \
         src/led.c \
         src/truetype.c \
         src/random.c

OBJS = $(SRCS:.c=.o)

libdhmini.a: $(OBJS)
	$(AR) $(ARFLAGS) libdhmini.a $(OBJS)	

PROGS = bin/clock \
        bin/demo \
        bin/load \
        bin/banner \
        bin/fixed \
        bin/dhtty

progs: $(PROGS)

bin/clock: progs/clock.c
	$(CC) $(CFLAGS) $(IFLAGS) -o bin/clock progs/clock.c $(LDFLAGS) -lpthread

bin/demo: progs/demo.c
	$(CC) $(CFLAGS) $(IFLAGS) -o bin/demo progs/demo.c $(LDFLAGS)

bin/load: progs/load.c
	$(CC) $(CFLAGS) $(IFLAGS) -o bin/load progs/load.c $(LDFLAGS) -lcurl

bin/banner: progs/banner.c
	$(CC) $(CFLAGS) $(IFLAGS) -o bin/banner progs/banner.c $(LDFLAGS)

bin/fixed: progs/fixed.c
	$(CC) $(CFLAGS) $(IFLAGS) -o bin/fixed progs/fixed.c $(LDFLAGS)

bin/dhtty: progs/dhtty.c
	$(CC) $(CFLAGS) $(IFLAGS) -o bin/dhtty progs/dhtty.c $(LDFLAGS)
