#include "hdict.h"

#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#define USED_UP_NUMBER 2 
#define dictIsRehashing(d) (d->rehashidx != -1) 


static void initDictht(dictht *ht)
{
	ht->table = NULL;
	ht->size = 0;
	ht->used = 0;
}

dict* dictCreate(dictType *type)
{
	dict *d = (dict*)malloc(sizeof(dict));
	d->type = type;
	d->rehashidx = -1;
	initDictht(&(d->ht[0]));
	initDictht(&(d->ht[1]));
	return d;
}

static unsigned long  GetNextSuitableSzie(unsigned long size)
{
	int i = 4;
	while (1){
		if (i >= size)
			return i;
		i *= 2;
	}
}


int dictExpand(dict *d, unsigned long size)
{   
	int realsize = GetNextSuitableSzie(size);
	if (dictIsRehashing(d) || d->ht[0].used > size)
		return DICT_ERR;
		
	if (realsize <= d->ht[0].size) return DICT_ERR;

	dictht ht;
	ht.size = realsize;
	ht.used = 0;
	ht.table = (dictEntry**)malloc(realsize*sizeof(dictEntry*));
	memset(ht.table,0, realsize*sizeof(dictEntry*));
	
	//初次初始化
	if (d->ht[0].table == NULL) {
		d->ht[0] = ht;
		return DICT_OK;
	}

	d->ht[1] = ht;
	d->rehashidx = 0;

	return DICT_OK;
}

static int  isNeedExpandSize(dict *d)
{
	if (dictIsRehashing(d)) return DICT_OK;

	if (d->ht[0].size == 0){
		dictExpand(d, 4);
		return DICT_OK;
	}

	if (d->ht[0].used / d->ht[0].size > USED_UP_NUMBER)
		return dictExpand(d, 2 * (d->ht[0].used));

	return DICT_OK;

}

int dictAdd(dict *d,void *key,void *val)
{
	if (isNeedExpandSize(d)!=0) return DICT_ERR;

	dictht *ht_ptr = dictIsRehashing(d) ? &d->ht[1] : &d->ht[0];

	int index = dictHashKey(d, key) % ht_ptr->size;

	dictEntry *p = ht_ptr->table[index];
	while (p){
		if (d->type->keyCompare(p->key, key)==0)
			return DICT_ERR;//key已经存在时无法再添加相同的key
		p = p->next;
	}

	p = ht_ptr->table[index];
	dictEntry *newEntry = (dictEntry*)malloc(sizeof(dictEntry));
	newEntry->key = key;
	newEntry->val = val;

	ht_ptr->table[index] = newEntry;
	ht_ptr->table[index]->next = p;
	ht_ptr->used++;

	return DICT_OK;
}

dictEntry* dictFind(dict *d, const void *key)//key有可能存在于2个空间中的任何一个。
{
	if (d->ht[0].size == 0) return NULL;
	int hashindex = dictHashKey(d, key);//这里将计算提出来，避免在循环中重复计算。
	for (int i = 0; i <= 1; i++){
		dictht ht = d->ht[i];
		int index = hashindex % ht.size;

		dictEntry *p = ht.table[index];
		while (p){
			if (d->type->keyCompare(p->key, key)==0)
				return p;
			p = p->next;
		}
		if (!dictIsRehashing(d)) return NULL; //当没有发生rehash过程时，没找到就可退出
	}
	return NULL;
}


int dictReplace(dict *d,void *key,void *val)
{
	dictEntry *p = dictFind(d,key);
	if (!p)  return dictAdd(d,key,val);
	p->val = val;
	return DICT_OK;
}

int dictDelete(dict *d,const void *key)
{
	int hashindex = dictHashKey(d, key);
	int flag = 0;
	if (d->ht[0].size == 0) return DICT_ERR;

	for (int i = 0; i <= 1; i++){
		dictht *ht_ptr = &d->ht[i];
		int index = hashindex%(ht_ptr->size);
		dictEntry *p = ht_ptr->table[index];
		dictEntry *pre = NULL;
		while (p){
			if (d->type->keyCompare(p->key, key)==0){
				
				if (pre) pre->next = p->next;
				else  ht_ptr->table[index] = NULL;
	
				dictKeyFree(d, key);
				dictValFree(d, p->val);
				free(p);

				ht_ptr->used--;
				flag++;
			}

			pre = p;
			p = p->next;
		}

		if (!dictIsRehashing(d)) return (flag == 0 ? DICT_ERR : DICT_OK);
	}

	return (flag == 0? DICT_ERR:DICT_OK);

}

void* dictFetchValue(dict *d,const void *key)
{
	dictEntry *entry = dictFind(d,key);

	return entry ? entry->val : NULL;
}

int dictResize(dict *d)
{
	if (dictIsRehashing(d)) return DICT_ERR;

	int min = d->ht[0].used;
	if (min < 4)
		min = 4;
	return dictExpand(d,min);
}


int dictRehash(dict *d,int n)
{
	if (!dictIsRehashing(d)) return DICT_ERR;


	while (n-- && d->ht[0].used>0){
		dictEntry *entry = d->ht[0].table[d->rehashidx];
		while (entry == NULL){
			d->rehashidx++;
			entry = d->ht[0].table[d->rehashidx];
		}
		
		while (entry)
		{
			int index = dictHashKey(d, entry->key) % d->ht[1].size;

			d->ht[0].table[d->rehashidx] = entry->next;
			entry->next = d->ht[1].table[index];
			d->ht[1].table[index] = entry;
			d->ht[0].used--;
			d->ht[1].used++;

			entry = d->ht[0].table[d->rehashidx];
		}

		d->ht[0].table[d->rehashidx] = NULL;
		d->rehashidx++;//不要忘记更新这个重要的标志位，这样才能转入下一个。
	}

	if (d->ht[0].used == 0){//rehash完成时要注意把table切换回去。ht[0]是主表，ht[1]只用于调整大小时避免性能瓶颈出现。这样可以分多次来执行扩容过程。
		d->rehashidx = -1;
		free(d->ht[0].table);
		d->ht[0] = d->ht[1];
		d->ht[1].table = NULL;
		d->ht[1].size = 0;
		d->ht[1].used = 0;
		//本来可以通过返回不同的值来确定，处于不同的状态。现在暂时忽略。
		return 1;//表示已完成
	}

	return DICT_OK;

}

long long timeInMilliseconds(void) {
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return (((long long)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}


int dictRehashMillisenconeds(dict *d, int ms)
{
	long long start = timeInMilliseconds();
	int rehashes = 0;

	while (dictRehash(d, 100)==0) {
		rehashes += 100;
		if (timeInMilliseconds() - start > ms) break;
	}
	return rehashes;

}

void dictEmpty(dict *d)
{
	for (int i = 0; i <= 1; i++){
		dictht *ht_ptr = &d->ht[i];
		for (int j = 0; j < ht_ptr->size; j++){
			dictEntry *entry = ht_ptr->table[j];
			while (entry){
				dictEntry *temp = entry;
				entry = entry->next;

				dictKeyFree(d, temp->key);
				dictValFree(d, temp->val);
				free(temp);

				ht_ptr->used--;
			}

			ht_ptr->table[j] = NULL;
		}
		free(ht_ptr);
	}

	free(d);
}


