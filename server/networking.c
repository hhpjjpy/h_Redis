#include "Server.h"
#include "../redis_src/util.h"

#include <sys/uio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define REDIS_IOBUF_LEN   16*1024
#define REDIS_REQ_MUL     2
#define REDIS_REQ_INL     1
#define REDIS_INLINE_MAX_SIZE 64*1024
#define REDIS_MBULK_BIG_ARG 32*1024

static int idcount = 0;
static 	robj *CRobj;

redisClient* createClient(int fd)
{
	redisClient *c = (redisClient*)malloc(sizeof(redisClient));
	if (c == NULL) return NULL;
	netNonBlock(NULL,fd);
	c->id = ++idcount;
	c->fd = fd;
	c->db = oredisServer.db;
	c->queryBuf = sdsempty();
	c->comd = NULL;
	c->queryBuf_peak = 0;
	c->queryType = -1;
	c->argc = 0;
	c->argv = NULL;
	c->multibulklen = 0;
	c->bulklen = 0;
	c->reply = CreateList();
	c->reply->free = RedisObjFree;
	c->reply_bytes = 0;
	c->bufpos = 0;
	c->sendlen = 0;

	CRobj = CreateSdsObj(sdsnew("\r\n"));

	return c;
}

void freeClient(redisClient *c)
{
	listNode *node = listSerchNode(oredisServer.clients, c);
	if (node != NULL)
		listRemoveNode(oredisServer.clients,node);
	aeDeleteFileEvent(oredisServer.ae,c->fd,AE_READ|AE_WRITE);
	close(c->fd);
	free(c->argv);
	FreeList(c->reply);
	free(c);
}

void resetClient(redisClient *c)
{
	for (int i = 0; i < c->argc; i++){
		if (c->argv[i] != NULL) FreeSdsObj(c->argv[i]);
	}
	c->argc = 0;
	if (c->argv != NULL) free(c->argv);
	c->argv = NULL;
	sdsfree(c->queryBuf);
	c->queryType = -1;
	c->queryBuf_peak = 0;
	c->queryBuf = sdsempty();
	c->multibulklen = 0;
	c->bulklen = 0;
}

void sendReplyToClient(aeEventLoop *ae,int fd,void *privdata,int mask)
{
	redisClient *c = (redisClient*)privdata;
	while (c->bufpos > 0 || ListLength(c->reply) > 0)
	{
		if (c->bufpos > 0){
			int ret = write(fd,c->buf+c->sendlen,c->bufpos - c->sendlen);
			if (ret <= 0){
				if (errno == EAGAIN)
					continue;
				else
					goto error;
			}
			c->sendlen += ret;
			if (c->sendlen == c->bufpos){
				c->sendlen = 0;
				c->bufpos = 0;
			}
		}
		else {
			robj *o = listGetFrist(c->reply);
			int buflen = sdslen(o->ptr);
			if (buflen == 0){
				listFreeFrist(c->reply);
			}

			int ret = write(fd,(char*)o->ptr+c->sendlen,buflen-c->sendlen);
			if (ret <= 0){
				if (errno == EAGAIN)
					continue;
				else
					goto error;
			}
			c->sendlen += ret;

			if (c->sendlen == buflen){
				listFreeFrist(c->reply);
				c->sendlen = 0;
			}
		}
	}

	if (c->bufpos == 0 && ListLength(c->reply) == 0){
		aeDeleteFileEvent(ae,fd,AE_WRITE);
	}

	return;

error:
		aeDeleteFileEvent(ae, fd, AE_WRITE|AE_READ);
		freeClient(c);

}

int prepareClientToWrite(redisClient *c)
{
	if (aeCreateFileEvent(oredisServer.ae, c->fd, AE_WRITE, sendReplyToClient, c) != 0){
		freeClient(c);
		return  SERVER_ERR;
	}

	return SERVER_OK;
}

int addReplyBulk(redisClient *c, robj *obj)//sds类型的
{
	if (prepareClientToWrite(c) != SERVER_OK) return SERVER_ERR;
	listAddNodeTail(c->reply, obj);
	return SERVER_OK;
}

