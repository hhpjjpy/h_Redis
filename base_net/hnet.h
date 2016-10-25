#ifndef HNET_H
#define HNET_H
#include <stdlib.h>

int netTcpConnect(char *err,char *addr,int port);
int netTcpNonBlockConnect(char *err,char *addr,int port);
int netTcpNonBlockBindConnect(char *err,char *addr,int port,char *source_addr);
int netTcpServer(char *err, int port, char *bindaddr,int backlog);
int netTcpAccept(char *err,int serversock,char *ip,size_t ip_len,int *port);
int netRead(int fd, char *buf, int count);
int netWrite(int fd,char *buf,int count);
int netNonBlock(char *err,int fd);
int netBlock(char* err,int fd);
int netEnableTcpNoDelay(char *err,int fd);
int netDisableTcpnoDelay(char *err,int fd);
int netTcpKeepAlive(char *err, int fd, int interval);

#define NET_OK 0
#define NET_ERR -1

#endif