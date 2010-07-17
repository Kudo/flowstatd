#include "multiplex.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int selectInitImpl(MultiplexorFunc_t *this)
{
    selectMultiplexor_t *multiplexor = container_of(this, selectMultiplexor_t, funcs);
    multiplexor->maxFd = 0;
    multiplexor->monitorFdCount = 0;
    memset(&multiplexor->fdList, 0, sizeof(multiplexor->fdList));

    return 1;
}

int selectUnInitImpl(MultiplexorFunc_t *this)
{
    //selectMultiplexor_t *multiplexor = container_of(this, selectMultiplexor_t, funcs);
    return 1;
}

int selectIsActiveImpl(MultiplexorFunc_t *this, int fd)
{
    selectMultiplexor_t *multiplexor = container_of(this, selectMultiplexor_t, funcs);
    return FD_ISSET(fd, &multiplexor->rfdList);
}

int selectAddToListImpl(MultiplexorFunc_t *this, int fd)
{
    selectMultiplexor_t *multiplexor = container_of(this, selectMultiplexor_t, funcs);
    multiplexor->fdList[multiplexor->monitorFdCount] = fd;
    ++multiplexor->monitorFdCount;
    if (multiplexor->maxFd < fd) multiplexor->maxFd = fd;
    return 1;
}

int selectRemoveFromListImpl(MultiplexorFunc_t *this, int fd)
{
    int newFdList[MAX_MONITOR_FD_COUNT];
    int newMaxFd = 0;
    int i, j;
    selectMultiplexor_t *multiplexor = container_of(this, selectMultiplexor_t, funcs);

    for (i = 0, j = 0; i < MAX_MONITOR_FD_COUNT && j < MAX_MONITOR_FD_COUNT; ++i)
    {
	if (multiplexor->fdList[i] != fd)
	    newFdList[j++] = multiplexor->fdList[i];
    }

    memcpy(&multiplexor->fdList, &newFdList, MAX_MONITOR_FD_COUNT);

    --multiplexor->monitorFdCount;

    for (i = 0; i < multiplexor->monitorFdCount; ++i)
    {
	if (newMaxFd < newFdList[i])
	    newMaxFd = newFdList[i];
    }
    multiplexor->maxFd = newMaxFd;
    return 1;
}

int selectWaitImpl(MultiplexorFunc_t *this)
{
    int i;
    selectMultiplexor_t *multiplexor = container_of(this, selectMultiplexor_t, funcs);

    FD_ZERO(&multiplexor->rfdList);
    for (i = 0; i < multiplexor->monitorFdCount; ++i)
	FD_SET(multiplexor->fdList[i], &multiplexor->rfdList);

    return select(multiplexor->maxFd + 1, &multiplexor->rfdList, NULL, NULL, NULL);
}

MultiplexorFunc_t *selectNewMultiplexor()
{
    selectMultiplexor_t *multiplexor = (selectMultiplexor_t *) malloc(sizeof(selectMultiplexor_t));
    multiplexor->funcs.Init = selectInitImpl;
    multiplexor->funcs.UnInit = selectUnInitImpl;
    multiplexor->funcs.IsActive = selectIsActiveImpl;
    multiplexor->funcs.AddToList = selectAddToListImpl;
    multiplexor->funcs.RemoveFromList = selectRemoveFromListImpl;
    multiplexor->funcs.Wait = selectWaitImpl;
    return &(multiplexor->funcs);
}

int selectFreeMultiplexor(MultiplexorFunc_t *this)
{
    this->UnInit(this);
    selectMultiplexor_t *multiplexor = container_of(this, selectMultiplexor_t, funcs);
    if (multiplexor != NULL)
    {
	free(multiplexor);
	this = NULL;
	return 1;
    }
    else
    {
	return 0;
    }
}

#ifdef USE_KQUEUE

int kqueueInitImpl(MultiplexorFunc_t *this)
{
    kqueueMultiplexor_t *multiplexor = container_of(this, kqueueMultiplexor_t, funcs);
    multiplexor->kqFd = kqueue();
    if (multiplexor->kqFd == -1)
    {
	return 0;
    }

    multiplexor->monitorFdCount = 0;
    return 1;
}

int kqueueUnInitImpl(MultiplexorFunc_t *this)
{
    kqueueMultiplexor_t *multiplexor = container_of(this, kqueueMultiplexor_t, funcs);
    close(multiplexor->kqFd);
    return 1;
}

int kqueueIsActiveImpl(MultiplexorFunc_t *this, int fd)
{
    kqueueMultiplexor_t *multiplexor = container_of(this, kqueueMultiplexor_t, funcs);
    register int i = 0;
    for (i = 0; i < multiplexor->monitorFdCount; ++i)
    {
	if (multiplexor->evlist[i].ident == (uint) fd && !(multiplexor->evlist[i].flags & EV_ERROR))
	    return 1;
    }
    return 0;
}

int kqueueAddToListImpl(MultiplexorFunc_t *this, int fd)
{
    kqueueMultiplexor_t *multiplexor = container_of(this, kqueueMultiplexor_t, funcs);
    EV_SET(&multiplexor->chlist[multiplexor->monitorFdCount], fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
    ++multiplexor->monitorFdCount;
    return 1;
}

int kqueueRemoveFromListImpl(MultiplexorFunc_t *this, int fd)
{
    kqueueMultiplexor_t *multiplexor = container_of(this, kqueueMultiplexor_t, funcs);
    EV_SET(&multiplexor->chlist[multiplexor->monitorFdCount], fd, EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, 0);
    --multiplexor->monitorFdCount;
    return 0;
}

int kqueueWaitImpl(MultiplexorFunc_t *this)
{
    kqueueMultiplexor_t *multiplexor = container_of(this, kqueueMultiplexor_t, funcs);
    return kevent(multiplexor->kqFd, 
	    multiplexor->chlist, multiplexor->monitorFdCount, 
	    multiplexor->evlist, multiplexor->monitorFdCount,
	    NULL);
}

MultiplexorFunc_t *kqueueNewMultiplexor()
{
    kqueueMultiplexor_t *multiplexor = (kqueueMultiplexor_t *) malloc(sizeof(kqueueMultiplexor_t));
    multiplexor->funcs.Init = kqueueInitImpl;
    multiplexor->funcs.UnInit = kqueueUnInitImpl;
    multiplexor->funcs.IsActive = kqueueIsActiveImpl;
    multiplexor->funcs.AddToList = kqueueAddToListImpl;
    multiplexor->funcs.RemoveFromList = kqueueRemoveFromListImpl;
    multiplexor->funcs.Wait = kqueueWaitImpl;
    return &(multiplexor->funcs);
}

int kqueueFreeMultiplexor(MultiplexorFunc_t *this)
{
    this->UnInit(this);
    kqueueMultiplexor_t *multiplexor = container_of(this, kqueueMultiplexor_t, funcs);
    if (multiplexor != NULL)
    {
	free(multiplexor);
	this = NULL;
	return 1;
    }
    else
    {
	return 0;
    }
}

#endif
