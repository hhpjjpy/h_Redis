#include "Hheap.h"

#include <malloc.h>
#include <string.h>

Heap* initHeap(heapType *type)
{
	Heap *h = (Heap*)malloc(sizeof(Heap));
	memset(h,0,sizeof(Heap));
	if (h == NULL) return NULL;
	h->type = type;
	h->size = 0;
	h->dataSpace = malloc(sizeof(void*)*MAX_NODE_NUM);
	memset(h->dataSpace,0,sizeof(void*)*MAX_NODE_NUM);
	if (h == NULL) goto error;
	return h;
error:
	free(h);
	return NULL;
}

void ptrSwap(void **ptr1,void** ptr2){
	void *temp = (*ptr1);
	*ptr1 = *ptr2;
	*ptr2 = temp;
	return;
}

void adjustUp(Heap *h,long i) //堆的上滤调整
{
	if (i == 0) return;

	void *child = h->dataSpace[i];
	long rootIndex = (i-1) / 2;
	void *root = h->dataSpace[rootIndex];
	
	if (h->type->Campare(root,child) > 0)
		ptrSwap(&child,&root);
	
	if (rootIndex == 0) return;
	
	adjustUp(h,rootIndex);
}

void adjustDown(Heap *h,long i) //以最小堆方式建立，NULL 无穷大
{
	if (i > h->size) return;

	void *root = h->dataSpace[i];
	long leftindex = 2 * i + 1;
	long rightindex = 2 * i + 2;
	if (leftindex > h->size) return;

	void *leftchild = h->dataSpace[leftindex];
	void *rightchild = rightindex > h->size ? NULL : h->dataSpace[rightindex];
	
	void *objchild = leftchild; 
	int  nextindex = leftindex;

	if (rightchild != NULL && (h->type->Campare(leftchild, rightchild) > 0)){//如果右孩子更小，则为右孩子
		objchild = rightchild;
		nextindex = rightindex;
	}
	if (h->type->Campare(root, objchild) > 0)  ptrSwap(&root,objchild);

	adjustDown(h,nextindex);
}


void*  popHead(Heap *h)
{
	ptrSwap(&(h->dataSpace[0]), &(h->dataSpace[h->size - 1]));
	void *val = h->dataSpace[h->size-1];
	h->size--;
	h->dataSpace[h->size - 1] = NULL;
	adjustDown(h,0);

	return val;
}

void* getHead(Heap *h)
{
	return h->dataSpace[0];
}

int addHeapVal(Heap *h,void *val)
{
	if (h->size >= MAX_NODE_NUM) return -1;
	h->dataSpace[h->size] = val;
	adjustUp(h,h->size);
	h->size++;
	return 0;
}

int isEmpty(Heap *h)
{
	return h->size;
}



