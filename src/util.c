#include "util.h"

#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int msleep(long msec)
{
    struct timespec ts;
    int res;
    if(msec < 0) {
        errno = EINVAL;
        return -1;
    }
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    do {
        res = nanosleep(&ts, &ts);
    } while(res && errno == EINTR);
    return res;
}

boolean strisnum(const char* str)
{
    unsigned char n;
    if(str[0] == '0' && strlen(str) > 1) return False;
    for(int i = 0; i < strlen(str); i++) {
        n = (unsigned char)(str[i] - '0');
        if(n > 9) return False;
    }
    return True;
}

unsigned int random_seed = 0;

unsigned int randint()
{
    random_seed ^= random_seed << 13;
    random_seed ^= random_seed >> 17;
    random_seed ^= random_seed << 5;
    return random_seed;
}
