// vim: set et ts=4 sw=4:
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include "globals.h"

static uint16_t dma = 0;
static uint32_t* lba = 0;
static uint8_t drive;


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

void bios_cboot()
{
    printf("\nCOLD BOOT");
}

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
    if (c ==0x7F)
        c = 0x08;
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


void bios_setdma()
{
    uint16_t xa = get_xa();
    dma = xa;
    ram[0xF4] = cpu->registers->a;
    ram[0xF5] = cpu->registers->x;
}

void bios_setlba()
{
    uint8_t* pp = ram;
    uint16_t xa = get_xa();
    pp += xa;
    lba = (uint32_t*)pp;
    //printf("setlba: %08x from: %02x\n", *lba, xa);
    ram[0xF6] = cpu->registers->a;
    ram[0xF7] = cpu->registers->x;
}

void bios_sdread()
{
    char* pp = ram;
    pp += dma;
    //printf("sdread: %08x to: %02x\n", *lba, dma);
    uint32_t l = *lba;
    if (fseek(sdimg, l * 512, 0) == -1)
    {
        set_result(SEEK_ERROR, false);
        ram[BIOS_ERROR_CODE] = cpu->registers->a;
        return;
    }
    uint16_t len = fread(pp, 1, 512, sdimg);
    if (len == -1)
    {
        set_result(DRIVE_ERROR, false);
        ram[BIOS_ERROR_CODE] = cpu->registers->a;
        return;
    };
    set_result(OK, true);
}

void bios_sdwrite()
{
    char* pp = ram;
    pp += dma;
    //printf("sdwrite: %08x to: %02x\n", *lba, dma);
    uint32_t l = *lba;
    if (fseek(sdimg, l * 512, 0) == -1)
    {
        set_result(SEEK_ERROR, false);
        ram[BIOS_ERROR_CODE] = cpu->registers->a;
        return;
    }
    uint16_t len = fwrite(pp, 1, 512, sdimg);
    if (len == -1)
    {
        set_result(DRIVE_ERROR, false);
        ram[BIOS_ERROR_CODE] = cpu->registers->a;
        return;
    };
    set_result(OK, true);
}

