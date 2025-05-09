// vim: set sw=4 ts=4 et:
#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdbool.h>
#include "lib6502/lib6502.h"

#define TPA_BASE 0x0800
#define SFOS_ENTRY 0x200
#define SFOS_ADDRESS 0x10000
#define EXIT_ADDRESS 0x0000

#define BIOS_DISPATCH      0x200
#define BIOS_CBOOT         0x203
#define BIOS_WBOOT         0x206
#define BIOS_CONOUT        0x209
#define BIOS_CONIN         0x20c
#define BIOS_CONST         0x20f
#define BIOS_CONPUTS       0x212
#define BIOS_CONBYTE       0x215
#define BIOS_SETDMA        0x218
#define BIOS_SETLBA        0x21b
#define BIOS_SDREAD        0x21e
#define BIOS_SDWRITE       0x221
#define BIOS_CONBEEP       0x224
#define BIOS_SN_START      0x227
#define BIOS_SN_SILENCE    0x22a
#define BIOS_SN_STOP       0x22d
#define BIOS_SN_SEND       0x230
#define BIOS_LED_ON        0x233
#define BIOS_LED_OFF       0x236
#define BIOS_GET_BUTTON    0x239
#define BIOS_DELAY_MS      0x23c
#define BIOS_ERROR_CODE    0x23f

enum{
    OK,
    FILE_NOT_FOUND,
    FILE_EXISTS,
    FILE_MAX_REACHED,
    FILE_EOF,
    END_OF_DIR,
    DRIVE_ERROR,
    DRIVE_FULL,
    PARSE_ERROR,
    OUT_OF_MEMORY,
    SEEK_ERROR
};

extern M6502* cpu;
extern uint8_t ram[0x10000];
extern uint16_t himem;
extern bool tracing;
extern bool flag_enter_debugger;
extern char* const* user_command_line;

extern FILE* sdimg;

extern void fatal(const char* message, ...);
extern void emulator_init(void);
extern void emulator_run(void);
extern void showregs(void);
extern void debug(void);

extern void bios_cboot();
extern void bios_wboot();
extern void bios_conin();
extern void bios_conout();
extern void bios_const();
extern void bios_conbyte();

extern void bios_setdma();
extern void bios_setlba();
extern void bios_sdread();
extern void bios_sdwrite();

extern void cm_on();
extern void cm_off();
extern bool ctrlc;
extern uint16_t tpa;
#endif

