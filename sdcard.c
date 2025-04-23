#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

uint8_t sdbuf[512];

int main()
{
    FILE* fd = fopen("sdcard.img", "r+");
    fseek(fd, 512*0x80, 0);
    fread(sdbuf, 1, 512, fd);

    for (uint8_t i=0; i<16; i++)
    {
        printf("%02x ", sdbuf[i]);
    }
    return 0;

}
