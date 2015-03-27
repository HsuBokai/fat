CC=gcc
CFLAGS= -g -Wall
IDIR =./include

LDIR =./lib

#_DEPS = cmdline.h utils.h
#DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

#_OBJ = main.o cmdline.o utils.o
#OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

OBJ := $(patsubst %.c, %.o, $(wildcard *.c))

TARGET = main.exe

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -I$(IDIR) -c -o $@ $<

.PHONY: clean

clean:
	rm -f $(OBJ) *~ core $(TARGET)
