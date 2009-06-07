default:
	@../toka/toka cross.toka
	@../tools/fix-image retroImage
	@cd ../tools && ../toka/toka image2js.toka ../image/retroImage >../vm/javascript/retroImage.js
	@cd ../tools && ../toka/toka image2c_console.toka ../image/retroImage >../vm/console/initial_image.c
	@cd ../tools && ../toka/toka image2c_fbvm.toka ../image/retroImage >../vm/framebuffer/initial_image.c
	@cd ../tools && ../toka/toka image2c_fbvm.toka ../image/retroImage >../vm/console_fast/initial_image.c
	@cd ../tools && ../toka/toka image2java.toka ../image/retroImage >../vm/java/retro_middle
	@cd ../tools && ../toka/toka image2java.toka ../image/retroImage >../vm/dotnet/retro_middle
