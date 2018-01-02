CFLAGS += -Wall -nostdlib -ffreestanding
LDFLAGS += -nostdlib -ffreestanding
CC = gcc

ALL: cat grep hello ls pingpong counter shell sigtest testprint syserr

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -Wall -o $@ $<

%.exe: tmnt%.o tmntsyscall.o tmntsupport.o
	$(CC) $(LDFLAGS) -o $@ $^

%: %.exe
	../elfconvert $<
	mv $<.converted to_fsdir/$@

clean::
	rm -f *~ *.o

clear: clean
	rm -f *.converted
	rm -f *.exe
	rm -f to_fsdir/*
