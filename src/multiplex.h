#ifndef _MULTIPLEX_H
#define _MULTIPLEX_H

#define USE_KQUEUE	    // Only avaliable on FreeBSD or use select() in default

#ifdef USE_KQUEUE
#define MULTIPLEXOR_NAME    kqueue
#else
#define MULTIPLEXOR_NAME    select
#endif


#ifdef USE_KQUEUE
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#else
#include <sys/select.h>
#endif


#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})


#define NewMultiplexor(name, ...) name##NewMultiplexor(__VA_ARGS__)
#define FreeMultiplexor(name, ...) name##FreeMultiplexor(__VA_ARGS__)

typedef struct _MultiplexorFunc_t MultiplexorFunc_t;
struct _MultiplexorFunc_t {
    int (*Init)(MultiplexorFunc_t *this);
    int (*UnInit)(MultiplexorFunc_t *this);

    int (*IsActive)(MultiplexorFunc_t *this, int fd);
    int (*AddToList)(MultiplexorFunc_t *this, int fd);
    int (*RemoveFromList)(MultiplexorFunc_t *this, int fd);
    int (*Wait)(MultiplexorFunc_t *this);
};

#ifdef USE_KQUEUE
#define MAX_KQ_FD_COUNT	    2

typedef struct _kqueueMultiplexor_t {
    MultiplexorFunc_t funcs;

    int kqFd;
    struct kevent evlist[MAX_KQ_FD_COUNT];
    struct kevent chlist[MAX_KQ_FD_COUNT];
    unsigned int monitorFdCount;
} kqueueMultiplexor_t;

#endif

typedef struct _selectMultiplexor_t selectMultiplexor_t;
struct _selectMultiplexor_t {
    MultiplexorFunc_t funcs;

    fd_set rfdList;
    unsigned int monitorFdCount;
};


#endif
