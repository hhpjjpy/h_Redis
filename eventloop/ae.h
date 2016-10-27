#ifndef AE_H
#define AE_H

#define AE_OK  0
#define AE_ERR -1

#define AE_NONE 0
#define AE_READ 1
#define AE_WRITE 2

struct aeEventLoop;

typedef void(*aeFileProc)(struct aeEventLoop *eventloop, int fd, void *clientDate, int mask);

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


//暂时不实现定时器
typedef struct aeEventLoop
{
	int maxfd; //注册到eventloop中fd的最大值
	int setSize;//能监控的fd数量
	aeFileEvent *events; //注册的事件
	aeFiledEvent *fired; //触发了的，事件
	int stop;
	void *apidata;
}aeEventLoop;



aeEventLoop* aeCreateEventLoop(int setSize);
void aeDeleteEventLoop(aeEventLoop *eventLoop);
void aeStop(aeEventLoop *eventLoop);
int aeCreateFileEvent(aeEventLoop *eventLoop,int fd,int mask,aeFileProc proc,void *clientData);
void aeDeleteFileEvent(aeEventLoop *eventLoop, int fd, int mask);
int aeGetFileEvent(aeEventLoop *eventLoop,int fd);
int aeProcessEvents(aeEventLoop *eventLoop,int flags);
//int aeWait(int fd,int mask,long long milliseconds); 暂时不实现定时器相关函数
void aeMain(aeEventLoop *eventLoop);
int aeGetSetSize(aeEventLoop *eventLoop);
char* aeGetApiName();

#endif









