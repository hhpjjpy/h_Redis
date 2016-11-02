#include "Hheap.h"

#include <malloc.h>
#include <string.h>

Heap* CreateHeap(heapType *type)
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

//����ָ����������ݣ�Ҫʹ��ָ���ָ�롣**
void ptrSwap(void **ptr1,void **ptr2){ 
	void *temp = (*ptr1);
	*ptr1 = *ptr2;
	*ptr2 = temp;
	return;
}


void adjustUp(Heap *h,long i) //�ѵ����˵���
{
	if (i == 0) return;

	void **child = &(h->dataSpace[i]); //������Ӧ����λ���ж�Ӧ��ָ�룬Ӧ�ð�����λ����Ϊ������¼�����������������ֵ�� ��Ҫ���´�ע�⣡������������������������
	long rootIndex = (i-1) / 2;
	void **root = &(h->dataSpace[rootIndex]);
	
	if (h->type->Campare(*root,*child) > 0)
		ptrSwap(child,root);
	
	if (rootIndex == 0) return;
	
	adjustUp(h,rootIndex);
}

void adjustDown(Heap *h,long i) //����С�ѷ�ʽ������NULL �����
{
	if (i > h->size) return;

	void **root = &(h->dataSpace[i]);
	long leftindex = 2 * i + 1;
	long rightindex = 2 * i + 2;
	if (leftindex >= h->size) return;

	void **leftchild = &(h->dataSpace[leftindex]);
	void **rightchild = rightindex >= h->size ? NULL :&(h->dataSpace[rightindex]);
	
	void **objchild = leftchild; 
	int  nextindex = leftindex;

	if (rightchild != NULL && (h->type->Campare(*leftchild, *rightchild) > 0)){//����Һ��Ӹ�С����Ϊ�Һ���
		objchild = rightchild;
		nextindex = rightindex;
	}
	if (h->type->Campare(*root, *objchild) > 0)  ptrSwap(root, objchild);
	else
		return;

	adjustDown(h,nextindex);
}


void*  popHead(Heap *h)
{
	ptrSwap(&(h->dataSpace[0]), &(h->dataSpace[h->size - 1]));
	void *val = h->dataSpace[h->size-1];
	h->dataSpace[h->size - 1] = NULL;
	
	h->size--;
	adjustDown(h,0);

	return val;
}

void  delHead(Heap *h){
	ptrSwap(&(h->dataSpace[0]), &(h->dataSpace[h->size - 1]));
	void *val = h->dataSpace[h->size - 1];
	h->dataSpace[h->size - 1] = NULL;
	
	h->size--;
	adjustDown(h, 0);

	if (h->type->valFree != NULL)
		h->type->valFree(val);
	else
		free(val);
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

int heapSize(Heap *h)
{
	return h->size;
}

int isEmpty(Heap *h)
{
	return h->size == 0?1:0;
}

void freeHeap(Heap *h)
{
	free(h->dataSpace);
	free(h);
	h = NULL;
	return;
}