int addReplyBulkCBuffer(redisClient *c, void *ptr, size_t len)
{
	if (prepareClientToWrite(c) != SERVER_OK) return SERVER_ERR;
	size_t emptySize = sizeof(c->buf) - c->bufpos;
	if (len>emptySize) return SERVER_ERR;
	strncpy((char*)(c->buf+c->bufpos), (char*)ptr, len);
	c->bufpos += len;
	return SERVER_OK;
}

void addReplyBulkCBufferToList(redisClient *c, void *ptr, size_t len)
{
	sds s = sdsnew(ptr);
	robj *obj = CreateSdsObj(s);
	addReplyBulk(c,obj);
}

void addReply(redisClient *c,robj *obj)
{  
	sds s = (sds)obj->ptr;
	size_t len = sdslen(s);
	if (addReplyBulkCBuffer(c, s, len) != SERVER_OK)
		addReplyBulk(c,obj);
}

void addReplyLenHead(redisClient *c, int ll, char prefix)
{
	char buf[10];
	int len;
	buf[0] = prefix;
	len = ll2string(buf + 1, sizeof(buf)-1, ll);
	buf[len + 1] = '\r';
	buf[len + 2] = '\n';
	addReplyBulkCBuffer(c, buf, len+3);
}

void addReplyLen(redisClient *c,robj *obj)
{
	size_t len = sdslen(obj->ptr);
	addReplyLenHead(c,len,'$');
}

void addReplyfull(redisClient *c,robj *obj)
{
	addReplyLen(c,obj);
	addReply(c, obj);
	addReply(c, CRobj);
}

void addReplyString(redisClient *c, char *s, size_t len)
{
	if (prepareClientToWrite(c) != SERVER_OK) return;
	if (addReplyBulkCBuffer(c, s, len) != SERVER_OK)
		addReplyBulkCBufferToList(c, s, len);
}

void addReplyErrorLength(redisClient *c,char *s,size_t len)
{
	addReplyString(c,"-ERR",5);
	addReplyString(c,s,len);
	addReplyString(c,"\r\n",2);
}

void addReplyError(redisClient *c,char *err)
{
	addReplyErrorLength(c,err,strlen(err));
}

void addReplyErrorFormat(redisClient *c,const char *fmt,...)
{
	size_t l, j;
	va_list ap;
	va_start(ap,fmt);
	sds s = sdscatvprintf(sdsempty(),fmt,ap);
	va_end(ap);
	l = sdslen(s);
	for (j = 0; j < 1; j++){
		if (s[j] == '\r' || s[j] == '\n')  s[j] = ' ';
	}
	addReplyErrorLength(c, s, sdslen(s));
	sdsfree(s);
}

void addReplyStatusLength(redisClient *c, char *s, size_t len) {
	addReplyString(c, "+", 1);
	addReplyString(c, s, len);
	addReplyString(c, "\r\n", 2);
}

void addReplyOK(redisClient *c)
{
	addReplyString(c, "+OK", 3);
	addReplyString(c, "\r\n", 2);
}

void addReplyStatus(redisClient *c, char *status) {
	addReplyStatusLength(c, status, strlen(status));
}

void addReplyStatusFormat(redisClient *c, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	sds s = sdscatvprintf(sdsempty(), fmt, ap);
	va_end(ap);
	addReplyStatusLength(c, s, sdslen(s));
	sdsfree(s);
}

