/*
    flowstatd - Netflow statistics daemon
    Copyright (C) 2012 Kudo Chien <ckchien@gmail.com>

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
#include "flowstatd.h"

#define MAX_MONITOR_FD_COUNT	    2

#ifdef USE_KQUEUE
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#else
#include <sys/select.h>
#endif


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

int kqueueInitImpl(MultiplexerFunc_t *this);
int kqueueUnInitImpl(MultiplexerFunc_t *this);
int kqueueIsActiveImpl(MultiplexerFunc_t *this, int fd);
int kqueueAddToListImpl(MultiplexerFunc_t *this, int fd);
int kqueueRemoveFromListImpl(MultiplexerFunc_t *this, int fd);
int kqueueWaitImpl(MultiplexerFunc_t *this);
MultiplexerFunc_t *kqueueNewMultiplexer();
int kqueueFreeMultiplexer(MultiplexerFunc_t *this);

#endif

typedef struct _selectMultiplexer_t selectMultiplexer_t;
struct _selectMultiplexer_t {
    MultiplexerFunc_t funcs;

    fd_set evlist;
    fd_set chlist;
    unsigned int maxFd;
};

int selectInitImpl(MultiplexerFunc_t *this);
int selectUnInitImpl(MultiplexerFunc_t *this);
int selectIsActiveImpl(MultiplexerFunc_t *this, int fd);
int selectAddToListImpl(MultiplexerFunc_t *this, int fd);
int selectRemoveFromListImpl(MultiplexerFunc_t *this, int fd);
int selectWaitImpl(MultiplexerFunc_t *this);
MultiplexerFunc_t *selectNewMultiplexer();
int selectFreeMultiplexer(MultiplexerFunc_t *this);

#endif
