#ifndef AE_H
#define AE_H

#define AE_OK  0
#define AE_ERR -1

#define AE_NONE 0
#define AE_READ 1
#define AE_WRITE 2

#include "../base_struct/Hheap.h" //优先队列堆

#include <time.h>
#include <sys/time.h>

struct aeEventLoop;

typedef void(*aeFileProc)(struct aeEventLoop *eventloop, int fd, void *clientDate, int mask);
typedef void(*aeTimeProc)(struct aeEventLoop *eventloop, long long id, void *clientDate);

typedef struct aeFileEvent
{
	int mask;
	aeFileProc rflieProc;
	aeFileProc wfileProc;
	void *clientData;
}aeFileEvent;

typedef struct aeFiledEvent
{
	int fd;
	int mask;
}aeFiledEvent;

typedef struct aeTimeEvent 
{
	long long timeId;
	struct timeval  when;
	struct timeval intervaltime;
	aeTimeProc timeProc;
	int interval;
	void *clientData;
}aeTimeEvent;

typedef struct aeEventLoop
{
	int maxfd; //注册到eventloop中fd的最大值
	int setSize;//能监控的fd数量
	aeFileEvent *events; //注册的事件
	aeFiledEvent *fired; //触发了的，事件
	long long timeIdMax;
	Heap *timeEvents; //时间事件堆
	int stop;
	void *apidata;
}aeEventLoop;

 

aeEventLoop* aeCreateEventLoop(int setSize);
void aeDeleteEventLoop(aeEventLoop *eventLoop);
void aeStop(aeEventLoop *eventLoop);
int aeCreateFileEvent(aeEventLoop *eventLoop,int fd,int mask,aeFileProc proc,void *clientData);
void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask);
int aeGetFileEvent(aeEventLoop *eventLoop,int fd);
long long aeCreateTimeEvent(aeEventLoop *eventloop,aeTimeProc proc,long howlong, int interval,void *clientData);//ms;
int aeDelTimeEvent(aeEventLoop *eventloop,long long timeId);
int aeProcessEvents(aeEventLoop *eventLoop,int flags);
void aeMain(aeEventLoop *eventLoop);
int aeGetSetSize(aeEventLoop *eventLoop);
char* aeGetApiName();

#endif









