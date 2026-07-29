# Makefile configuration

CPU             = msp430g2553

CFLAGS          = -mmcu=${CPU} -Os -I../h -I/opt/ti/msp430-gcc/include/

LDFLAGS         = -L../lib -L/opt/ti/msp430-gcc/include

CC              = msp430-elf-gcc
AS              = msp430-elf-gcc -mmcu=${CPU} -c

all: wakedemo.elf

wakedemo.elf: wakedemo.o buzzer.o wdt_handler.o
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^ -lTimer -lLcd

load: wakedemo.elf
	mspdebug ${MSPLOADER_OPTION} "prog $^"

wakedemo.o: buzzer.h
buzzer.o: buzzer.h

clean:
	rm -f *.o *.elf
