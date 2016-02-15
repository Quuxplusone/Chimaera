# vbcc needs this environment variable so it can find its config file.
export VBCC:=./vbcc

VBCCZ = ./vbcc/bin/vc +z -c99 -O1 -DZ_MACHINE -DSAVE_AND_RESTORE -Iz-machine/include
MAXES = \$$MAX_ARRAYS=1100 \
        \$$MAX_ZCODE_SIZE=9000 \
        \$$MAX_STATIC_DATA=60000

all: chimaera.z8

chimaera: chimaera.c chimaera.h
	gcc -ansi -pedantic -o $@ chimaera.c

chimaera.z8: z-machine/Main.inf z-machine/stubs.inf z-machine/runtime.inf z-machine/cstubs.s z-machine/chimaera.s
	cd z-machine ; inform -v8 -ew~S~X~D $(MAXES) Main.inf ../chimaera.z8

z-machine/chimaera.s: chimaera.c
	$(VBCCZ) -c $< -o $@ -module-name=chimaera

z-machine/cstubs.s: z-machine/cstubs.c
	$(VBCCZ) -c $< -o $@ -module-name=libc

clean:
	rm -f chimaera chimaera.z8
