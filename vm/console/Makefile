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

default:
	@echo Building VM for text console...
	@gcc -Wall -O3 -fomit-frame-pointer $(CURSES) devices.c disassemble.c endian.c initial_image.c loader.c ngaro.c vm.c -o retro
	@gcc -Wall -O3 -fomit-frame-pointer $(CURSES) devices.c disassemble.c endian.c initial_image.c loader.c ngaro.c vm_mat.c -DMAT -o retro-fast
