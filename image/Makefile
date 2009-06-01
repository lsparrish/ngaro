default:
	@../toka/toka cross.toka
	@../tools/fix-image retroImage
	@cd ../tools && ../toka/toka image2js.toka ../image/retroImage >../vm/javascript/retroImage.js
	@cd ../tools && ../toka/toka image2c.toka ../image/retroImage >../vm/console/initial_image.c
	@cd ../tools && ../toka/toka image2c.toka ../image/retroImage >../vm/framebuffer/initial_image.c
	@cd ../tools && ../toka/toka image2java.toka ../image/retroImage >../vm/java/retroImage.java
	@cd ../tools && ../toka/toka image2java.toka ../image/retroImage >../vm/dotnet/retro_middle
