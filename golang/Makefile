include $(GOROOT)/src/Make.$(GOARCH)

TARG=ngaro
GOFILES=ngaro.go

gonga: _obj/$(TARG).a gonga.go
	$(GC) -I_obj gonga.go
	$(LD) -L_obj -o $@ gonga.$O

include $(GOROOT)/src/Make.pkg
