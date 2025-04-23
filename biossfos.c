// vim: set et ts=4 sw=4:
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <poll.h>
#include "globals.h"


uint16_t dma = 0;

static uint16_t get_xa(void)
{
    return (cpu->registers->x << 8) | cpu->registers->a;
}

static void set_xa(uint16_t xa)
{
    cpu->registers->x = xa >> 8;
    cpu->registers->a = xa;
}

static void set_result(uint16_t xa, bool succeeded)
{
    set_xa(xa);
    if (succeeded)
        cpu->registers->p &= ~0x01;
    else
        cpu->registers->p |= 0x01;
}


/* ---- BIOS FUNCTIONS ------------------------------------------------------*/

void bios_conout()
{
    (void)write(1, &cpu->registers->a, 1);
}

void bios_conin()
{
    char c = 0;
    (void)read(0, &c, 1);
    if (c == '\n')
        c = '\r';
    if(c==0x18) {
        debug();
        return;
    }
    set_result(c, true);
}

void bios_const(void)
{
    if (!ctrlc)
    {
        uint8_t c=0;

        struct pollfd pollfd = {0, POLLIN, 0};
        poll(&pollfd, 1, 0);
        if (pollfd.revents & POLLIN)
        {
            (void)read(0,&c,1);
            if(c=='\n')
                c='\r';
            if(c==0x7F)
                c='\b';
            if(c==0x18) {
                debug();
                return;
            }
            cpu->registers->a = c;
            cpu->registers->p |= 0x01;
            cpu->registers->p &= ~0x02;
        }
        else {
            cpu->registers->a = 0;
            cpu->registers->p &= ~0x01;
            cpu->registers->p |= 0x02;
        }
    }
    else
    {
        cpu->registers->a = 3;
        cpu->registers->p |= 0x01;
        cpu->registers->p &= ~0x02;
        ctrlc = false;
    }
}

void bios_conputs(void)
{
    char* pp = ram;
    pp += get_xa();
    while (*pp++ != 0)
        (void)write(1, pp, 1);
}

void bios_conbyte(void)
{
    uint8_t c = cpu->registers->a;
    printf("%02X", c);
}

/* ---- SFOS FUNCTIONS ------------------------------------------------------*/

void sfos_c_printstr(void)
{
    bios_conputs();
}

void sfos_c_readstr(void)
{
    char* pp = ram;
    uint16_t xa = get_xa();
    pp += xa;

    char* start = pp;

    cm_on();
    char* buf = readline(NULL);
    cm_off();

    if (strcmp(buf, "DEBUG")==0)
    {
        debug();
        set_result(0, true);
        return;
    }
    uint8_t len = strlen(buf);
    pp ++;
    memcpy(pp, buf, 127);
    if (len < 127)
        *start = len;
    else
        *start = 127;
    set_result(xa, true);
}

static void sfos_setdma()
{
    dma = get_xa();
}


void sfos_entry()
{
    switch(cpu->registers->y) {
        case SFOS_C_READ:
            bios_conin();
            break;
        case SFOS_C_WRITE:
            bios_conout();
            break;
        case SFOS_C_STATUS:
            bios_const();
            break;
        case SFOS_C_PRINTSTR:
            sfos_c_printstr();
            break;
        case SFOS_C_READSTR:
            sfos_c_readstr();
            break;
        default:
            fatal("unimplimented [%d]", cpu->registers->y);
    }
}
