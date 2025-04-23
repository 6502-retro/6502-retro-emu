// vim: set sw=4 ts=4 et:
#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdbool.h>
#include "lib6502/lib6502.h"

#define TPA_BASE 0x0800
#define SFOS_ENTRY 0x200
#define SFOS_ADDRESS 0xFF00
#define EXIT_ADDRESS BIOS_WBOOT

#define BIOS_DISPATCH      0x200
#define BIOS_REBOOT        0x203
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
#define BIOS_ERROR_CODE    0x23c

enum {
    SFOS_S_RESET,           //0
    SFOS_C_READ,            //1
    SFOS_C_WRITE,           //2
    SFOS_C_PRINTSTR,        //3
    SFOS_C_READSTR,         //4
    SFOS_C_STATUS,          //5
    SFOS_D_GETSETDRIVE,     //6
    SFOS_D_CREATEFCB,       //7
    SFOS_D_PARSEFCB,        //8
    SFOS_D_FINDFIRST,       //9
    SFOS_D_FINDNEXT,        //10 A
    SFOS_D_MAKE,            //11 B
    SFOS_D_OPEN,            //12 C
    SFOS_D_CLOSE,           //13 D
    SFOS_D_SETDMA,          //14 E
    SFOS_D_READSEQBLOCK,    //15 F
    SFOS_D_WRITESEQBLOCK,   //16 10
    SFOS_D_READSEQBYTE,     //17 11
    SFOS_D_WRITESEQBYTE,    //18 12
    SFOS_D_SETLBA,          //19 13
    SFOS_D_READRAWBLOCK,    //20 14
    SFOS_D_WRITERAWBLOCK,   //21 15
    SFOS_S_SETTPA,          //22 16
    SFOS_S_GETTPA,          //23 17
};

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
    OUT_OF_MEMORY
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

extern void sfos_entry();

extern void bios_conin();
extern void bios_conout();
extern void bios_const();
extern void bios_conputs();
extern void bios_conbyte();

extern void bios_setdma();
extern void bios_setlba();
extern void bios_sdread();
extern void bios_sdwrite();

extern void cm_on();
extern void cm_off();
extern bool ctrlc;
#endif

