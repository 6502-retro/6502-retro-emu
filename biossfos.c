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


/* forward declarations */
static void sfos_d_findnext(void);


typedef struct {
    uint8_t isloggedin;
    uint16_t maxdir;
} logvec;

logvec logvecs[8] = {0};

typedef struct {
    uint8_t     DRIVE;
    char        NAME[11];
    uint16_t    LOAD;
    uint8_t     SC;
    uint8_t     FILE_NUM;
    uint8_t     ATTRIB;
    uint16_t    EXEC;
    uint8_t     Z1;
    uint8_t     Z2;
    uint32_t    SIZE;               // Note we only need 24 bytes for the size.
    uint8_t     CR;
    uint8_t     DS;
} _fcb;

static uint16_t dma = 0;
static uint32_t lba = 0;
static uint8_t drive = 0;
static uint8_t current_dirpos = 0;
static char* bufptr = ram;
static char* bufend = ram;

uint8_t sdbuf[512];

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
    uint16_t xa = get_xa();
    char* pp = ram + xa;
    do
    {
        (void)write(1, pp, 1);
    } while (*pp++ != 0);
}

void bios_conbyte(void)
{
    uint8_t c = cpu->registers->a;
    printf("%02X", c);
}

void bios_setdma()
{
    dma = get_xa();
}

void bios_setlba()
{
    uint8_t* pp = ram;
    pp += get_xa();
    lba = (uint32_t)*pp;
}

