#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

typedef unsigned char BOOL;
#define TRUE ((BOOL)(1==1))
#define FALSE ((BOOL)(!TRUE))

#define DEBUG FALSE

#define LOG(msg) if(DEBUG){printf(msg);}

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#define COLOR "\033[38;5;%dm"

int msleep(long msec);

void modify_terminal();
void restore_old_terminal();

inline int kbhit()
{
    char ch = getchar();
    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

extern unsigned int random_seed;

inline void clear_screen()
{
    printf("\033[2J");
}

inline void hide_cursor()
{
    printf("\033[?25l");
}

inline void show_cursor()
{
    printf("\033[?25h");
}

inline void reset_cursor()
{
    printf("\033[0;0H");
}

inline void reset_color()
{
    printf("\033[0m");
}

inline void gotoxy(int x, int y)
{
    printf("\033[%d;%dH", y, x);
}

inline void set_seed(unsigned int seed)
{
    random_seed = seed;
}

unsigned int randint();

inline unsigned int randrange(int min, int max)
{
    return (randint() % (max + 1 - min) + min);
}

#endif /* UTIL_H */
