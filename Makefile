CC=x86_64-w64-mingw32-gcc
CC_LEGACY=i686-w64-mingw32-gcc
CC_HOST=gcc
CFLAGS=-Wall -Wextra -Wno-sign-compare -ggdb -I.
LDFLAGS=-lws2_32

OL=ol
OWL_REPO=/home/kpm/nmojeprogramy/owl
OL_FASL=$(OWL_REPO)/fasl/ol.fasl
OL_SCM=$(OWL_REPO)/owl/ol.scm

packaged=ovm.exe ovm32.exe ol.exe ol-angry.exe ol32.exe hello.exe hello.fasl ovm.c README LICENSE

all: ovm.exe ovm32.exe ol.exe ol-angry.exe ol32.exe
ovm.exe: ovm.c
	$(CC) -DNOT_RT -o ovm.exe $(CFLAGS) ovm.c $(LDFLAGS)
ovm32.exe: ovm.c
	$(CC_LEGACY) -DNOT_RT -o ovm32.exe $(CFLAGS) ovm.c $(LDFLAGS)
ol.c: ovm.exe
	$(CC_HOST) makeol.c -o makeol
	./makeol $(OL_FASL) | cat - ovm.c > ol.c
ol.exe: ol.c
	$(CC) -DSILENT -o ol.exe $(CFLAGS) ol.c $(LDFLAGS)
ol32.exe: ol.c
	$(CC_LEGACY) -DSILENT -o ol32.exe $(CFLAGS) ol.c $(LDFLAGS)
ol-angry.exe: ol.c
	$(CC) -o ol-angry.exe $(CFLAGS) ol.c $(LDFLAGS)
test: ol-angry.exe
	$(MAKE) -C t
owl.zip: $(packaged)
	mkdir -p owl
	cp $(packaged) owl/
	zip -r owl.zip owl
	rm -r owl
hello.exe: ol32.exe t/hello.scm ovm.c
	wine ol32.exe -C ovm.c -x c -o - t/hello.scm | $(CC_LEGACY) -x c - -o hello.exe $(CFLAGS) $(LDFLAGS)
hello.fasl: ol.exe t/hello.scm
	wine ol.exe -x fasl -o hello.fasl t/hello.scm
clean:
	rm -f  ol.c *.exe makeol test.c ol.c hello.exe *.fasl owl.zip
pubcpy: all owl.zip
	yes | pubcpy ovm.exe
	yes | pubcpy ovm32.exe
	yes | pubcpy ol.exe
	yes | pubcpy ol-angry.exe
	yes | pubcpy ol32.exe
	yes | pubcpy owl.zip
