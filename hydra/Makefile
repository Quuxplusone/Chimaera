# vbcc needs this environment variable so it can find its config file.
export VBCC:=./vbcc

VBCCZ = ./vbcc/bin/vc +z -c99 -O1 -DZ_MACHINE -DSAVE_AND_RESTORE -Iz-machine/include
MAXES = \$$MAX_ARRAYS=700 \
        \$$MAX_ZCODE_SIZE=38000 \
        \$$MAX_STATIC_DATA=48000

SOURCES = distance.c main.c map.c npcs.c rooms.c util.c vocab.c
S_SOURCES := $(patsubst %.c,z-machine/%.s,$(SOURCES))

all: hydra

hydra: $(SOURCES)
	gcc -std=c99 -O3 -pedantic -W -Wall -Wextra \
		-Wno-switch -Wno-unused-result \
		$^ -o $@

hydra.z8: z-machine/Main.inf z-machine/stubs.inf z-machine/cstubs.s $(S_SOURCES)
	cd z-machine ; inform -v8 -ew~S~X~D $(MAXES) Main.inf ../hydra.z8

z-machine/cstubs.s: z-machine/cstubs.c
	$(VBCCZ) -c $< -o $@ -module-name=libc

z-machine/%.s: %.c
	$(VBCCZ) -c $< -o $@ -module-name=$(patsubst %.c,%,$<)

clean:
	rm -f hydra hydra.z8 $(S_SOURCES)
