SDK = /Users/charleschilders/Desktop/sdk

default:
	cd source && $(SDK)/bin/adt -package -storetype pkcs12 -keystore ../cert.pfx ../../../bin/Retro.air application.xml .

key:
	$(SDK)/bin/adt -certificate -cn SelfSign -ou Dev -o "Retro" -c US 2048-RSA cert.pfx password

test:
	cd source && $(SDK)/adl application.xml

clean:
	@rm -f cert.pfx