int processInlineBuffer(redisClient *c)
{
	char *newline;
	int argc, j;
	sds *argv, aux;
	size_t querylen;

	/* Search for end of line */
	newline = strchr(c->queryBuf, '\n');

	/* Nothing to do without a \r\n */
	if (newline == NULL) {
		if (sdslen(c->queryBuf) > REDIS_INLINE_MAX_SIZE) {
			addReplyError(c, "Protocol error: too big inline request");
			sdsrange(c->queryBuf, 0, -1);
		}
		return SERVER_ERR;
	}

	/* Handle the \r\n case. */
	if (newline && newline != c->queryBuf && *(newline - 1) == '\r')
		newline--;

	/* Split the input buffer up to the \r\n */
	querylen = newline - (c->queryBuf);
	aux = sdsnewlen(c->queryBuf, querylen);
	argv = sdssplitargs(aux, &argc);
	sdsfree(aux);
	if (argv == NULL) {
		addReplyError(c, "Protocol error: unbalanced quotes in request");
		sdsrange(c->queryBuf, 0, -1);
		return SERVER_ERR;
	}

	/* Newline from slaves can be used to refresh the last ACK time.
	* This is useful for a slave to ping back while loading a big
	* RDB file. */


	/* Leave data after the first line of the query in the buffer */
	sdsrange(c->queryBuf, querylen + 2, -1);

	/* Setup argv array on client structure */
	if (argc) {
		if (c->argv) free(c->argv);
		c->argv = malloc(sizeof(robj*)*argc);
	}

	/* Create redis objects for all arguments. */
	for (c->argc = 0, j = 0; j < argc; j++) {
		if (sdslen(argv[j])) {
			c->argv[c->argc] = CreateSdsObj(argv[j]);
			c->argc++;
		}
		else {
			sdsfree(argv[j]);
		}
	}
	free(argv);
	return SERVER_OK;

}

int processMultibulkBuffer(redisClient *c)
{
	char *newline = NULL;
	int pos = 0, ok;
	long long ll;

	if (c->multibulklen == 0) {

		/* Multi bulk length cannot be read without a \r\n */
		newline = strchr(c->queryBuf, '\r');
		if (newline == NULL) {
			if (sdslen(c->queryBuf) > REDIS_INLINE_MAX_SIZE) {
				addReplyError(c, "Protocol error: too big mbulk count string");
				resetClient(c);
			}
			return SERVER_ERR;
		}

		/* Buffer should also contain \n */
		if (newline - (c->queryBuf) > ((signed)sdslen(c->queryBuf) - 2))
			return SERVER_ERR;

		ok = string2ll(c->queryBuf + 1, newline - (c->queryBuf + 1), &ll);
		if (!ok || ll > 1024 * 1024) {
			addReplyError(c, "Protocol error: invalid multibulk length");
			resetClient(c);
			return SERVER_ERR;
		}

		pos = (newline - c->queryBuf) + 2;
		if (ll <= 0) {
			sdsrange(c->queryBuf, pos, -1);
			resetClient(c);
			return SERVER_OK;
		}

		c->multibulklen = ll;

		/* Setup argv array on client structure */
		if (c->argv) free(c->argv);
		c->argv = malloc(sizeof(robj*)*c->multibulklen);
	}

	while (c->multibulklen) {
		/* Read bulk length if unknown */
		if (c->bulklen == 0) {
			newline = strchr(c->queryBuf + pos, '\r');
			if (newline == NULL) {
				if (sdslen(c->queryBuf) > REDIS_INLINE_MAX_SIZE) {
					addReplyError(c,
						"Protocol error: too big bulk count string");
					resetClient(c);
					return SERVER_ERR;
				}
				break;
			}

			/* Buffer should also contain \n */
			if (newline - (c->queryBuf) > ((signed)sdslen(c->queryBuf) - 2))
				break;

			if (c->queryBuf[pos] != '$') {
				addReplyErrorFormat(c,
					"Protocol error: expected '$', got '%c'",
					c->queryBuf[pos]);
				resetClient(c);
				return SERVER_ERR;
			}

			ok = string2ll(c->queryBuf + pos + 1, newline - (c->queryBuf + pos + 1), &ll);
			if (!ok || ll < 0 || ll > 512 * 1024 * 1024) {
				addReplyError(c, "Protocol error: invalid bulk length");
				resetClient(c);
				return SERVER_ERR;
			}

			pos += newline - (c->queryBuf + pos) + 2;
			if (ll >= REDIS_MBULK_BIG_ARG) {
				size_t qblen;

				/* If we are going to read a large object from network
				* try to make it likely that it will start at c->queryBuf
				* boundary so that we can optimize object creation
				* avoiding a large copy of data. */
				sdsrange(c->queryBuf, pos, -1);
				pos = 0;
				qblen = sdslen(c->queryBuf);
				/* Hint the sds library about the amount of bytes this string is
				* going to contain. */
				if (qblen < (size_t)ll + 2)
					c->queryBuf = sdsMakeRoomFor(c->queryBuf, ll + 2 - qblen);
			}
			c->bulklen = ll;
		}

		/* Read bulk argument */
		if (sdslen(c->queryBuf) - pos < (unsigned)(c->bulklen + 2)) {
			/* Not enough data (+2 == trailing \r\n) */
			break;
		}
		else {
			/* Optimization: if the buffer contains JUST our bulk element
			* instead of creating a new object by *copying* the sds we
			* just use the current sds string. */
			if (pos == 0 &&
				c->bulklen >= REDIS_MBULK_BIG_ARG &&
				(signed)sdslen(c->queryBuf) == c->bulklen + 2)
			{
				c->argv[c->argc++] = CreateSdsObj(sdsnewlen(c->queryBuf + pos, c->bulklen));
				sdsIncrLen(c->queryBuf, -2); /* remove CRLF */
				c->queryBuf = sdsempty();
				/* Assume that if we saw a fat argument we'll see another one
				* likely... */
				c->queryBuf = sdsMakeRoomFor(c->queryBuf, c->bulklen + 2);
				pos = 0;
			}
			else {
				c->argv[c->argc++] =
					CreateSdsObj(sdsnewlen(c->queryBuf + pos, c->bulklen));

				pos += c->bulklen + 2;
			}
			c->bulklen = 0;
			c->multibulklen--;
		}
	}

	/* Trim to pos */
	if (pos) sdsrange(c->queryBuf, pos, -1);

	/* We're done when c->multibulk == 0 */
	if (c->multibulklen == 0) return SERVER_OK;

	/* Still not read to process the command */
	return SERVER_ERR;

}