void bios_sdread()
{
    char* pp = ram;
    pp += dma;
    if (fseek(sdimg, lba*512, 0) == -1)
    {
        set_result(DRIVE_ERROR, false);
        ram[BIOS_ERROR_CODE] = cpu->registers->a;
        return;
    }
    if (fread(pp, 1, 512, sdimg)==-1)
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
    fseek(sdimg, lba*512, 0);
    (void)fwrite(pp, 1, 512, sdimg);
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

static void login_drive(bool rescan)
{
    if(!rescan)
    {
        if (logvecs[drive-1].isloggedin)
            return;
    }
    /* a rescan wasn't requested but drive is not logged in so log in anyway */
    lba = ((drive-1)*0x10)+0x80;
    uint32_t w = lba * 512;
    for (uint8_t i=0; i< 16; i++)       /* read in 16 sectors*/
    {
        if (fseek(sdimg, w, 0)==-1)
            fatal("Failed to seek to LBA");

        if (fread(sdbuf, 1, 512, sdimg) == -1)
            fatal("Failed to read from sdcard");

        _fcb* dirent = (_fcb*)sdbuf;
        while (dirent < (_fcb*)sdbuf+16)
        {
            if (dirent->ATTRIB != 0xE5)
                logvecs[drive-1].maxdir ++;
            dirent += 1;
        }

        w += 512;
    }
}

static void sfos_d_getsetdrive(void)
{
    uint8_t d = cpu->registers->a;
    if (d == 0xFF) {
        set_result(drive, true);
        return;
    }
    if( (d<1) || (d>8)) {
        set_result(DRIVE_ERROR, false);
        return;
    }

    drive = d;
    login_drive(false);
    set_result(OK, true);
}


static void replace_wc(char* buff, size_t len)
{
    while (len > 0) {
        if (*buff == '?')
            *buff = ' ';
        len --;
        buff++;
    }
}

bool match(char* src, char* dst, uint8_t len)
{
    while(len > 0)
    {
        if (*src != *dst)
            if (*dst != '?')
                return false;
        len--;
        src++;
        dst++;
    }
    return true;
}

static void sfos_d_findfirst(void)
{
    current_dirpos = 0;
    sfos_d_findnext();
}

static void sfos_d_findnext(void)
{
    uint16_t xa = get_xa();
    uint8_t* pp = ram;
    pp += xa;
    _fcb* search_fcb = (_fcb*)pp;

    lba = ((drive-1)*0x10)+0x80;
    uint32_t w = lba * 512;

    for (uint8_t i=0; i< 16; i++)
    {
        if (fseek(sdimg, w, 0)==-1)
            fatal("Failed to seek to LBA");

        if (fread(sdbuf, 1, 512, sdimg) == -1)
        fatal("Failed to read from sdcard");

        _fcb* dirent = (_fcb*)sdbuf;
        dirent += current_dirpos;

        while (dirent < (_fcb*)sdbuf+16)
        {
            if (match(dirent->NAME, search_fcb->NAME, 11))
            {
                memcpy((char*)search_fcb, (char*)dirent, 27);
                replace_wc((char*)search_fcb, 12);
                set_result(OK, true);
                current_dirpos += 1;
                return;
            }
            else
            {
                current_dirpos += 1;
            }
            dirent += 1;
        }
        w+=512;
    }
    set_result(FILE_NOT_FOUND, false);
}

void sfos_d_parsefcb(void)
{
    uint16_t xa = get_xa();
    char* pp = ram;
    pp+= xa;
    char* cmd = (char*)pp;

    char* pf = ram;
    pf+=dma;
    _fcb* f = (_fcb*)pf;

    char c;

    // populate fcb f with a filename and drive from cmd.
    if(cmd[1] == ':')
    {
        f->DRIVE = 'A' - cmd[0];
        cmd+=2;
    } else {
        f->DRIVE = drive;
    }
    uint8_t i = 0;
    do
    {
        c = *cmd++;
        if ((c == '.') || (c=='?') ||(c=='*') || (c==' ') || (c=='\0')) {
            cmd --;
            c = ' ';
        }

        f->NAME[i] = toupper(c);
        i++;
    } while (i < 8);
    cmd ++;
    i = 0;
    do {
        c = *cmd++;
        if ((c == '.') || (c=='?') ||(c=='*') || (c==' ') || (c=='\0'))
        {
            cmd --;
            c = ' ';
        }
        f->NAME[i+8] = toupper(c);
        i++;
    } while (i < 3);

    set_result(OK, true);
}

void sfos_d_open(void)
{
    uint16_t xa = get_xa();
    uint8_t* pp = ram;
    pp += xa;
    _fcb* f = (_fcb*)pp;

    uint8_t drive = f->DRIVE;
    if (drive != 0)
    {
        cpu->registers->a = drive;
        sfos_d_getsetdrive();
        if (cpu->registers->p & 1 == 1)
        {
            set_result(0, false);
            return;
        }
    }

    cpu->registers->a = xa & 0xFF;
    cpu->registers->x = xa >> 8;
    sfos_d_findfirst();
    if (cpu->registers->p & 1 == 1)
    {
        set_result(FILE_NOT_FOUND, false);
        return;
    }

    dma = f->LOAD;
    f->CR = 0;
    set_result(OK, true);
}

void sfos_d_setdma(void)
{
    dma = get_xa();

    bufend = ram;
    bufptr = ram;

    bufptr += dma;
    bufend += dma + 512;
}

void sfos_d_readseqblock(void)
{
    uint16_t xa = get_xa();
    uint8_t* pp = ram;
    pp += xa;
    _fcb* f = (_fcb*)pp;
    lba = 0 | ( (drive<<16) | (f->FILE_NUM<<8) | (f->CR) );
    f->CR++;

    if (f->CR == 0)
    {
        set_result(FILE_MAX_REACHED, false);
        ram[BIOS_ERROR_CODE] = cpu->registers->a;
        return;
    } else if (f->CR > f->SC)
    {
        set_result(FILE_EOF, false);
        return;
    }
    else
    {
        bios_sdread();
        if (cpu->registers->p & 1 == 1)
            set_result(0, false);

        set_result(OK, true);
    }
}

void sfos_d_readseqbyte(void)
{
    uint16_t xa = get_xa();
    uint8_t* pp = ram;
    pp += xa;
    _fcb* f = (_fcb*)pp;
    uint8_t c = *bufptr++;
    if (bufptr == bufend)
    {
        bufptr = ram;
        bufptr += dma;
        sfos_d_readseqblock();
        if(cpu->registers->p & 1 == 1)
        {
            ram[BIOS_ERROR_CODE] = cpu->registers->a;
            return;
        }
     }
    f->SIZE --;
    if (f->SIZE == 0) 
        set_result(FILE_EOF, false);
    else
        set_result(c, true);
}

static void sfos_s_gettpa(void)
{
    set_result(tpa, true);
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
        case SFOS_D_GETSETDRIVE:
            sfos_d_getsetdrive();
            break;
        case SFOS_D_FINDFIRST:
            sfos_d_findfirst();
            break;
        case SFOS_D_FINDNEXT:
            sfos_d_findnext();
            break;
        case SFOS_D_PARSEFCB:
            sfos_d_parsefcb();
            break;
        case SFOS_D_OPEN:
            sfos_d_open();
            break;
        case SFOS_D_SETDMA:
            sfos_d_setdma();
            break;
        case SFOS_D_READSEQBLOCK:
            sfos_d_readseqblock();
            break;
        case SFOS_D_READSEQBYTE:
            sfos_d_readseqbyte();
            break;
        case SFOS_S_GETTPA:
            sfos_s_gettpa();
            break;

        default:
            showregs();
            fatal("Unimplimented SFOS Call");
    }
}
