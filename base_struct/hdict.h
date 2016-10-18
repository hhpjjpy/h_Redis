#ifndef HDICT_H
#define HDICT_H

typedef struct dictEntry{
	void *key;
	void *val;
	struct dictEntry *next;
}dictEntry;//链地址法解决冲突

typedef struct dictType{
	unsigned int(*hashFunction)(const void *key);
	void* (*keyDup)(void *privdata,const void  *key);
	void* (*valDup)(void *privdata, const void *obj);
	int (*keyCompare)(void *key1, void *key2);
	void (*keyFree)(void *key);
	void (*valFree)(void *val);
}dictType;

typedef struct dictht{
	dictEntry **table;
	unsigned long size;
	unsigned long used;
}dictht;

typedef struct dict{
	dictType *type;
	dictht ht[2];
	long rehashidx;
}dict;

#define dictHashKey(d,key) (d)->type->hashFunction(key)
#define dictKeyFree(d,key) (d)->type->keyFree(key)
#define dictValFree(d,val) (d)->type->valFree(val)

#define DICT_OK 0;
#define DICT_ERR -1;

dict *dictCreate(dictType *type);
int dictExpand(dict *d, unsigned long size);
int dictAdd(dict *d,void *key,void *val);
dictEntry* dictFind(dict *d, const void *key);
int dictReplace(dict *d,void *key,void *val);
void dictDelete(dict *d,const void *key);
void* dictFetchValue(dict *d,const void *key);
int dictResize(dict *d);
int dictRehash(dict *d,int n);
int dictRehashMilliseconeds(dict *d,int ms);
void dictEmpty(dict *d);

#endif