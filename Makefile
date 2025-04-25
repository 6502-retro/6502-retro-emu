CC      = gcc
CFLAGS  = -g
RM      = rm -f

SRCS = \
       lib6502/lib6502.c \
       emulator.c \
       bios.c \
       main.c

default: all

all: emu

emu: $(SRCS)
	$(CC) $(CFLAGS) -o emu $^ -lreadline

clean veryclean:
	$(RM) emu
