#include "Server.h"

#include <sys/time.h>

int removeExpire(redisDb *db, robj *key) //key is sds
{
	int ret1 = dictDelete(db->expires, (sds)key->ptr);
	int ret2 = dictDelete(db->dicts, key);

	if (ret1 == 0 && ret2 == 0)
		return 0;
	return -1;
}
int expireIfNeeded(redisDb *db, robj *key)
{
	struct timeval tvl;
	gettimeofday(&tvl, NULL);
	long long now = tvl.tv_sec * 1000 + tvl.tv_usec / 1000;
	long long when = getExpire(db,key);

	if (when == 0)
		return 0;//为设置

	if (now > when){
		removeExpire(db, key);
		return -1; //表示已到过期时间
	}

	return  0;
}

long long getExpire(redisDb *db, robj *key)
{
	long long *ptr = dictFetchValue(db->expires,(sds)key->ptr);
	if (ptr == NULL) 
		return 0;
	else 
		return *ptr;
}

int setExpire(redisDb *db, robj *key, long long lifetime)
{
	struct timeval tvl;
	gettimeofday(&tvl, NULL);
	 
	long long *ptr = (long long *)malloc(sizeof(long long));
	*ptr = tvl.tv_sec * 1000 + tvl.tv_usec / 1000 + lifetime;
	return dictReplace(db->expires, key->ptr, ptr);
}

robj* lookupkey(redisDb *db, robj *key)
{
	expireIfNeeded(db, key);
	return dictFetchValue(db->dicts,key);
}

void dbAdd(redisDb *db, robj *key, robj *val)
{
	dictAdd(db->dicts,key,val);
}
void dbOverwrite(redisDb *db, robj *key, robj *val)
{

	dictReplace(db->dicts, key, val);
}

int dbExists(redisDb *db, robj *key)
{
	removeExpire(db,key);
	dictEntry *entry = dictFind(db->dicts,key);

	return entry == NULL ? 0 : 1;
}

int dbDelete(redisDb *db, robj *key)
{
	return dictDelete(db->dicts,key);
}