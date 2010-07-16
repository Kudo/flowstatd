#include "multiplex.h"
#include <stdio.h>
#include <stdlib.h>

int selectInitImpl(MultiplexorFunc_t *this)
{
    selectMultiplexor_t *multiplexor = container_of(this, selectMultiplexor_t, funcs);
    FD_ZERO(&multiplexor->rfdList);
    multiplexor->monitorFdCount = 0;

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
    FD_SET(fd, &multiplexor->rfdList);
    ++multiplexor->monitorFdCount;
    return 1;
}

int selectRemoveFromListImpl(MultiplexorFunc_t *this, int fd)
{
    selectMultiplexor_t *multiplexor = container_of(this, selectMultiplexor_t, funcs);
    --multiplexor->monitorFdCount;
    FD_CLR(fd, &multiplexor->rfdList);
    return 1;
}

int selectWaitImpl(MultiplexorFunc_t *this)
{
    selectMultiplexor_t *multiplexor = container_of(this, selectMultiplexor_t, funcs);
    return select(multiplexor->monitorFdCount + 1, &multiplexor->rfdList, NULL, NULL, NULL);
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
    // TODO
    kqueueMultiplexor_t *multiplexor = container_of(this, kqueueMultiplexor_t, funcs);
    for (int i = 0; i < multiplexor->monitorFdCount; ++i)
    {
	if (multiplexor->evlist[i].ident == (uint) fd)
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
    // TODO
    kqueueMultiplexor_t *multiplexor = container_of(this, kqueueMultiplexor_t, funcs);
    return 0;
}

int kqueueWaitImpl(MultiplexorFunc_t *this)
{
    kqueueMultiplexor_t *multiplexor = container_of(this, kqueueMultiplexor_t, funcs);
    return kevent(multiplexor->kqFd, 
	    multiplexor->chlist, multiplexor->monitorFdCount + 1, 
	    multiplexor->evlist, multiplexor->monitorFdCount + 1,
	    NULL);
}

#endif
