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
#include "globals.h"

bool flag_enter_debugger = false;
char* const* user_command_line = NULL;



void fatal(const char* message, ...)
{
    va_list ap;
    va_start(ap, message);
    fprintf(stderr, "fatal: ");
    vfprintf(stderr, message, ap);
    fprintf(stderr, "\n");
    exit(1);
}
static void syntax(void)
{
    printf("emu [<flags>] [command] [args]:\n");
    printf("  -h             this help\n");
    printf("  -d             enter debugger on startup\n");
    printf("  -t             enable instruction tracing on startup\n");
    printf("  -m NUM         top of memory (by default, 0xff\n");
    printf("  -p DRIVE=PATH  map a drive to a path (by default, A=.)\n");
    printf(
        "If command is specified, a Unix file of that name will be loaded "
        "and\n");
    printf(
        "injected directly into memory (it's not loaded through the CCP).\n");
    printf("The first two arguments are mapped to the standard FCBs.\n");
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

static void read_file()
{
    printf("reading in file: %s\n", user_command_line[0]);

    int fd = open(user_command_line[0], O_RDONLY);
    if (fd == -1)
        fatal("couldn't open program: %s", strerror(errno));
    read(fd, &ram[TPA_BASE], SFOS_ADDRESS - TPA_BASE);
    close(fd);
}

int main(int argc, char* const* argv)
{
    parse_options(argc, argv);
    emulator_init();
    read_file();
    for (;;)
    {
        emulator_run();
    }

    return 0;
}
