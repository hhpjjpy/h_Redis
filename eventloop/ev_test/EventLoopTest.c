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

	client->ae = ae;

	int count = 0;
	if ((count = netRead(fd, client->readbuf, 256)) <= 0){
		FreeClient(client);
		return;
	}

	client->readbuf[count] = '\0';

	printf("read ok  message : %s \n",client->readbuf);

	processBuff(client);
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
	aeCreateFileEvent(ae, clifd, AE_READ, &clientRead, (void*)data);	
}


int main(){
	aeEventLoop *ae = aeCreateEventLoop(1024);

	char err[256];
	int listenFd = netTcpServer(err,4567,"127.0.0.1",1000);

	aeCreateFileEvent(ae, listenFd, AE_READ, &AcceptConnect, NULL);
		
	aeMain(ae);
}




