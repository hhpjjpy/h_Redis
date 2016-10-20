#ifndef HSKIPLIST_H
#define HSKIPLIST_H
typedef struct hSkipListNode
{
	void *value;
	unsigned int power;//Ȩֵ
	unsigned int level;
	struct hSkipListNode *nextArr[1];

}hSkipListNode;

typedef struct hSkipType
{
	int(*SetPower)(void *val);//���Ը�����Ӧ������������Ȩֵ
	void(*ValFree)(void *val);

}hSkipType;

typedef struct hSkipList
{
	unsigned int maxLevel;
	hSkipListNode *head[1];
	hSkipListNode *tail[1];
}hSkipList;


hSkipList* CreateSkipList(hSkipType *type);
int SkipListAdd(hSkipList *splist,void *val);
hSkipListNode* SkipListFind(hSkipList *splist,void *val);
int SkipListDelete(hSkipList *splist,void *val);
void FreeSkipList(hSkipList *splist);

#endif









