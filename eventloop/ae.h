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


//��ʱ��ʵ�ֶ�ʱ��
typedef struct aeEventLoop
{
	int maxfd; //ע�ᵽeventloop��fd�����ֵ
	int setSize;//�ܼ�ص�fd����
	aeFileEvent *events; //ע����¼�
	aeFiledEvent *fired; //�����˵ģ��¼�
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
//int aeWait(int fd,int mask,long long milliseconds); ��ʱ��ʵ�ֶ�ʱ����غ���
void aeMain(aeEventLoop *eventLoop);
int aeGetSetSize(aeEventLoop *eventLoop);
char* aeGetApiName();

#endif









