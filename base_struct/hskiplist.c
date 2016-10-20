#include "hskiplist.h"

#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>

hSkipList* CreateSkipList(hSkipType *type)
{
	srand((unsigned)time(NULL));//�������������
	hSkipList *splist = (hSkipList*)malloc(sizeof(hSkipList));
	if (splist == NULL)  return NULL;
	splist->type = type;
	splist->maxLevel = MAXLEVEL;
	splist->head = (hSkipListNode*)malloc(sizeof(hSkipListNode));
	if (splist->head == NULL)  return NULL;
 	splist->head->level = MAXLEVEL;
	splist->head->power = 0;//0ΪȨֵ��С�ڵ�
	splist->head->value = NULL;//��ֵ
	splist->head->nextArr = (hSkipListNode**)malloc(MAXLEVEL*sizeof(hSkipListNode*));
	if (splist->head->nextArr == NULL) return NULL;
	memset(splist->head->nextArr, 0, MAXLEVEL*sizeof(hSkipListNode*));
	return splist;
}

static hSkipListNode** GetPreptrArr(hSkipList *splist, void *val)
{
	static hSkipListNode* ptrArr[MAXLEVEL];//����Ϊ��̬������ÿ�ζ������ͷ��ڴ�
	hSkipListNode *sptr = splist->head;
	unsigned int power = splist->type->SetPower(val);
	int i = splist->maxLevel;
	while (i-- > 0){
		while (sptr->nextArr[i] != NULL&&sptr->nextArr[i]->power < power)
			sptr = sptr->nextArr[i];
		ptrArr[i] = sptr;
	}
	
	return ptrArr;

}

int SkipListAdd(hSkipList *splist,void *val)
{
	int randLevel = (rand() % (splist->maxLevel));

	hSkipListNode *spNode = (hSkipListNode*)malloc(sizeof(hSkipListNode));
	if (spNode == NULL) return -1;

	spNode->level = randLevel;
	
	spNode->nextArr = (hSkipListNode**)malloc(randLevel * sizeof(hSkipListNode*));
	if (spNode == NULL) return -1;

	memset(spNode->nextArr,0,randLevel*sizeof(hSkipListNode*));
	spNode->power = splist->type->SetPower(val);
	spNode->value = val;

	/*
	*�����ò��ҹ����ҳ��ڵ��ǰһ���ڵ��ָ�����飬������ʵ��O(lonN)��ʱ�临�Ӷ�
	*/

	hSkipListNode **ptr = GetPreptrArr(splist,val);

	for (int i = 0; i < randLevel;i++){
		spNode->nextArr[i] = ptr[i]->nextArr[i];
		ptr[i]->nextArr[i] = spNode;
	}

	return 1;

	/*
	*���ڲ����ÿ������һ�������б�����Ԫ�ش�����ĳ�����ڸò��е�λ����ȷ���ġ��ʲ�������൱�ڸ�ÿ���
	*���������һ����ͬ��Ԫ�ء�
	*ԭʵ��Ϊ������ÿ��������룬������ʱ�临�Ӷ�ΪO��n�����������Ž�,�е�ɵ����
	for (int i = 0; i < randLevel; i++) 
	{
		if (splist->head->nextArr[i] == NULL){
			splist->head->nextArr[i] = spNode;
			spNode->nextArr[i] = NULL;
			continue;
		}

		hSkipListNode *sp = splist->head->nextArr[i];
		hSkipListNode *spPre = NULL;
		while (sp->power <= spNode->power&&sp!=NULL){//Ĭ�ϰ�Ȩֵ����
			spPre = sp;
			sp = sp->nextArr[i];
		}
		if (spPre == NULL){
			spNode->nextArr[i] = splist->head->nextArr[i];
			splist->head->nextArr[i] = spNode;
		}
		else{
			spNode->nextArr[i] = spPre->nextArr[i];
			spPre->nextArr[i] = spNode;
		}
	}
	*/
}


hSkipListNode* SkipListFind(hSkipList *splist,void *val)
{
	hSkipListNode *sptr = splist->head;
	unsigned int power = splist->type->SetPower(val);
	int i = splist->maxLevel;
	while (i-- > 0){
		while (sptr->nextArr[i]!=NULL&&sptr->nextArr[i]->power < power)
			sptr = sptr->nextArr[i];
		if (sptr->nextArr[i]!=NULL&&sptr->nextArr[i]->power == power) return sptr->nextArr[i];
	}
	
	return NULL;
}

void* SkipListFetchValue(hSkipList *splist, void *val)
{
	hSkipListNode *sptr = SkipListFind(splist,val);

	return sptr == NULL ? sptr : sptr->value;
}

int SkipListDelete(hSkipList *splist,void *val)
{
	hSkipListNode *nowNode = SkipListFind(splist, val);
	if (nowNode == NULL)  return -1;
	hSkipListNode **ptrArr = GetPreptrArr(splist,val);
	int level = nowNode->level;
	for (int i = 0; i < level;i++){
		ptrArr[i]->nextArr[i] = nowNode->nextArr[i];
	}

	splist->type->ValFree(nowNode->value);
	free(nowNode->nextArr);
	free(nowNode);

	return 0;
 
}

void FreeSkipList(hSkipList *splist)
{
	hSkipListNode *p = splist->head->nextArr[0];
	while (p!=NULL){
		hSkipListNode *pre = p;
		p = p->nextArr[0];

		splist->type->ValFree(p->value);
		free(p->nextArr);
		free(p);
	}

	free(splist->head->nextArr);
	free(splist->head);
	
	splist = NULL;
}
