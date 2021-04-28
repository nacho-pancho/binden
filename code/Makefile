
ifdef DEBUG
CFLAGS  := -g -Wall -O0
else
CFLAGS  := -pg -Wall -O3 -msse3
endif
OBJECTS :=  image.o  ascmat.o  dudelib.o 
INCFLAGS = -I../dist
LIBS    := -lm
TARGETS := dude_bsc dude_pe threshold mktpl

all:  $(TARGETS)

threshold: threshold.o $(OBJECTS) Makefile
	cc -o $@ $< image.o  $(LDFLAGS) $(LIBS)

dude%: dude%.o $(OBJECTS) Makefile
	cc -o $@ $< $(OBJECTS) $(LDFLAGS) $(LIBS)

.c.o:
	cc -o $@ -c $(CFLAGS) $(INCFLAGS) $<

clean:
	rm -f *.o $(TARGETS)

mktpl: mktpl.o image.o ascmat.o
	cc -o $@ $< image.o ascmat.o $(LDFLAGS) $(LIBS)

