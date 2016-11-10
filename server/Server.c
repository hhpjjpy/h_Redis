#include "Server.h"

#include <malloc.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define EVENT_NUM 10000
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT  4567

redisCommand  commd[] = {
	{"set",setCommand,-3},
	{"del",delCommand,-2},
	{"exists",existsCommand,2},
	{"get",getCommand,2},
	{"hset",hsetCommand,-4},
	{"hdel",hdelCommand,3},
	{"hmset",hmsetCommand,-5},
	{"hmget",hmgetCommand,2},
	{"hlen",hlenCommand,2}
};

//command function

void setCommand(redisClient *c)
{
	if (c->argc >= 3){
		robj *obj = lookupkey(c->db, c->argv[1]);
		if (obj == NULL){
			dbAdd(c->db, c->argv[1], c->argv[2]);
			c->argv[1]->refcount++;
			c->argv[2]->refcount++;
		}
		else{
			dbOverwrite(c->db,c->argv[1],c->argv[2]);
			c->argv[2]->refcount++;
		}
	}
	else
		addReplyError(c,"invalid  parameter ");

	if (c->argc == 4){
		long long ll = 0;
		robj *obj = c->argv[3];
		int ok = string2ll((const char*)obj->ptr,sdslen(obj->ptr),&ll);
		setExpire(c->db,c->argv[1],ll);
		c->argv[1]->refcount++;
	}
	addReplyOK(c);
}
void delCommand(redisClient *c)
{
	addReplyError(c, " now is not realize this commd, place wait ... ");
}
void existsCommand(redisClient *c)
{
	addReplyError(c, " now is not realize this commd, place wait ... ");
}
void getCommand(redisClient *c)
{
	if (c->argc != 2){
		addReplyError(c, " invalid  parameter ");
		return;
	}
	robj *obj = lookupkey(c->db,c->argv[1]);
	if (obj == NULL){
		addReplyError(c, " not have this key ");
	}
	else{
		addReplyfull(c,obj);
	}
	
}
void hsetCommand(redisClient *c)
{
	addReplyError(c, "now is not realize this commd, place wait ... ");
}
void hgetCommand(redisClient *c)
{
	addReplyError(c, "now is not realize this commd, place wait ... ");
}
void hdelCommand(redisClient *c)
{
	addReplyError(c, "now is not realize this commd, place wait ... ");
}
void hmsetCommand(redisClient *c)
{
	addReplyError(c, "now is not realize this commd, place wait ... ");
}
void hmgetCommand(redisClient *c)
{
	addReplyError(c, "now is not realize this commd, place wait ... ");
}
void hlenCommand(redisClient *c)
{
	addReplyError(c, "now is not realize this commd, place wait ... ");
}

//redis obj fun

robj* CreateSdsObj(sds s)
{
	robj *obj = (robj*)malloc(sizeof(obj));
	obj->refcount = 1;
	obj->type = 1;
	obj->ptr = s;

	return obj;
}

robj* CreateSdsObjLen(sds s, size_t len)
{
	robj *obj = (robj*)malloc(sizeof(obj));
	obj->refcount = 1;
	obj->type = 1;
	obj->ptr = malloc(sizeof(char)*len + 1);
	memcpy(obj->ptr, s, len);
	char *buf = obj->ptr;
	buf[len] = '\n';

	return obj;
}

robj* CreateDictObj(dict *d)
{
	robj *obj = (robj*)malloc(sizeof(robj));
	obj->ptr = d;
	obj->refcount = 1;
	obj->type = 2;

	return obj;
}

void FreeSdsObj(robj *o)
{
	o->refcount--;
	if (o->refcount != 0) return;
	sdsfree(o->ptr);
	free(o);
}

void FreeDictObj(robj *o)
{
	o->refcount--;
	if (o->refcount != 0) return;
	dictFree((dict*)o->ptr);
	free(o);
}
void FreeLL(void *ll)
{
	free(ll);
}

unsigned int dictSdsHash(const void *key) {
	return dictGenHashFunction((unsigned char*)key, sdslen((char*)key));
}

int dictSdsKeyCompare( void *key1, void *key2)
{
	int l1, l2;

	l1 = sdslen((sds)key1);
	l2 = sdslen((sds)key2);
	if (l1 != l2) return l1 - l2;
	return memcmp(key1, key2, l1);
}

void dictSdsFree(void *val)
{
	sdsfree(val);
}

