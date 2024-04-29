#define _POSIX_C_SOURCE 199309L

#include "util.h"

#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

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

void modify_terminal()
{
    struct termios terminal_interface;
    /* int fcntl_flags = 0; */
    tcgetattr(STDIN_FILENO, &terminal_interface);
    terminal_interface.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &terminal_interface);
    /* fcntl_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
       fcntl(STDIN_FILENO, F_SETFL, fcntl_flags | O_NONBLOCK); */
}

void restore_old_terminal()
{
    struct termios terminal_interface;
    /* int fcntl_flags = 0; */
    tcgetattr(STDIN_FILENO, &terminal_interface);
    terminal_interface.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &terminal_interface);
    /* fcntl_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
       fcntl(STDIN_FILENO, F_SETFL, fcntl_flags & ~O_NONBLOCK); */
}

unsigned int random_seed = 0;

unsigned int randint()
{
    random_seed ^= random_seed << 13;
    random_seed ^= random_seed >> 17;
    random_seed ^= random_seed << 5;
    return random_seed;
}
