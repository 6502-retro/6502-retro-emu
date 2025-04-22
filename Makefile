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

test.out:
	ca65 -t none --cpu 65C02 -o test.o test.s -ltest.lst
	ld65 -t none test.o -o test.out

clean veryclean:
	$(RM) emu
	$(RM) test.out
