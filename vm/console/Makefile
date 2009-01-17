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
CURSES = -lncurses
CC = gcc
CFLAGS = -Wall -O3 -fomit-frame-pointer

default: retro retro-fast
.PHONY: default

# Most files can be compiled using CFLAGS...
OFILES = devices.o disassemble.o endian.o loader.o ngaro.o
$(OFILES) vm.o: %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ... but vm_mat.o needs -DMAT as well...
vm_mat.o: vm_mat.c
	$(CC) -DMAT $(CFLAGS) -c $< -o $@

# ... and initial_image.o compiles much faster without -O3.
initial_image.o: initial_image.c
	$(CC) -Wall -c $< -o $@

# Both executables link the same way, but the dependencies differ.
retro: $(OFILES) initial_image.o vm.o
	$(CC) $(CURSES) $^ -o $@
retro-fast: $(OFILES) initial_image.o vm_mat.o
	$(CC) $(CURSES) $^ -o $@

clean:
	rm -f *.o
