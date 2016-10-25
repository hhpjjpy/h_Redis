#include "../hnet.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

int runServer();
int runClient();

int main()
{
	int flag = 0;
	printf("start server or client \n");
	scanf("%d",&flag);
	
	if (flag == 0)
		runServer();
	else
		runClient();
	return 0;
}

int runServer()
{
	char err[256];
	int fd = netTcpServer(err,4567,"127.0.0.1",1000);
	if (fd <= 0){
		printf("TcpServer error message : %s \n", err);
		return -1;
	}
	/*
	if (netNonBlock(err, fd) != 0){
		printf("netNonBlock error message : %s \n", err);
		return -1;
	}
	*/
	
	int clifd = 0;
	char ip[16];
	int port;

	clifd = 0; //只能处理一个连接，
	if ((clifd = netTcpAccept(err, fd, ip, 16, &port)) < 0){
		printf("netTcpAccept error message : %s \n", err);
		return -1;
	}

	while (1)
	{
		char buf[256];
		int ret = 0;
		if ((ret=netRead(clifd, buf, 256)) <= 0){
			printf("read error \n");
			close(clifd);
			return -1;
		}
		else{
			printf("read buf is : %s \n", buf);
		}

		memcpy(buf,"is Ok send by server \n",24);
		
		if ((ret=netWrite(clifd, buf, 24)) <= 0){
			printf("write error \n");
			close(clifd);
			return -1;
		}

	}

}

int runClient()
{
	char err[256];
	int clifd = netTcpConnect(err,"127.0.0.1",4567);
	if (clifd < 0){
		printf("TcpConnect is error , message: %s\n",err);
	}
	
	char buf[256];
	while (1){
		memset(buf,0,256);
		scanf("%s", buf);
		if (netWrite(clifd, buf, 256) <= 0){
			printf("read error \n");
			close(clifd);
			return -1;
		}

		int ret = 0;
		if ((ret = netRead(clifd, buf, 23)) <= 0){
			printf("read error \n");
			close(clifd);
			return -1;
		}
		else{
			printf("read buf is : %s \n", buf);
		}
	}
	
}


