# .----,   .-----  -------  .----,    ,---,
# |     \  |          |     |     \  |     |
# |-----/  |---       |     |-----/  |     |
# |  \     |          |     |  \     |     |
# |   \    |          |     |   \    |     |
# |    \   `-----     |     |    \    `---'
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Written by Charles Childers
# This code is gifted to the public domain.
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CC = gcc
CFLAGS = -Wall -O3 -fomit-frame-pointer -DUSE_SDL `sdl-config --cflags`
LIBS = `sdl-config --libs`

default: retro-fb
.PHONY: default

OFILES = disassemble.o endian.o loader.o ngaro.o devices.o vm.o
$(OFILES): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

initial_image.o: initial_image.c
	$(CC) -Wall -c $< -o $@

retro-fb: $(OFILES) initial_image.o
	$(CC) $(LIBS) $^ -o $@

clean:
	@rm -f retro-fb *.o
