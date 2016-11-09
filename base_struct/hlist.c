#include "hlist.h"

#include <stdlib.h>
#include <malloc.h>

list* CreateList(void)
{
	list *l = (list*)malloc(sizeof(list));

	if (!l) return NULL;

	l->m_head = NULL;
	l->m_tail = NULL;
	l->length = 0;
	l->match = NULL;
	l->free = NULL;
	l->dup = NULL;
	return l;
}

void* listGetFrist(list *list)
{
	if (list->m_head == NULL) return NULL;
	return list->m_head->m_value;
}

void* listGetLast(list *list)
{
	if (list->m_tail == NULL) return NULL;
	return list->m_tail->m_value;
}

void listFreeFrist(list *list)
{
	if (list->m_head == NULL) return;
	listNode *temp = list->m_head;
	list->m_head = list->m_head->m_next;
	if (list->m_head!=NULL)
		list->m_head->m_prev = NULL;
	if (list->free == NULL)
		free(temp->m_value);
	else
		list->free(temp->m_value);
	free(temp);

	list->length--;
}

void listFreeLast(list *list){
	if (list->m_tail == NULL) return;
	listNode *temp = list->m_tail;
	list->m_tail = list->m_tail->m_prev;
	if (list->m_tail != NULL)
		list->m_tail->m_next = NULL;
	if (list->free == NULL)
		free(temp->m_value);
	else
		list->free(temp->m_value);
	free(temp);

	list->length--;
}

list* listAddNodeHead(list *list, void *value)
{
	listNode *node;
	if ((node = (listNode*)malloc(sizeof(listNode))) == NULL)
		return NULL;
	
	node->m_value = value;
	node->m_next = NULL;
	node->m_prev = NULL;

	if (!list->m_head){
		list->m_head = node;
		list->m_tail = node;
	}
	else{
		node->m_next = list->m_head;
		node->m_next->m_prev = node;
		list->m_head = node;
		node->m_prev = NULL;
	}
	list->length++;

	return list;
}


list* listAddNodeTail(list *list, void *value)
{
	listNode *node;
	if ((node = (listNode*)malloc(sizeof(listNode))) == NULL)
		return NULL;

	node->m_value = value;
	node->m_prev = NULL;
	node->m_next = NULL;

	if (!list->m_tail){
		list->m_head = node;
		list->m_tail = node;
	}
	else{
		list->m_tail->m_next = node;
		node->m_next = NULL;
		node->m_prev = list->m_tail;
		list->m_tail = node;
	}
	list->length++;

	return list;
}

list* listInsertNode(list *list, listNode *oldNode,void *value,int direction)
{
	listNode *node;
	if ((node = (listNode*)malloc(sizeof(listNode))) == NULL)
		return NULL;

	node->m_value = value;
	node->m_prev = NULL;
	node->m_next = NULL;

	if (direction > 0){//向后方向
		if (!oldNode->m_next){
			oldNode->m_next = node;
			node->m_prev = oldNode;
			list->m_tail = node;
		}
		else{
			node->m_next = oldNode->m_next;
			oldNode->m_next = node;
			node->m_next->m_prev = node;
			node->m_prev = oldNode;
		}
	}
	else{
		if (!oldNode->m_prev){
			oldNode->m_prev = node;
			node->m_next = oldNode;
			list->m_head = node;
		}
		else{
			node->m_prev = oldNode->m_prev;
			oldNode->m_prev = node;
			node->m_prev->m_next = node;
			node->m_next = oldNode;
		}
	}

	list->length++;

	return list;
}

listNode* listSerchNode(list *list, void *key)
{
	listNode *node = list->m_head;
	while (node){
		if (list->match){
			if (list->match(node->m_value, key) == 0)  return node;
		}
		else{
			if (node->m_value == key) return node;
		}

		node = node->m_next;
	}

	return NULL;
}

void listRemoveNode(list *list,listNode *node)
{
	if (!node->m_next){
		node->m_prev->m_next = NULL;
		list->m_tail = node->m_prev;
	}
	else if(!node->m_prev){
		node->m_next->m_prev = NULL;
		list->m_head = node->m_next;
	}
	else{
		node->m_next->m_prev = node->m_prev;
		node->m_prev->m_next = node->m_next;
	}

	if (list->free)
		list->free(node->m_value);
	else
		free(node->m_value);
	free(node);
	list->length--;
}

void  FreeList(list *list)
{
	listNode *node = list->m_head;
	while (node){
		listNode *freeNode = node;
		node = node->m_next;

		if (list->free)
			list->free(freeNode->m_value);
		else
			free(freeNode->m_value);
		free(freeNode);
	}
	free(list);
}
