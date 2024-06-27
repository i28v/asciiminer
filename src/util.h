#ifndef UTIL_H
#define UTIL_H

#ifndef _POSIX_C_SOURCE

#define _POSIX_C_SOURCE 199309L

#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>

#if defined USE_STDBOOL_H

#include <stdbool.h>

#else 

typedef unsigned char boolean;
#define True ((boolean)(1==1))
#define False ((boolean)(!True))

#endif 

#define DEBUG FALSE

#define LOG(msg) if(DEBUG){printf(msg);}


#ifndef STDIN_FILENO

#define STDIN_FILENO 0


#endif

typedef unsigned char type;

#define DEFAULT ((type)0)
#define NONE ((type)255)


int msleep(long msec);

boolean strisnum(const char* str);

#if defined USE_INLINING

inline boolean file_exists(const char* fn)
{
    return (access(fn, F_OK) == 0) ? True : False;
}

#else

#define file_exists(fn) ((access(fn, F_OK) == 0) ? True : False)

#endif

extern unsigned int random_seed;

unsigned int randint();

#if defined USE_INLINING

inline void set_seed(unsigned int seed)
{
    random_seed = seed;
}

inline void randrange(int min, int max)
{
    return (randint() % (max + 1 - min) + min);
}

#else 

#define set_seed(seed) random_seed = seed;

#define randrange(min, max) (randint() % (max + 1 - min) + min)

#endif

#endif /* UTIL_H */
