include $(GOROOT)/src/Make.$(GOARCH)

TARG=ngaro
GOFILES=ngaro.go

gonga: ./$(TARG).a gonga.go image.go
	$(GC) -o gonga.$O image.go gonga.go
	$(LD) -o $@ gonga.$O

./$(TARG).a: ./_obj/$(TARG).a
	cp ./_obj/$(TARG).a .

include $(GOROOT)/src/Make.pkg
