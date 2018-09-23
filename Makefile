PREFIX=../../
CC=$(PREFIX)/bin/sh3eb-elf-gcc.exe
MKG3A=$(PREFIX)/bin/mkg3a.exe
RM=del
CFLAGS=-m3 -mb -O3 -mhitachi -Wall -nostdlib  -I$(PREFIX)/include -lfxcg -lgcc -L$(PREFIX)/lib -std=c99
MKG3AFLAGS=-i uns:unselected.bmp -i sel:selected.bmp -n basic:Bomberman
LDFLAGS=$(CFLAGS) -T../common/prizm.ld -Wl,-static
CSOURCES=src/main.c src/world.c src/bomberman.c src/bombs.c src/draw.c src/astar.c src/key.c src/menu.c
OBJECTS=$(CSOURCES:.c=.o)
BIN=Bomberman.bin
ADDIN=$(BIN:.bin=.g3a)

all: $(ADDIN)
	
$(BIN): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	
$(ADDIN): $(BIN)
	$(MKG3A) $(MKG3AFLAGS) $< $@

.s.o:
	$(CC) -c $(CFLAGS) $< -o $@
	
.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(OBJECTS) $(BIN) $(ADDIN)
	
	
