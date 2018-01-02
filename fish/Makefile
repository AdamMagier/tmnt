all: fish

# Note that you must be superuser to run the emulated version of the
# program.

fish_emulated: fish.o blink.o tmntemulate.o tmntsupport.o
	gcc -nostdlib -lc -g -o fish_emulated fish.o blink.o tmntemulate.o tmntsupport.o

fish: fish.exe
	../elfconvert fish.exe
	mv fish.exe.converted fish

fish.exe: fish.o blink.o tmntsupport.o tmntsyscall.o
	gcc -nostdlib -g -o fish.exe fish.o blink.o tmntsyscall.o tmntsupport.o

%.o: %.S
	gcc -nostdlib -c -Wall -g -D_USERLAND -D_ASM -o $@ $<

%.o: %.c
	gcc -nostdlib -Wall -c -g -o $@ $<

clean::
	rm -f *.o *~
clear: clean
	rm -f fish fish.exe fish_emulated
