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
.PHONY: vm image toka tools


default:
	@clear
	@echo For most people, a simple \'make vm\' will suffice. None of the
	@echo other options are required to build a working Retro system.
	@echo ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	@echo make vm
	@echo - Build Ngaro \(the virtual machine\), console version
	@echo
	@echo make fbvm
	@echo - Build Ngaro \(the virtual machine\), framebuffer version
	@echo
	@echo make image
	@echo - Rebuild the initial retroImage
	@echo - Also builds Toka and the additional tools
	@echo
	@echo make clean
	@echo - Remove temporary files, Ngaro binary
	@echo
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
tools:
	@cd tools && make
toka:
	@cd toka && make
image: toka tools
	@cd image && make
	@mv image/retroImage* bin
clean:
	@rm -f bin/retro
	@rm -f bin/retro-fast
	@rm -f bin/retro-fb
	@rm -f bin/retroImage.map
	@rm -f bin/retroImage
	@rm -f toka/toka
	@rm -f latest.tar.gz
	@rm -f tools/fix-image
	@cd vm/framebuffer && make clean
	@cd vm/java && make clean
vm:
	@cd vm/console && make
	@mv vm/console/retro bin
	@mv vm/console/retro-fast bin
fbvm:
	@cd vm/framebuffer && make
	@mv vm/framebuffer/retro-fb bin
dist:
	@git archive master | gzip -9 >latest.tar.gz
	@mkdir retro-10.latest
	@cd retro-10.latest && tar xvf ../latest.tar.gz
	@rm -r latest.tar.gz
	@tar cf latest.tar retro-10.latest
	@gzip -9 latest.tar
	@rm -rf retro-10.latest
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
