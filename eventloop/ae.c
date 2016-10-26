#include "ae.h"
#include <malloc.h>
#include <memory.h>

#ifndef APPLE_OS
	#include "ae_epoll.c"
#else
#endif

aeEventLoop* aeCreateEventLoop(int setSize)
{
	aeEventLoop *ae = (aeEventLoop*)malloc(sizeof(aeEventLoop));
	if (ae == NULL) return NULL;
	ae->maxfd = 0;
	ae->setSize = setSize;
	ae->events = (aeFileEvent*)malloc(setSize*sizeof(aeFileEvent));
	memset(ae->events,0,setSize*sizeof(aeFileEvent));
	if (ae->events == NULL)  goto error;
	ae->fired = (aeFiledEvent*)malloc(setSize*sizeof(aeFiledEvent));
	memset(ae->fired,0,setSize*sizeof(aeFiledEvent));
	if (ae->fired == NULL) goto error;
	ae->stop = 1;
	aeApiCreate(ae);

	return ae;
error:
	free(ae);
	free(ae->events);
	free(ae->fired);
	aeApifree(ae);
	return NULL;
}

void aeDeleteEventLoop(aeEventLoop *eventLoop)
{
	free(eventLoop->events);
	free(eventLoop->fired);
	aeApiFree(eventLoop);
	free(eventLoop);
}

void aeStop(aeEventLoop *eventLoop)
{
	eventLoop->stop = 1;
}

int aeCreateFlieEvent(aeEventLoop *eventLoop, int fd, int mask, aeFileProc proc, void *clientDatta)
{
	if (aeApiAddEvent(eventLoop, fd, mask) != AE_OK)
		return AE_ERR;
	eventLoop->events[fd].mask |= mask;
	eventLoop->events[fd].clientData = clientDatta;
	if (mask&AE_READ) eventLoop->events[fd].rflieProc = proc;
	if (mask&AE_WRITE) eventLoop->events[fd].wfileProc = proc;

	return AE_OK;
}

void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask)
{
	if (aeApiDelEvent(eventLoop, fd, mask) != AE_OK)
		return AE_ERR;

	aeFileEvent *fileevent = &(eventLoop->events[fd]);
	if (mask & (fileevent->mask)& AE_READ)
		fileevent->rflieProc = NULL;
	if (mask&(fileevent->mask)&AE_WRITE)
		fileevent->wfileProc = NULL;
	fileevent->mask &= ~mask;

	return AE_OK;
}

int aeGetFilevent(aeEventLoop *eventLoop,int fd)
{
	return eventLoop->events[fd].mask;
}

int aeProcessEvents(aeEventLoop *eventLoop,int flags)
{
	struct timeval  interval;
	interval.tv_usec = 10000;
	int readyNum = aeApiPoll(eventLoop,&interval);
	for (int i = 0; i < readyNum; i++){
		aeFileEvent *fileEvent = &(eventLoop->events[eventLoop->fired[i].fd]);
		if ((eventLoop->fired[i].mask)&(fileEvent->mask)&AE_READ)
			if (fileEvent->rflieProc != NULL) 
				fileEvent->rflieProc(eventLoop,eventLoop->fired[i].fd,fileEvent->clientData,eventLoop->fired[i].mask);
		if ((eventLoop->fired[i].mask)&(fileEvent->mask)&AE_WRITE)
			if (fileEvent->wfileProc != NULL)
				fileEvent->wfileProc(eventLoop, eventLoop->fired[i].fd, fileEvent->clientData, eventLoop->fired[i].mask);
	}
	memset(eventLoop->fired,0,eventLoop->setSize*sizeof(aeFiledEvent));//处理完之后及时清理

	return readyNum;
}

int aeWait(aeEventLoop *eventLoop)
{
	return  0;//暂时不实现定时器相关的函数
}

void aeMain(aeEventLoop *eventLoop)
{
	eventLoop->stop = 0;
	while (!eventLoop->stop){
		aeProcessEvents(eventLoop,0);
	}
}

int aeGetSetSize(aeEventLoop *eventLoop)
{
	return eventLoop->setSize;
}

char* aeGetApiName()
{
	return aeApiName();
}


