/*
    flowd - Netflow statistics daemon
    Copyright (C) 2011 Kudo Chien <ckchien@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    Optionally you can also view the license at <http://www.gnu.org/licenses/>.
*/

#ifndef _MULTIPLEX_H
#define _MULTIPLEX_H

#define MAX_MONITOR_FD_COUNT	    2

#undef USE_KQUEUE	    // Only avaliable on FreeBSD or use select() in default

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


#ifdef USE_KQUEUE

#define NewMultiplexer(...) kqueueNewMultiplexer(__VA_ARGS__)
#define FreeMultiplexer(...) kqueueFreeMultiplexer(__VA_ARGS__)

#else

#define NewMultiplexer(...) selectNewMultiplexer(__VA_ARGS__)
#define FreeMultiplexer(...) selectFreeMultiplexer(__VA_ARGS__)

#endif


typedef struct _MultiplexerFunc_t MultiplexerFunc_t;
struct _MultiplexerFunc_t {
    int (*Init)(MultiplexerFunc_t *this);
    int (*UnInit)(MultiplexerFunc_t *this);

    int (*IsActive)(MultiplexerFunc_t *this, int fd);
    int (*AddToList)(MultiplexerFunc_t *this, int fd);
    int (*RemoveFromList)(MultiplexerFunc_t *this, int fd);
    int (*Wait)(MultiplexerFunc_t *this);
};

#ifdef USE_KQUEUE
typedef struct _kqueueMultiplexer_t {
    MultiplexerFunc_t funcs;

    int kqFd;
    struct kevent evlist[MAX_MONITOR_FD_COUNT];
    struct kevent chlist[MAX_MONITOR_FD_COUNT];
    unsigned int monitorFdCount;
} kqueueMultiplexer_t;

#endif

typedef struct _selectMultiplexer_t selectMultiplexer_t;
struct _selectMultiplexer_t {
    MultiplexerFunc_t funcs;

    fd_set evlist;
    fd_set chlist;
    unsigned int maxFd;
};


#endif
