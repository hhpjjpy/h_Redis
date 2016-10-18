#include "hdict.h"

#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define USED_UP_NUMBER 10 
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
	
	//���γ�ʼ��
	if (d->ht[0].table == NULL) {
		d->ht[0] = ht;
		return DICT_OK;
	}

	d->ht[1] = ht;
	d->rehashidx = 0;

	return DICT_ERR;
}

static int  isNeedExpandSize(dict *d)
{
	if (dictIsRehashing(d)) return DICT_OK;

	if (d->ht[0].size == 0){
		dictExpand(d, 4);
		return DICT_OK;
	}

	if (d->ht[0].used / d->ht[0].size > USED_UP_NUMBER)
		return dictExpand(d, 2 * (d->ht[0].size));

	return DICT_OK;

}

int dictAdd(dict *d,void *key,void *val)
{
	if (!isNeedExpandSize(d)) return DICT_ERR;

	dictht *ht_ptr = dictIsRehashing(d) ? &d->ht[1] : &d->ht[0];

	int index = dictHashKey(d, key) % ht_ptr->size;

	dictEntry *p = ht_ptr->table[index];
	while (p){
		if (d->type->keyCompare(p->key, key))
			return DICT_ERR;//key�Ѿ�����ʱ�޷��������ͬ��key
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

dictEntry* dictFind(dict *d, const void *key)//key�п��ܴ�����2���ռ��е��κ�һ����
{
	if (d->ht[0].size == 0) return NULL;
	int hashindex = dictHashKey(d, key);//���ｫ�����������������ѭ�����ظ����㡣
	for (int i = 0; i <= 1; i++){
		dictht ht = d->ht[i];
		int index = hashindex % ht.size;

		dictEntry *p = ht.table[index];
		while (p){
			if (d->type->keyCompare(p->key, key))
				return p;
			p = p->next;
		}
		if (!dictIsRehashing(d)) return NULL; //��û�з���rehash����ʱ��û�ҵ��Ϳ��˳�
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

void dictDelete(dict *d,const void *key)
{
	int hashindex = dictHashKey(d, key);
	for (int i = 0; i <= 1; i++){
		dictht *ht_ptr = &d->ht[i];
		int index = hashindex%(ht_ptr->size);
		dictEntry *p = ht_ptr->table[index];
		dictEntry *pre = NULL;
		while (p){
			if (d->type->keyCompare(p->key, key)){
				
				if (pre) pre->next = p->next;
				else  ht_ptr->table[index] = NULL;
	
				dictKeyFree(d, key);
				dictValFree(d, p->val);
				free(p);

				ht_ptr->used--;
			}

			pre = p;
			p = p->next;
		}
	}

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
		d->rehashidx++;//��Ҫ���Ǹ��������Ҫ�ı�־λ����������ת����һ����
	}

	if (d->ht[0].used == 0){//rehash���ʱҪע���table�л���ȥ��ht[0]������ht[1]ֻ���ڵ�����Сʱ��������ƿ�����֡��������Էֶ����ִ�����ݹ��̡�
		d->rehashidx = -1;
		free(d->ht[0].table);
		d->ht[0] = d->ht[1];
		d->ht[1].table = NULL;
		d->ht[1].size = 0;
		d->ht[1].used = 0;
		//��������ͨ�����ز�ͬ��ֵ��ȷ�������ڲ�ͬ��״̬��������ʱ���ԡ�
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

	while (dictRehash(d, 100)) {
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


