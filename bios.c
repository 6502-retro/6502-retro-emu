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


void sfos_c_write()
{
    (void)write(1, &cpu->registers->a, 1);
}

void sfos_c_read()
{
    char c = 0;
    (void)read(0, &c, 1);
    if (c == '\n')
        c = '\r';
    set_result(c, true);
}


void sfos_c_status(void)
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

void sfos_c_printstr(void)
{
    char* pp = ram;
    pp += get_xa();
    printf("%s", pp);
    set_result(0, true);
}

void sfos_c_readstr(void)
{
    char* pp = ram;
    uint16_t xa = get_xa();
    pp += xa;

    char* start = pp;

    char* buf = readline(NULL);
    uint8_t len = strlen(buf);
    pp ++;
    memcpy(pp, buf, 127);
    if (len < 127)
        *start = len;
    else
        *start = 127;
    set_result(xa, true);
}

void sfos_entry()
{
    switch(cpu->registers->y) {
        case C_READ:
            sfos_c_read();
            break;
        case C_WRITE:
            sfos_c_write();
            break;
        case C_PRINTSTR:
            sfos_c_printstr();
            break;
        case C_READSTR:
            sfos_c_readstr();
            break;
        default:
            fatal("unimplimented [%02x]", cpu->registers->y);
    }
}
