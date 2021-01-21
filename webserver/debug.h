#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdio.h>

/* If we haven't passed -DDEBUG=1 to gcc, then this will be set to 0: */
#ifndef DEBUG
#define DEBUG 0
#endif

#define LOGP(str) \
    do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): %s", __FILE__, \
            __LINE__, __func__, str); } while (0)

#define LOG(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
            __LINE__, __func__, __VA_ARGS__); } while (0)

#endif
