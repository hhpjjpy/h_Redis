#include "hskiplist.h"

#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <time.h>

hSkipList* CreateSkipList(hSkipType *type)
{
	srand((unsigned)time(NULL));//设置随机数种子
	hSkipList *splist = (hSkipList*)malloc(sizeof(hSkipList));
	if (splist == NULL)  return NULL;
	splist->type = type;
	splist->maxLevel = MAXLEVEL;
	splist->head = (hSkipListNode*)malloc(sizeof(hSkipListNode));
	if (splist->head == NULL)  return NULL;
 	splist->head->level = MAXLEVEL;
	splist->head->power = 0;//0为权值最小节点
	splist->head->value = NULL;//无值
	splist->head->nextArr = (hSkipListNode**)malloc(MAXLEVEL*sizeof(hSkipListNode*));
	if (splist->head->nextArr == NULL) return NULL;
	memset(splist->head->nextArr, 0, MAXLEVEL*sizeof(hSkipListNode*));
	return splist;
}

static hSkipListNode** GetPreptrArr(hSkipList *splist, void *val)
{
	static hSkipListNode* ptrArr[MAXLEVEL];//设置为静态，避免每次都分配释放内存
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
	*可利用查找过程找出节点的前一个节点的指针数组，这样可实现O(lonN)的时间复杂度
	*/

	hSkipListNode **ptr = GetPreptrArr(splist,val);

	for (int i = 0; i < randLevel;i++){
		spNode->nextArr[i] = ptr[i]->nextArr[i];
		ptr[i]->nextArr[i] = spNode;
	}

	return 1;

	/*
	*由于插入后每层仍是一个有序列表，即若元素存在于某层则在该层中的位置是确定的。故插入操作相当于给每层的
	*单链表都添加一个相同的元素。
	*原实现为，遍历每个链表插入，这样的时间复杂度为O（n），不是最优解,有点傻逼了
	for (int i = 0; i < randLevel; i++) 
	{
		if (splist->head->nextArr[i] == NULL){
			splist->head->nextArr[i] = spNode;
			spNode->nextArr[i] = NULL;
			continue;
		}

		hSkipListNode *sp = splist->head->nextArr[i];
		hSkipListNode *spPre = NULL;
		while (sp->power <= spNode->power&&sp!=NULL){//默认按权值升序
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
