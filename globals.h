#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdbool.h>
#include "lib6502/lib6502.h"

#define TPA_BASE 0x0800
#define SFOS_ENTRY 0x200
#define SFOS_ADDRESS 0xFF00
#define EXIT_ADDRESS 0x206

#define	BIOS_DISPATCH	0x200
#define	BIOS_REBOOT	BIOS_DISPATCH + 3
#define	BIOS_WBOOT	BIOS_REBOOT + 3
#define	BIOS_CONOUT	BIOS_WBOOT + 3
#define	BIOS_CONIN	BIOS_CONOUT + 3
#define	BIOS_CONST	BIOS_CONIN + 3
#define	BIOS_CONPUTS	BIOS_CONST + 3
#define	BIOS_CONBYTE	BIOS_CONPUTS + 3
#define	BIOS_CONBEEP	BIOS_CONBYTE + 3
#define	BIOS_SN_START	BIOS_CONBEEP + 3
#define	BIOS_SN_SILENCE	BIOS_SN_START + 3
#define	BIOS_SN_STOP	BIOS_SN_SILENCE + 3
#define	BIOS_SN_SEND	BIOS_SN_STOP + 3
#define	BIOS_LED_ON	BIOS_SN_SEND + 3
#define	BIOS_LED_OFF	BIOS_LED_ON + 3
#define	BIOS_GET_BUTTON BIOS_LED_OFF + 3
#define	BIOS_ERROR_CODE BIOS_GET_BUTTON + 3

enum {
	S_RESET,
	C_READ,
	C_WRITE,
	C_PRINTSTR,
	C_READSTR,
	C_STATUS,
};

extern M6502* cpu;
extern uint8_t ram[0x10000];
extern uint16_t himem;
extern bool tracing;
extern bool flag_enter_debugger;
extern char* const* user_command_line;


extern void fatal(const char* message, ...);
extern void emulator_init(void);
extern void emulator_run(void);
extern void showregs(void);

extern void sfos_entry();

extern void bios_conin();
extern void bios_conout();
extern void bios_const();

#endif