void processInputBuffer(redisClient *c)
{
	while (sdslen(c->queryBuf))
	{
		if (c->queryType == -1){
			if (c->queryBuf[0] == '*'){
				c->queryType = REDIS_REQ_MUL;
			}else{
				c->queryType = REDIS_REQ_INL;
			}
		}

		if (c->queryType == REDIS_REQ_INL){
			if (processInlineBuffer(c) == SERVER_ERR) break;
		}
		else if (c->queryType == REDIS_REQ_MUL){
			if (processMultibulkBuffer(c) == SERVER_ERR) break;
		}
		else{
			addReplyError(c,"is unknow request type");
		}

		if (c->argc == 0){
			resetClient(c);
		}
		else{
			if (processCommand(c) == SERVER_OK)
				resetClient(c);
		}
	}
}

void acceptTcpHandler(aeEventLoop *ae,int fd,void *privdata,int mask)
{
	int clientfd = netTcpAccept(NULL,fd,NULL,0,NULL);
	if (clientfd == -1) return;
	redisClient *c = createClient(clientfd);
	if (c == NULL) return;
	listAddNodeTail(oredisServer.clients,c);
	aeCreateFileEvent(ae, clientfd, AE_READ, readQueryFromClient, c);
}

void readQueryFromClient(aeEventLoop *ae,int fd,void *privdata,int mask)
{
	redisClient *c = (redisClient*)privdata;
	int nread, readlen;
	size_t qblen;

	readlen = REDIS_IOBUF_LEN;

	qblen = sdslen(c->queryBuf);
	if (c->queryBuf_peak < qblen)  c->queryBuf_peak = qblen;
	c->queryBuf = sdsMakeRoomFor(c->queryBuf, readlen);
	nread = read(fd,c->queryBuf+qblen,readlen);
	if (nread <= 0){
		if (errno == EAGAIN)
			nread = 0;
		else{
			freeClient(c);
			return;
		}
	}
	if (nread){
		sdsIncrLen(c->queryBuf,nread);
	}
	//长度有最大限制，先不处理
	processInputBuffer(c);
}





