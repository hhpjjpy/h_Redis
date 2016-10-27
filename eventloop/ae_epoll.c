#include "ae.h"

#include <sys/epoll.h>

typedef struct aeApiState{
	int epfd;
	struct epoll_event *enents;
}aeApiState;

static int aeApiCreate(aeEventLoop *eventLoop)
{
	aeApiState *state = (aeApiState*)malloc(sizeof(aeApiState));
	if (state == NULL) return -1;
	state->epfd = epoll_create1(0);//参数实际已经作废，填0即可
	if (state->epfd == -1){
		free(state);
		return -1;
	}
	state->enents = (struct epoll_event*)malloc(eventLoop->setSize*sizeof(struct epoll_event));
	if (state->enents == NULL) {
		free(state);
		return -1;
	}
	eventLoop->apidata = state;
}

static void aeApiFree(aeEventLoop *eventLoop)
{
	aeApiState *state = eventLoop->apidata;
	free(state->enents);
	free(state);
}

static int aeApiAddEvent(aeEventLoop *eventLoop,int fd,int mask)
{
	aeApiState *state = (aeApiState*)eventLoop->apidata;
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLERR | EPOLLHUP;
	if (mask&AE_READ)  ev.events |= EPOLLIN;
	if (mask&AE_WRITE)  ev.events |= EPOLLOUT;

	int opflag = EPOLL_CTL_ADD;
	if (eventLoop->events[fd].mask != 0)  opflag = EPOLL_CTL_MOD;
	
	if (epoll_ctl(state->epfd, opflag, fd, &ev) == -1)
		return -1;

	return 0;

}

static int aeApiDelEvent(aeEventLoop *eventLoop,int fd,int delmask)
{
	aeApiState *state = (aeApiState*)eventLoop->apidata;
	struct epoll_event ev; 
	ev.data.fd = fd;
	ev.events = EPOLLERR|EPOLLHUP;
	int mask = eventLoop->events[fd].mask;
	int opflag = EPOLL_CTL_DEL;
	if ( mask ^ delmask != 0){
		opflag = EPOLL_CTL_MOD;
		mask &= ~delmask;
		if (mask&AE_READ) ev.events |= EPOLLIN;
		if (mask&AE_WRITE) ev.events |= EPOLLOUT;
		if (mask&AE_READ == 0 && mask&AE_WRITE) //可读可写事件都不存在时，为删除该fd;
			opflag = EPOLL_CTL_DEL;
	}
	if (epoll_ctl(state->epfd, opflag, fd, &ev) == -1)
		return -1;
	
	return 0;
}

static int aeApiPoll(aeEventLoop *eventLoop, int timeout)
{
	aeApiState *state = (aeApiState*)eventLoop->apidata;
	int readyNum = 0;
	if ((readyNum = epoll_wait(state->epfd, state->enents, eventLoop->setSize, timeout)) == -1) return -1;
	for (int i = 0; i < readyNum; i++){
		struct epoll_event ev = state->enents[i];
		int mask = AE_NONE;
		if (ev.events&EPOLLIN) mask |= AE_READ;
		if (ev.events&EPOLLOUT) mask |= AE_WRITE;
		eventLoop->fired[i].fd = ev.data.fd;
		eventLoop->fired[i].mask = mask;
	}
	return readyNum;
}


static char* aeApiName()
{
	return "epoll";
}



