CC=x86_64-w64-mingw32-gcc
CC_HOST=gcc
CFLAGS=-ggdb -I.
LDFLAGS=-lws2_32

OL=ol
OWL_REPO=/home/kpm/nmojeprogramy/owl
OL_FASL=$(OWL_REPO)/fasl/ol.fasl
OL_SCM=$(OWL_REPO)/owl/ol.scm

all: ovm.exe ol.exe ol-angry.exe
ovm.exe: ovm.c
	$(CC) -DNOT_RT -o ovm.exe $(CFLAGS) ovm.c $(LDFLAGS)
ol.c: ovm.exe
	$(CC_HOST) makeol.c -o makeol
	./makeol $(OL_FASL) | cat - ovm.c > ol.c
ol.exe: ol.c
	$(CC) -DSILENT -o ol.exe $(CFLAGS) ol.c $(LDFLAGS)
ol-angry.exe: ol.c
	$(CC) -o ol-angry.exe $(CFLAGS) ol.c $(LDFLAGS)
test: ol.exe
	$(MAKE) -C t
clean:
	rm -f  ol.c *.exe makeol test.c ol.c
pubcpy: all
	yes | pubcpy ovm.exe
	yes | pubcpy ol.exe
	yes | pubcpy ol-angry.exe
