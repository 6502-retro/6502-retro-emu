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

int main(void){
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= ICANON; 
    t.c_lflag |= ECHO;

    // Apply the new settings
    tcsetattr(STDIN_FILENO, TCSANOW, &t); 
}
