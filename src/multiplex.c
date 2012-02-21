/*
    flowd - Netflow statistics daemon
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "multiplex.h"
#include "flowd.h"

int selectInitImpl(MultiplexerFunc_t *this)
{
    selectMultiplexer_t *multiplexer = container_of(this, selectMultiplexer_t, funcs);
    multiplexer->maxFd = 0;
    FD_ZERO(&multiplexer->evlist);
    FD_ZERO(&multiplexer->chlist);

    return 1;
}

int selectUnInitImpl(MultiplexerFunc_t *this)
{
    //selectMultiplexer_t *multiplexer = container_of(this, selectMultiplexer_t, funcs);
    return 1;
}

int selectIsActiveImpl(MultiplexerFunc_t *this, int fd)
{
    selectMultiplexer_t *multiplexer = container_of(this, selectMultiplexer_t, funcs);
    return FD_ISSET(fd, &multiplexer->chlist);
}

int selectAddToListImpl(MultiplexerFunc_t *this, int fd)
{
    selectMultiplexer_t *multiplexer = container_of(this, selectMultiplexer_t, funcs);
    FD_SET(fd, &multiplexer->evlist);
    if (multiplexer->maxFd < fd) multiplexer->maxFd = fd;
    return 1;
}

int selectRemoveFromListImpl(MultiplexerFunc_t *this, int fd)
{
    int i;
    selectMultiplexer_t *multiplexer = container_of(this, selectMultiplexer_t, funcs);

    FD_CLR(fd, &multiplexer->evlist);

    for (i = multiplexer->maxFd - 1; i >= 0; --i)
    {
	if (FD_ISSET(i, &multiplexer->evlist))
	{
	    multiplexer->maxFd = i;
	    break;
	}
    }
    return 1;
}

int selectWaitImpl(MultiplexerFunc_t *this)
{
    selectMultiplexer_t *multiplexer = container_of(this, selectMultiplexer_t, funcs);

    memcpy(&multiplexer->chlist, &multiplexer->evlist, sizeof(fd_set));
    return select(multiplexer->maxFd + 1, &multiplexer->chlist, NULL, NULL, NULL);
}

MultiplexerFunc_t *selectNewMultiplexer()
{
    selectMultiplexer_t *multiplexer = (selectMultiplexer_t *) malloc(sizeof(selectMultiplexer_t));
    multiplexer->funcs.Init = selectInitImpl;
    multiplexer->funcs.UnInit = selectUnInitImpl;
    multiplexer->funcs.IsActive = selectIsActiveImpl;
    multiplexer->funcs.AddToList = selectAddToListImpl;
    multiplexer->funcs.RemoveFromList = selectRemoveFromListImpl;
    multiplexer->funcs.Wait = selectWaitImpl;
    return &(multiplexer->funcs);
}

int selectFreeMultiplexer(MultiplexerFunc_t *this)
{
    this->UnInit(this);
    selectMultiplexer_t *multiplexer = container_of(this, selectMultiplexer_t, funcs);
    if (multiplexer != NULL)
    {
	free(multiplexer);
	this = NULL;
	return 1;
    }
    else
    {
	return 0;
    }
}

#ifdef USE_KQUEUE

int kqueueInitImpl(MultiplexerFunc_t *this)
{
    kqueueMultiplexer_t *multiplexer = container_of(this, kqueueMultiplexer_t, funcs);
    multiplexer->kqFd = kqueue();
    if (multiplexer->kqFd == -1)
    {
	return 0;
    }

    multiplexer->monitorFdCount = 0;
    return 1;
}

int kqueueUnInitImpl(MultiplexerFunc_t *this)
{
    kqueueMultiplexer_t *multiplexer = container_of(this, kqueueMultiplexer_t, funcs);
    close(multiplexer->kqFd);
    return 1;
}

int kqueueIsActiveImpl(MultiplexerFunc_t *this, int fd)
{
    kqueueMultiplexer_t *multiplexer = container_of(this, kqueueMultiplexer_t, funcs);
    register int i = 0;
    for (i = 0; i < multiplexer->monitorFdCount; ++i)
    {
	if (multiplexer->evlist[i].ident == (uint) fd && !(multiplexer->evlist[i].flags & EV_ERROR))
	    return 1;
    }
    return 0;
}

int kqueueAddToListImpl(MultiplexerFunc_t *this, int fd)
{
    kqueueMultiplexer_t *multiplexer = container_of(this, kqueueMultiplexer_t, funcs);
    EV_SET(&multiplexer->chlist[multiplexer->monitorFdCount], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
    ++multiplexer->monitorFdCount;
    return 1;
}

int kqueueRemoveFromListImpl(MultiplexerFunc_t *this, int fd)
{
    kqueueMultiplexer_t *multiplexer = container_of(this, kqueueMultiplexer_t, funcs);
    EV_SET(&multiplexer->chlist[multiplexer->monitorFdCount], fd, EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, 0);
    --multiplexer->monitorFdCount;
    return 0;
}

int kqueueWaitImpl(MultiplexerFunc_t *this)
{
    kqueueMultiplexer_t *multiplexer = container_of(this, kqueueMultiplexer_t, funcs);
    return kevent(multiplexer->kqFd, 
	    multiplexer->chlist, multiplexer->monitorFdCount, 
	    multiplexer->evlist, multiplexer->monitorFdCount,
	    NULL);
}

MultiplexerFunc_t *kqueueNewMultiplexer()
{
    kqueueMultiplexer_t *multiplexer = (kqueueMultiplexer_t *) malloc(sizeof(kqueueMultiplexer_t));
    multiplexer->funcs.Init = kqueueInitImpl;
    multiplexer->funcs.UnInit = kqueueUnInitImpl;
    multiplexer->funcs.IsActive = kqueueIsActiveImpl;
    multiplexer->funcs.AddToList = kqueueAddToListImpl;
    multiplexer->funcs.RemoveFromList = kqueueRemoveFromListImpl;
    multiplexer->funcs.Wait = kqueueWaitImpl;
    return &(multiplexer->funcs);
}

int kqueueFreeMultiplexer(MultiplexerFunc_t *this)
{
    this->UnInit(this);
    kqueueMultiplexer_t *multiplexer = container_of(this, kqueueMultiplexer_t, funcs);
    if (multiplexer != NULL)
    {
	free(multiplexer);
	this = NULL;
	return 1;
    }
    else
    {
	return 0;
    }
}

#endif
