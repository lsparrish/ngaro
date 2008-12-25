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
.PHONY: applet app retroImage.java

default: clean
	@echo
	@echo Ngaro VM \- Java Implementation
	@echo ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	@echo Try \'make applet\' or \'make app\'
	@echo
	@echo You will need a retroImage.java in this directory for the build
	@echo to work. Check in the top level \'bin\' directory, or run
	@echo \'make image\' at the top level if you don\'t see one there.
	@echo ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	@echo

applet: retroImage.java
	@cat applet/Ngaro.top retroImage.java applet/Ngaro.bottom >Retro.java
	@javac Retro.java

app: retroImage.java
	@cat app/Ngaro.top retroImage.java app/Ngaro.bottom >RetroApp.java
	@javac RetroApp.java

clean:
	@rm -f *.java *.class
