#include"../hskiplist.h"

#include<stdio.h>
#include<malloc.h>
#include<stdlib.h>

typedef struct TestStruct
{
	int num1;
	int num2;

}TestStruct;

int SetPowerOfTest(void *val)
{
	TestStruct *ts = (TestStruct*)val;

	return ts->num1*ts->num2+1;
}

void TestFree(void *val)
{
	free(val);
}

void showAllSplist(hSkipList *splist)
{
	int  maxLevel = splist->maxLevel;
	for (int i = 0; i < maxLevel;i++){
		hSkipListNode *node = splist->head->nextArr[i];
		while (node != NULL){
			TestStruct *ts = (TestStruct*)node->value;
			printf("< %d  %d>  ", ts->num1, ts->num2);
			node = node->nextArr[i];
		}
		printf("\n");
	}

}

int main()
{
	hSkipType *testType = (hSkipType*)malloc(sizeof(hSkipType));
	testType->SetPower = SetPowerOfTest;
	testType->ValFree = TestFree;

	hSkipList *splist = CreateSkipList(testType);
	
	for (int i = 0; i < 100; i++){
		TestStruct *testVal = (TestStruct*)malloc(sizeof(TestStruct));
		testVal->num1 = i;
		testVal->num2 = i + 1;

		if (SkipListAdd(splist, testVal) == 0)
			printf("<%d %d> add ok\n",testVal->num1,testVal->num2);
		else
			printf("<%d %d> add err\n", testVal->num1, testVal->num2);
		showAllSplist(splist);
		printf("********************************************************************************************************************\n");
	}

	//showAllSplist(splist);

	TestStruct *testGetVal = (TestStruct*)malloc(sizeof(TestStruct));

	testGetVal->num1 = 55;
	testGetVal->num2 = 56;

	hSkipListNode *node = SkipListFind(splist,testGetVal);

	if (node != NULL){
		TestStruct *ts = (TestStruct*)(node->value);
		printf("<num1:%d   num2:%d> \n",ts->num1,ts->num2);
	}
	else{
		printf("find err\n");
	}

	free(testGetVal);

	TestStruct *testDeleteVal = (TestStruct*)malloc(sizeof(TestStruct));
	for (int i = 0; i < 50;i++)
	{
		testDeleteVal->num1 = i;
		testDeleteVal->num2 = i + 1;

		if (SkipListDelete(splist, testDeleteVal) == 0)
			printf("<%d %d> delete OK\n",testDeleteVal->num1,testDeleteVal->num2);
		else
			printf("<%d %d> delete err\n", testDeleteVal->num1, testDeleteVal->num2);

	}
	

	showAllSplist(splist);

	FreeSkipList(splist);

	printf("skiplist test over \n");
}

