#include "../../base_net/hnet.h"
#include "../ae.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>

typedef struct clientData
{
	int fd;
	aeEventLoop *ae;
	char *readbuf;
	char *writebuf;
	
}clientData;

void FreeClient(clientData *client)
{
	free(client->readbuf);
	free(client->writebuf);
	aeDeleteFileEvent(client->ae,client->fd,AE_READ|AE_WRITE);
	close(client->fd);
	free(client);
}

void dealBuf(char *buf, int ret)
{
	char *p = buf;
	char *now = buf + ret;
	while (*now != '\0')
	{
		*p = *now;
		p++;
		now++;
	}
	*p = '\0';
}

void clientWrite(aeEventLoop *ae, int fd, void *data, int mask)
{
	clientData *client = (clientData*)data;
	int ret = 0;
	int count = strlen(client->writebuf);
	char num[4];
	memcpy(num, &count, sizeof(int));
	if ((ret = netWrite(fd, num, sizeof(int))) <= 0)
		goto error;
	if ((ret = netWrite(fd, client->writebuf, count)) <= 0)
		goto error;
	dealBuf(client->writebuf,ret);
	if (strlen(client->writebuf)==0)
		aeDeleteFileEvent(ae, fd, AE_WRITE);
	return ;
error:
	FreeClient(client);
}

void processBuff(clientData *client)
{
	printf("process read buf \n");//µÈµÈÀ©³ä

	strcpy(client->writebuf, client->readbuf);

	aeCreateFileEvent(client->ae, client->fd, AE_WRITE, &clientWrite, client);
}


void clientRead(aeEventLoop *ae,int fd,void *clientdata,int mask)
{
	clientData *client = (clientData*)clientdata;

	int count = 0;
	if ((count = netRead(fd, client->readbuf, 256)) <= 0){
		FreeClient(client);
		return;
	}

	client->readbuf[count] = '\0';

	printf("read ok  message : %s \n",client->readbuf);

	processBuff(client);
}

void setTimeProc(aeEventLoop *eventloop, long long id, void *clientDate)
{
	clientData *client = (clientData*)clientDate;
	strcpy(client->writebuf, "timeProc test run 001 \n");
	aeCreateFileEvent(client->ae, client->fd, AE_WRITE, &clientWrite, client);
}

void setTimeProc002(aeEventLoop *eventloop, long long id, void *clientDate)
{
	clientData *client = (clientData*)clientDate;
	strcpy(client->writebuf, "timeProc test run 002 \n");
	aeCreateFileEvent(client->ae, client->fd, AE_WRITE, &clientWrite, client);
}

void setTimeProc003(aeEventLoop *eventloop, long long id, void *clientDate)
{
	clientData *client = (clientData*)clientDate;
	strcpy(client->writebuf, "timeProc test run 003 \n");
	aeCreateFileEvent(client->ae, client->fd, AE_WRITE, &clientWrite, client);
}

void setTimeProc004(aeEventLoop *eventloop, long long id, void *clientDate)
{
	clientData *client = (clientData*)clientDate;
	strcpy(client->writebuf, "timeProc test run 004 \n");
	aeCreateFileEvent(client->ae, client->fd, AE_WRITE, &clientWrite, client);
}

void setTimeProc005(aeEventLoop *eventloop, long long id, void *clientDate)
{
	clientData *client = (clientData*)clientDate;
	strcpy(client->writebuf, "timeProc test run 005 \n");
	aeCreateFileEvent(client->ae, client->fd, AE_WRITE, &clientWrite, client);
}


void AcceptConnect(aeEventLoop *ae,int fd,void *clientDate,int mask)
{
	clientData *data = (clientData*)malloc(sizeof(clientData));
	data->readbuf = (char*)malloc(256 * sizeof(char));
	data->writebuf = (char*)malloc(256 * sizeof(char));
	char err[256];
	char ip[16];
	int port;
	int clifd = netTcpAccept(err, fd, ip, 16, &port);

	printf("fd : %d is connect \n", clifd);
	printf("clien ip : %s  port: %d \n",ip,port);
	
	data->fd = clifd;
	data->ae = ae;
	aeCreateFileEvent(ae, clifd, AE_READ, &clientRead, (void*)data);
	aeCreateTimeEvent(ae, &setTimeProc, 5000, 1, (void*)data);
	aeCreateTimeEvent(ae, &setTimeProc002, 7000, 1, (void*)data);
	aeCreateTimeEvent(ae, &setTimeProc003, 13000, 1, (void*)data);
	aeCreateTimeEvent(ae, &setTimeProc004, 17000, 0, (void*)data);
	aeCreateTimeEvent(ae, &setTimeProc005, 23000, 1, (void*)data);	
}


int main(){
	aeEventLoop *ae = aeCreateEventLoop(1024);

	char err[256];
	int listenFd = netTcpServer(err,4567,"127.0.0.1",1000);

	aeCreateFileEvent(ae, listenFd, AE_READ, &AcceptConnect, NULL);
		
	aeMain(ae);
}