unsigned int dictSdsObjHash(const void *key) {
	robj *obj = (robj*)key;
	sds realKey = (sds)obj->ptr;
	return dictGenHashFunction((unsigned char*)realKey, sdslen(realKey));
}

int dictSdsObjKeyCompare(void *key1, void *key2)
{
	robj *obj1 = (robj*)key1;
	robj *obj2 = (robj*)key2;
	
	sds realKey1 = (sds)obj1->ptr;
	sds realKey2 = (sds)obj2->ptr;

	int l1, l2;

	l1 = sdslen((sds)realKey1);
	l2 = sdslen((sds)realKey2);
	if (l1 != l2) return l1 - l2;
	return memcmp(realKey1, realKey2, l1);
}

void dictSdsObjFree(void *val)
{
	robj *obj = (robj*)val;
	obj->refcount--;
	if (obj->refcount != 0) return;
	sdsfree(obj->ptr);
	free(obj);
}


void RedisObjFree(void *pridata)
{
	robj *obj = (robj*)pridata;
	switch (obj->type) //只先支持string-string 和string - hash
	{
	case 1: //sds string 
		FreeSdsObj(obj);
		break;
	case 2:
		FreeDictObj(obj);
		break;
	}
}

/*Db->ditc key are sds string vals are redis object*/
dictType dbDictType = {
	dictSdsObjHash,
	NULL,
	NULL,
	dictSdsObjKeyCompare,
	dictSdsFree,
	RedisObjFree
};

/*Db->expires */
dictType exDictType = {
	dictSdsHash,
	NULL,
	NULL,
	dictSdsKeyCompare,
	dictSdsFree,
	FreeLL
};

redisDb* CreateRedisDb()
{
	redisDb *db = (redisDb*)malloc(sizeof(redisDb));
	db->dicts = dictCreate(&dbDictType);
	db->expires = dictCreate(&exDictType);
}

redisServer  oredisServer;

int clientMatch(void *client1,void *client2)
{
	redisClient *c1 = (redisClient*)client1;
	redisClient *c2 = (redisClient*)client2;

	return c1->id - c2->id;
}

void initServer(redisServer *s)
{
	s->pid = getpid();
	s->db = CreateRedisDb();
	s->commands = dictCreate(&exDictType);
	s->ae = aeCreateEventLoop(EVENT_NUM);
	s->ip = SERVER_IP;
	s->port = SERVER_PORT;
	s->clients = CreateList();
	ListMatchFunSet(s->clients,clientMatch);
	s->listenfd = netTcpServer(NULL,s->port,s->ip,1000);
	
	aeCreateFileEvent(s->ae,s->listenfd,AE_READ,acceptTcpHandler,NULL);
}

//core funciton 
int processCommand(redisClient *c)
{
	if (!strcmp(c->argv[0]->ptr, "quit")){
		addReplyOK(c);
		return 0;
	}

	c->comd = lookupCommandSds(c->argv[0]->ptr);
	if (c->comd == NULL){
		addReplyError(c,"unknow commd ");
		return 0;
	}
	if ((c->comd->arity>0 && c->comd->arity > c->argc)||c->comd->arity < -c->argc ){
		addReplyError(c," parameter number is error");
		return  0;
	}

	call(c, 0);

	return 0;
}

redisCommand* lookupCommandSds(sds name)
{
	redisCommand *commd = (redisCommand*)dictFetchValue(oredisServer.commands,name);
	
	return commd;
}

redisCommand* lookupCommandChar(char *s)
{
	sds name = sdsnew(s);
	redisCommand *commd = (redisCommand*)dictFetchValue(oredisServer.commands, name);
	return commd;
}

void call(redisClient *c,int flags)
{
	c->comd->proc(c);
}

void populateCommandTable(){
	int commdSize = sizeof(commd) / sizeof(redisCommand);
	for (int i = 0; i < commdSize; i++){
		redisCommand *cd = commd + i;
		dictAdd(oredisServer.commands,sdsnew(cd->name),cd);
	}
}

void PirntVersion()
{
	printf("********************************\n");
	printf("         h_redis 0.01        \n");
	printf("********************************\n");
}

void goDaemon()
{
	if (fork() != 0) exit(0);
	setsid();
	int fd;
	if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		if (fd > STDERR_FILENO) close(fd);
	}
}


int main()
{
	PirntVersion();

	initServer(&oredisServer);

	populateCommandTable();

	aeMain(oredisServer.ae);
}


