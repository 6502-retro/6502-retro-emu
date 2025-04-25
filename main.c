// vim: set et ts=4 sw=4:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <glob.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <stdarg.h>
#include <termios.h>
#include "globals.h"

bool flag_enter_debugger = false;
char* const* user_command_line = NULL;
FILE* sdimg;
uint16_t tpa;

void cm_off(void)
{
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~ICANON; 
    t.c_lflag &= ~ECHO;

    // Apply the new settings
    tcsetattr(STDIN_FILENO, TCSANOW, &t); 
}

void cm_on(void)
{
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= ICANON; 
    t.c_lflag |= ECHO;

    // Apply the new settings
    tcsetattr(STDIN_FILENO, TCSANOW, &t); 
}

void fatal(const char* message, ...)
{
    va_list ap;
    va_start(ap, message);
    fprintf(stderr, "fatal: ");
    vfprintf(stderr, message, ap);
    fprintf(stderr, "\n");
    cm_on();
    fclose(sdimg);
    exit(1);
}
static void syntax(void)
{
    printf("emu [<flags>] [command] [args]:\n");
    printf("  -h             this help\n");
    printf("  -d             enter debugger on startup\n");
    printf("  -t             enable instruction tracing on startup\n");
    printf(
        "If command is specified, a Unix file of that name will be loaded "
        "and\n");
    printf(
        "injected directly into memory (it's not loaded through the CCP).\n");
    printf("The first two arguments are mapped to the standard FCBs.\n");
    printf("\nCTRL+C will usually issue a CTRL+C to the program.");
    printf("\nCTRL+X will exit to the debugger.  If that doesn't work, try");
    printf("\ntyping `DEBUG` which can help when the emulator is in readline mode\n");
    exit(1);
}

static void parse_options(int argc, char* const* argv)
{
    for (;;)
    {
        switch (getopt(argc, argv, "hdt"))
        {
            case -1:
                goto end_of_flags;

            case 'd':
                flag_enter_debugger = true;
                break;

            case 't':
                tracing = true;
                break;

            case 'h':
                syntax();
                break;

            default:
                syntax();
        }
    }

end_of_flags:
    user_command_line = &argv[optind];
}

static void load_sd_image(void)
{
    printf("opening sdcard image: %s\n", user_command_line[0]);
    sdimg = fopen(user_command_line[0], "r+");
    if (!sdimg)
        fatal("couldn't open sdcard image: %s", strerror(errno));


    if (fseek(sdimg, 512, 0) == -1)
        fatal("Could not seek to os on SDCARD");


    printf("reading sdcard image: %s\n", user_command_line[0]);
    char* pp = ram;
    pp += 0xE000;
    if (fread(pp, 1, 0x2000, sdimg)==-1)
        fatal("Could not load OS");
}

int main(int argc, char* const* argv)
{
    cm_off();
    parse_options(argc, argv);

    emulator_init();

    load_sd_image();

    cpu->registers->pc = 0xEE03;    // magic cboot number.

    for (;;)
    {
        emulator_run();
    }
    cm_on();
    fclose(sdimg);
    return 0;
}
