#include "hnet.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#define ANET_ERR_LEN 256


static void netSetError(char *err,const char *fmt,...)
{
	va_list ap;

	if (!err) return;
	va_start(ap,fmt);
	vsnprintf(err,ANET_ERR_LEN,fmt,ap);
	va_end(ap);
}

static int netSetBlock(char *err,int fd,int non_block)
{
	int flags;

	if ((flags = fcntl(fd, F_GETFL)) == -1){
		netSetError(err, "fcntl(F_GETFL):%s", strerror(errno));
		return NET_ERR;
	}

	if (non_block){
		flags |= O_NONBLOCK;
	}
	else{
		flags &= ~O_NONBLOCK;
	}

	if (fcntl(fd, F_SETFL, flags) == -1){
		netSetError(err,"fcntl(F_SETFL):%s ",strerror(errno));
		return NET_ERR;
	}
	
	return NET_OK;
}

int netNonBlock(char *err,int fd)
{
	return netSetBlock(err,fd,1);
}

int netBlock(char *err,int fd)
{
	return netSetBlock(err,fd,0);
}

int netTcpKeepAlive(char *err,int fd,int interval)
{
	int val = 1;

	if (setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,&val,sizeof(val))==-1)
	{
		netSetError(err,"setsockopt SO_KEEPALIVE: %s ",strerror(errno));
		return NET_ERR;
	}

#ifdef __linux__
	//linux下默认的检查时间是过长，有时，需要手动设置

	val = interval;
	if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1)
	{
		netSetError(err, "setsockopt SO_KEEPALIVE: %s ", strerror(errno));
		return NET_ERR;
	}

#endif

	return NET_OK;

}

static int netSetTcpNoDelay(char *err,int fd,int val)
{
	if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) == -1)
	{
		netSetError(err,"setsockopt  TCP_NODELAY: %s ",strerror(errno));
		return NET_ERR;
	}

	return NET_OK;
}

int netEnableTcpNoDelay(char *err,int fd)
{
	return netSetTcpNoDelay(err,fd,1);
}

int netDisableTcpNoDelay(char *err,int fd)
{
	return netSetTcpNoDelay(err,fd,0);
}

/* Set the socket send timeout (SO_SNDTIMEO socket option) to the specified
* number of milliseconds, or disable it if the 'ms' argument is zero. */
int anetSendTimeout(char *err, int fd, long long ms) {
	struct timeval tv;

	tv.tv_sec = ms / 1000;
	tv.tv_usec = (ms % 1000) * 1000;
	if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) == -1) {
		netSetError(err, "setsockopt SO_SNDTIMEO: %s", strerror(errno));
		return NET_ERR;
	}
	return NET_OK;
}

//将普通格式的地址转换为标志点分十进制的ip
int netGenericResolve(char *err,char *host,char *ipbuf,size_t ipbuf_len,int flags)
{
	struct addrinfo hints, *info;
	int rv;

	memset(&hints,0,sizeof(hints));
	if (flags & 1)  hints.ai_flags == AI_NUMERICHOST;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(host, NULL, &hints, &info)) != 0){
		netSetError(err,"%s",gai_strerror(rv));
		return NET_ERR;
	}

	if (info->ai_family == AF_INET){
		struct sockaddr_in *sa = (struct sockaddr_in*)info->ai_addr;
		inet_ntop(AF_INET, &(sa->sin_addr), ipbuf, ipbuf_len);
	}
	else{
		struct sockaddr_in *sa = (struct sockaddr_in*)info->ai_addr;
		inet_ntop(AF_INET6, &(sa->sin_addr), ipbuf, ipbuf_len);
	}

	freeaddrinfo(info);
	return NET_OK;
}

int netResolve(char *err, char *host, char *ipbuf, size_t ipbuf_len) {
	return netGenericResolve(err, host, ipbuf, ipbuf_len, 0);
}

int netResolveIP(char *err, char *host, char *ipbuf, size_t ipbuf_len) {
	return netGenericResolve(err, host, ipbuf, ipbuf_len, 1);
}

static int netSetReuseAddr(char *err,int fd)
{
	int yes = 1;
	if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes))==-1){
		netSetError(err,"setsockopt SO_REUSEADDR: %s ",strerror(errno));
		return NET_ERR;
	}
	return NET_OK;
}

static int netCreateSocket(char *err,int domain)
{
	int s;
	if ((s = socket(domain, SOCK_STREAM, 0)) == -1){
		netSetError(err,"create socket : %s",strerror(errno));
		return NET_ERR;
	}
	if (netSetReuseAddr(err, s) == NET_ERR){
		close(s);
		return NET_ERR;
	}

	return s;
}

#define NET_CONNECT_NONE 0
#define NET_CONNECT_NONBLOCK 1
#define NET_CONNECT_BE_BINDING 2

