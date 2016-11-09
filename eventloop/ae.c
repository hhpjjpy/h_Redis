#include "ae.h"
#include <malloc.h>
#include <memory.h>
#include <sys/time.h>

#ifndef APPLE_OS
	#include "ae_epoll.c"
#else
#endif

/*
时间堆类型相关函数
*/
int timeCompare(void *val1,void *val2)
{
	aeTimeEvent *tev1 = (aeTimeEvent*)val1;
	aeTimeEvent *tev2 = (aeTimeEvent*)val2;
	
	return (tev1->when.tv_sec * 1000 + tev1->when.tv_usec / 1000) - (tev2->when.tv_sec * 1000 + tev2->when.tv_usec / 1000);
}

int timeEventMatch(void *heapnode,void *data)
{
	long long *timeid = (long long*)data;
	aeTimeEvent *tev = (aeTimeEvent*)heapnode;

	return tev->timeId == *timeid;
}



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
	ae->timeIdMax = 0;
	heapType *htype = (heapType*)malloc(sizeof(heapType));
	htype->Campare = timeCompare;
	htype->matchVal = timeEventMatch;
	htype->valFree = NULL;
	ae->timeEvents = CreateHeap(htype);
	aeApiCreate(ae);

	return ae;
error:
	free(ae);
	free(ae->events);
	free(ae->fired);
	aeApiFree(ae);
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

int aeCreateFileEvent(aeEventLoop *eventLoop, int fd, int mask, aeFileProc proc, void *clientDatta)
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
		return ;

	aeFileEvent *fileevent = &(eventLoop->events[fd]);
	if (mask & (fileevent->mask)& AE_READ)
		fileevent->rflieProc = NULL;
	if (mask&(fileevent->mask)&AE_WRITE)
		fileevent->wfileProc = NULL;
	fileevent->mask &= ~mask;
}

int aeGetFileEvent(aeEventLoop *eventLoop, int fd)
{
	return eventLoop->events[fd].mask;
}

long long aeCreateTimeEvent(aeEventLoop *eventloop, aeTimeProc proc, long howlong, int interval,void *clientdata)
{
	struct timeval  tval;
	gettimeofday(&tval,NULL);
	tval.tv_sec += howlong / 1000;
	tval.tv_usec += (howlong % 1000) * 1000;

	aeTimeEvent *aeTev = (aeTimeEvent*)malloc(sizeof(aeTimeEvent));
	aeTev->timeId = ++(eventloop->timeIdMax);
	aeTev->interval = interval;
	aeTev->intervaltime.tv_sec = howlong / 1000;
	aeTev->intervaltime.tv_usec = (howlong % 1000) * 1000;
	aeTev->when = tval;
	aeTev->timeProc = proc;
	aeTev->clientData = clientdata;
	aeTimeEvent *oldHead = getHead(eventloop->timeEvents);
	if (addHeapVal(eventloop->timeEvents, aeTev) != 0)
		return -1;
	return aeTev->timeId;
}

int aeDelTimeEvent(aeEventLoop *eventloop, long long timeId)
{
	return delMatch(eventloop->timeEvents, &timeId);

}

struct timeval* getTimeVal(aeEventLoop *eventLoop)
{
	aeTimeEvent *ae = (aeTimeEvent*)getHead(eventLoop->timeEvents);
	if (ae == NULL) return NULL;
	static struct timeval  pooltvl;
	gettimeofday(&pooltvl, NULL);
 	
	long sec = ae->when.tv_sec - pooltvl.tv_sec;
	pooltvl.tv_sec = sec > 0 ? sec : 0;
	long usec = ae->when.tv_usec - pooltvl.tv_usec;
	pooltvl.tv_usec = usec > 0 ? usec : 0;
	return &pooltvl;
}

int compareTimeVal(struct timeval *tv1,struct timeval *tv2)
{
	return (tv1->tv_sec * 1000 + tv1->tv_usec / 1000) - (tv2->tv_sec * 1000 + tv2->tv_usec / 1000);
}

void RunNowReadyTimeEvent(aeEventLoop *eventloop)
{
	struct timeval tvl;
	gettimeofday(&tvl, NULL);
	while (!isEmptyHeap(eventloop->timeEvents)){
		aeTimeEvent *aeTev = getHead(eventloop->timeEvents);
		if (compareTimeVal(&tvl, &(aeTev->when)) < 0) break;
		if (aeTev->timeProc!=NULL)
			aeTev->timeProc(eventloop,aeTev->timeId,aeTev->clientData);
		if (aeTev->interval >0){
			popHead(eventloop->timeEvents);
			aeTev->when.tv_sec = tvl.tv_sec + aeTev->intervaltime.tv_sec;
			aeTev->when.tv_usec = tvl.tv_usec + aeTev->intervaltime.tv_usec;
			addHeapVal(eventloop->timeEvents,aeTev);
		}
		else
			delHead(eventloop->timeEvents);
	}
}

int aeProcessEvents(aeEventLoop *eventLoop,int flags)
{
	struct timeval *p = getTimeVal(eventLoop);

	int readyNum = aeApiPoll(eventLoop,p);
	if (readyNum ==-1 ) return -1;
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

	RunNowReadyTimeEvent(eventLoop);

	return readyNum;
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


