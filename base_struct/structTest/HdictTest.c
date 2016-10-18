#include "../hdict.h"
#include "../hstring.h"
#include <stdio.h>

long long  simpleHashFunction(long long  key)
{
	long long hashkey = 0;
	while (key){
		hashkey += key % 10 * 13;
		key = key / 10;
		hashkey += key % 10 + 53;
		key = key / 10;
		hashkey += key % 10 * 17;
		key = key / 10;
	}

	return hashkey;
}


long long TestHashFunction(hstring *hs)
{
	long sum = 0;
	char *p = hs->ptr;
	int i = hs->length;
	while (p != '\0'&&i-->0){
		sum = sum * 10 + (int)(*p - '0');
		p = p + 1;
	}

	return simpleHashFunction(sum);
}

unsigned int hashfuntion(const void *key)
{
	hstring *hs = (hstring*)key;

	return TestHashFunction(hs);

}

void TestKeyFree(void *key)
{
	HsFree((hstring*)key);
}

void TestValFree(void *val)
{
	HsFree((hstring*)val);
}

int TestCompare(void *key1,void *key2)
{
	return HsCampare((hstring*)key1,(hstring*)key2);

}

int main()
{
	dictType *TestType = (dictType*)malloc(sizeof(dictType));
	TestType->hashFunction = hashfuntion;
	TestType->keyDup = NULL;
	TestType->keyFree = TestKeyFree;
	TestType->valFree = TestValFree;
	TestType->keyCompare = TestCompare;

	dict *TestDict = dictCreate(TestType);

	char *p1 = (char*)malloc(4 * sizeof(char));
	memcpy(p1, "hhp",3);
	p1[3] = '\0';
	hstring *hskey = CreateHstring(p1);

	char *p2 = (char*)malloc(10* sizeof(char));
	memcpy(p2, "testValue",9);
	p2[9] = '\0';
	hstring *hsval = CreateHstring(p2);

	if (dictAdd(TestDict, hskey, hsval) != 0)
		printf("add err\n");

	hstring *hs = (hstring*) dictFetchValue(TestDict, hskey);

	if (hs != NULL) printf("get val is : %s\n",hs->ptr);

}






