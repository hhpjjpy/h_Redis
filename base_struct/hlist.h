#ifndef HLIST
#define HLIST

typedef struct listNode
{
	struct listNode *m_prev;
	struct listNode *m_next;
	void *m_value;
}listNode;

typedef struct list
{
	listNode *m_head;
	listNode *m_tail;
	unsigned int length;
	int (*match)(void *ptr, void *key);//大于0 key大，小于0 key小
	void(*free)(void *ptr);
	void* (*dup)(void *ptr);
}list;

#define ListMatchFunSet(l,m)  ((l)->match = (m))
#define ListFreeFunSet(l,m) ((l)->free = (m))
#define ListDupFunSet(l,m) ((l)->dup = (m))

#define ListLength(l) ((l)->length)

list* CreateList(void);
void* listGetFrist(list *list);
void* listGetLast(list *list);
void listFreeFrist(list *list);
void listFreeLast(list *list);
list* listAddNodeHead(list *list, void *value);
list* listAddNodeTail(list *list,void *value);
list* listInsertNode(list *list, listNode *oldNode, void *value, int direction);
listNode* listSerchNode(list *list,void *key);
void listRemoveNode(list *list, listNode *node);
void  FreeList(list *list);

#endif






