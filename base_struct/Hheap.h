#ifndef HHEAP_H
#define HHEAP_H

#define MAX_NODE_NUM  10000

typedef struct heapType
{
	int (*Campare)(void *value1,void *value2);
	void (*valFree)(void *value);
}heapType;

typedef struct Heap
{
	heapType *type;
	void **dataSpace;
	long size;
}Heap;

Heap* CreateHeap(heapType *type);
void* popHead(Heap *h); // where del  the node 
void  delHead(Heap *h);
void* getHead(Heap *h);
int addHeapVal(Heap *h,void *val);
int heapSize(Heap *h);
int isEmpty(Heap *h);

#endif