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

default: retro
.PHONY: default

OFILES = disassemble.o endian.o loader.o ngaro.o devices.o vm.o
$(OFILES): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

retro: $(OFILES)
	$(CC) $(LIBS) $^ -o $@

clean:
	@rm -f retro *.o
