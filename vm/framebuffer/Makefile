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

default:
	@echo Building VM w/Framebuffer support...
	@gcc -Wall -O3 -fomit-frame-pointer disassemble.c endian.c initial_image.c loader.c ngaro.c devices.c vm.c -DUSE_SDL -Wall `sdl-config --cflags --libs` -o retro-fb

clean:
	@rm -f retro-fb
