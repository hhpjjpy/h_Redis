#include "../Hheap.h"

#include <stdio.h>
#include <malloc.h>

int intCampare(void *val1,void *val2)
{
	return *((int*)val1) - *((int*)val2);
}

int  main()
{
	heapType *type = (heapType*)malloc(sizeof(heapType));
	type->Campare = intCampare;
	type->valFree = NULL;

	Heap *h = CreateHeap(type);

	int num = 0;
	scanf("%d",&num);
	while (num != -1)
	{
		int *ptrint = (int*)malloc(sizeof(int));
		*ptrint = num;
		if (addHeapVal(h, ptrint))  printf("add to heap error \n");

		scanf("%d", &num);
	}

	while (!isEmpty(h))
	{
		printf("%d\n",*((int*)getHead(h)));
		delHead(h);
	}

}