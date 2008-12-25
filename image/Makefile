default:
	@../toka/toka cross.toka
	@../tools/fix-image retroImage
	@../toka/toka ../tools/image2js.toka retroImage >../vm/javascript/retroImage.js
	@../toka/toka ../tools/image2java.toka retroImage >../vm/java/retroImage.java
	@../toka/toka ../tools/image2c.toka retroImage >initial_image.c
	@cp initial_image.c ../vm/console
	@cp initial_image.c ../vm/framebuffer
	@rm -f initial_image.c
