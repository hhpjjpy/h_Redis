#ifndef SERVER_H
#define SERVER_H

#include "baseInclude.h"

#define SERVER_OK  0
#define SERVER_ERR -1
#define REDIS_REPLY_CHUNK_BYTES (16*1024) /* 16k output buffer */

typedef struct redisObject
{
	unsigned type;
	int refcount;//用引用计数，就要再所以使用到的地方都使用，这个作为管理资源的条件。不然bug很难调。 涉及到资源共享的问题，引用计数是个不错的解决方式。
	void *ptr;
}robj;


typedef struct redisDb
{
	dict *dicts; //主存储 
	dict *expires; //存储key的过期时间
	//int id;

}redisDb;

struct redisClient;

typedef void(*redisCommandProc)(struct redisClient *c);

typedef struct redisCommand
{
	char *name;
	redisCommandProc proc;
	int arity;//参数个数，正的为读命令，负的为写命令。
}redisCommand;

typedef struct redisClient
{
	uint64_t id;
	int fd;
	redisDb *db;
	redisCommand *comd;
	sds queryBuf;
	size_t queryBuf_peak;
	int queryType;
	int argc;
	robj **argv;
	int multibulklen;
	long bulklen;
	list *reply;
	unsigned long reply_bytes;

	//response buffer 
	int bufpos;
	int sendlen;
	char buf[REDIS_REPLY_CHUNK_BYTES];
	
}redisClient;


typedef struct redisServer
{
	pid_t pid;
	redisDb *db;
	dict *commands;
	aeEventLoop *ae;
	char *ip;
	int port;
	int listenfd;
	list *clients;
}redisServer;

extern struct redisServer oredisServer;

/* networking.c -- Networking and client related related oprations */

redisClient *createClient(int fd);
void freeClient(redisClient *c);
void resetClient(redisClient *c);
void sendReplyToClient(aeEventLoop *el,int fd,void *privdata,int mask);
void processInputBuffer(redisClient *c);
void acceptTcpHandler(aeEventLoop *el,int fd,void *privdata,int mask);
void readQueryFromClient(aeEventLoop *el,int fd,void *privdata,int mask);
void addReplyfull(redisClient *c, robj *obj);
int addReplyBulkCBuffer(redisClient *c, void *ptr,size_t len);
void addReplyError(redisClient *c, char *err);
void addReplyErrorFormat(redisClient *c, const char *fmt, ...);
void addReplyOK(redisClient *c);

//core function 

int processCommand(redisClient *c);
redisCommand* lookupCommandSds(sds name);
redisCommand* lookupCommandChar(char *s);
void call(redisClient *c,int flags);
void populateCommandTable(void);

//hahs data type

//db.c -- keyspace access api
int removeExpire(redisDb *db,robj *key);
int expireIfNeeded(redisDb *db,robj *key);
long long getExpire(redisDb *db,robj *key);
int setExpire(redisDb *db,robj *key,long long lifetime);//lifetime key生存期单位毫秒
robj* lookupkey(redisDb *db,robj *key);
void dbAdd(redisDb *db,robj *key,robj *val);
void dbOverwrite(redisDb *db,robj *key,robj *val);
int dbExists(redisDb *db, robj *key);
int dbDelete(redisDb *db,robj *key);

//commands prototypes 
void setCommand(redisClient *c);
void delCommand(redisClient *c);
void existsCommand(redisClient *c);
void getCommand(redisClient *c);
void hsetCommand(redisClient *c);
void hgetCommand(redisClient *c);
void hdelCommand(redisClient *c);
void hmsetCommand(redisClient *c);
void hmgetCommand(redisClient *c);
void hlenCommand(redisClient *c);

//redis obj function
robj* CreateSdsObj(sds s);
robj* CreateSdsObjLen(sds s,size_t len);
robj* CreateDictObj(dict *d);
void FreeSdsObj(robj *o);
void FreeDictObj(robj *o);
void RedisObjFree(void *pridata);
#endif