static int netTcpGenericConnect(char *err,char *addr,int port,char *source_addr,int flags)
{
	int s = NET_ERR, rv;
	char portstr[6];
	struct addrinfo hints, *servinfo, *bservinfo, *p, *b;

	snprintf(portstr,sizeof(portstr),"%d",port);
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(addr, portstr, &hints, &servinfo)) != 0){
		netSetError(err,"%s",gai_strerror(rv));
		return NET_ERR;
	}

	for(p = servinfo; p != NULL; p = p->ai_next){

		if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			continue;

		if (netSetReuseAddr(err, s) == NET_ERR) goto error;

		if (source_addr){
			int bound = 0;
			if ((rv = getaddrinfo(source_addr, NULL, &hints, &bservinfo)) != 0){
				netSetError(err,"%s",gai_strerror(rv));
				goto error;
			}
			for (b = bservinfo; b != NULL;b = b->ai_next){
				if (bind(s, b->ai_addr, b->ai_addrlen) != -1){
					bound = 1;
					break;
				}
			}

			freeaddrinfo(bservinfo);
			if (!bound){
				netSetError(err,"bind:%s ",strerror(errno));
				goto error;
			}
		}

		if (connect(s, p->ai_addr, p->ai_addrlen) == -1){
			if (errno == EINPROGRESS && flags & NET_CONNECT_NONBLOCK)
				goto end;
			close(s);
			s = NET_ERR;
			continue;
		}

		goto end;
	}
	if (p == NULL)
		netSetError(err,"creating socket : %s ",strerror(errno));
error:
	if (s!=NET_ERR){
		close(s);
		s = NET_ERR;
	}
end:
	freeaddrinfo(servinfo);

	if (s == NET_ERR && source_addr && (flags & NET_CONNECT_BE_BINDING))
		return netTcpGenericConnect(err, addr, port, NULL, flags);
	else
		return s;

}


int netTcpConnect(char *err, char *addr, int port)
{
	return netTcpGenericConnect(err, addr, port, NULL, NET_CONNECT_NONE);
}

int netTcpNonBlockConnect(char *err, char *addr, int port)
{
	return netTcpGenericConnect(err, addr, port, NULL, NET_CONNECT_NONBLOCK);
}

int netTcpNonBlockBindConnect(char *err, char *addr, int port,
	char *source_addr)
{
	return netTcpGenericConnect(err, addr, port, source_addr,NET_CONNECT_NONBLOCK);
}

int netRead(int fd,char *buf,int count)
{
	int doneCount = 0,nowCount = 0;
	while (doneCount != count){
		nowCount = read(fd,buf,count-doneCount);
		if (nowCount == 0) return doneCount;
		if (nowCount == -1) return -1;
		doneCount += nowCount;
		buf +=  doneCount;
	}

	return doneCount;

}

int netWrite(int fd,char *buf,int count)
{
	int doneCount = 0, nowCount = 0;
	while (doneCount != count){
		nowCount = write(fd, buf, count - doneCount);
		if (nowCount == 0) return doneCount;
		if (nowCount == -1) return -1;
		doneCount += nowCount;
		buf += doneCount;
	}

	return doneCount;
}

static int netListen(char *err,int s,struct sockaddr *sa,socklen_t len,int backlog)
{
	if (bind(s, sa, len) == -1){
		netSetError(err,"bind: %s ",strerror(errno));
		close(s);
		return NET_ERR;
	}

	if (listen(s,backlog)==-1){
		netSetError(err,"listen:%s ",strerror(errno));
		close(s);
		return NET_ERR;
	}

	return NET_OK;
}

static int _netTcpServer(char *err,int port,char *bindaddr,int af,int backlog)
{
	int s, rv;
	char _port[6];
	struct addrinfo hints, *servinfo, *p;

	snprintf(_port,6,"%d",port);
	memset(&hints,0,sizeof(hints));
	hints.ai_family = af;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(bindaddr, _port, &hints, &servinfo)) != 0){
		netSetError(err,"%s",gai_strerror(rv));
		return NET_ERR;
	}

	for (p = servinfo; p != NULL;p=p->ai_next){
		if ((s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			continue;
		if (netListen(err, s, p->ai_addr, p->ai_addrlen, backlog) == NET_ERR) goto error;
		goto end;
	}

	if (p == NULL){
		netSetError(err,"unable to bind socket ");
		goto error;
	}

error:
	s = NET_ERR;
end:
	freeaddrinfo(servinfo);
	return s;
}

int netTcpServer(char *err,int port,char *bindaddr,int backlog)
{
	return _netTcpServer(err,port,bindaddr,AF_INET,backlog);
}

static int netGenericAccept(char *err,int s,struct sockaddr *sa,socklen_t *len)
{
	int fd;
	while (1){//保证在非阻塞的情况下，运行正确
		fd = accept(s, sa, len);
		if (fd == -1){
			if (errno == EINTR)
				continue;
			else{
				netSetError(err,"accept: %s ",strerror(errno));
				return NET_ERR;
			}
		}
		break;
	}
	return fd;
}

int netTcpAccept(char *err,int s,char *ip,size_t ip_len,int *port)
{
	int fd;
	struct sockaddr_storage sa;
	socklen_t salen = sizeof(sa);
	if ((fd = netGenericAccept(err, s, (struct sockaddr*)&sa, &salen)) == -1)
		return NET_ERR;
	if (sa.ss_family == AF_INET){
		struct sockaddr_in *s = (struct sockaddr_in *)&sa;
		if (ip) inet_ntop(AF_INET,(void*)&(s->sin_addr),ip,ip_len);
		if (port) *port = ntohs(s->sin_port);
	}

	return fd;
}


