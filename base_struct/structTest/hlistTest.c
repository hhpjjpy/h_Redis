#include <stdio.h>
#include <malloc.h>
#include<string.h>
#include "../hlist.h"

typedef struct Test{
	int a;
	int b;
	char *p;
}Test;


int TestMatch(void *ptr,void *key)
{
	Test *one = (Test*)ptr;
	Test *two = (Test*)key;
	
	int numa = one->a + one->b;
	int numb = two->a + two->b;

	if (numa == numb) return 0;
	
	return numa > numb ? -1 : 1;
}


void Testfree(void *ptr){
	Test *p = (Test*)ptr;
	free(p->p);
	free(p);
}


int main()
{
	list *TestList;
	TestList = CreateList();
	ListMatchFunSet(TestList, TestMatch);
	ListFreeFunSet(TestList,Testfree);

	for (int i = 0; i < 10;i++){
		for (int j = 0; j < 10;j++){
			Test *value = (Test*)malloc(sizeof(Test));
			value->a = i;
			value->b = j;
			value->p = (char*)malloc(10 * sizeof(char));
			memcpy(value->p, "test", 4);

			list *newList = listAddNodeTail(TestList, (void*)value);
			if (newList){
				printf("add ok\n");
			}

		}
	}

	for (int i = 11; i < 20; i++){
		for (int j = 11; j < 20; j++){
			Test *value = (Test*)malloc(sizeof(Test));
			value->a = i;
			value->b = j;
			value->p = (char*)malloc(10 * sizeof(char));
			memcpy(value->p, "test", 4);

			list *newList = listAddNodeHead(TestList, (void*)value);
			if (newList){
				printf("add ok\n");
			}

		}
	}


	Test key = { 8, 9, NULL };

	listNode *node = listSerchNode(TestList,(void*)&key);

	if (node){
		printf("get ok \n");
		Test *p = (Test*)node->m_value;
		printf("the Test struct Value is a:%d,b:%d",p->a,p->b);
		
		Test *value = (Test*)malloc(sizeof(Test));
		value->a = 100;
		value->b = 100;
		value->p = NULL;

		listInsertNode(TestList,node,(void*)value,1);
	}



	listNode *node_print1 = TestList->m_head;
	int index1 = 1;
	while (node_print1){
		Test *p = (Test*)node_print1->m_value;
		printf("index:%d  the Test struct Value is a:%d,b:%d\n", index1++, p->a, p->b);
		node_print1 = node_print1->m_next;
	}




	Test key_de = { 100, 100, NULL };

	listNode *node_check = listSerchNode(TestList, (void*)&key_de);

	if (node_check){
		printf("get ok 2\n");
		Test *p = (Test*)node_check->m_value;
		printf("the Test struct Value is a:%d,b:%d", p->a, p->b);
	}

	listRemoveNode(TestList, node_check);

	listNode *node_print = TestList->m_head;
	int index = 1;
	while (node_print){
		Test *p = (Test*)node_print->m_value;
		printf("index:%d  the Test struct Value is a:%d,b:%d\n",index++,p->a, p->b);
		node_print = node_print->m_next;
	}


	printf("list length : %d ",TestList->length);

	FreeList(TestList);

	printf("is test over \n");

}
