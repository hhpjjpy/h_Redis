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

typedef  struct parArg
{
	int number;
	void *argv[3];
}parArg;

void addCmdFun(dict * d,parArg *ptr)
{
	int ret;
	if (ptr->number != 3){
		printf("invalid parameter \n");
	}
	if ((ret = dictAdd(d, ptr->argv[1], ptr->argv[2])) == 0)
		printf("add ok \n");
	else
		printf("add err\n");
}


void deleteCmdFun(dict *d,parArg *ptr)
{
	if (ptr->number != 2)
		printf("invalid parameter \n");
	dictDelete(d,ptr->argv[1]);

	if (dictFind(d, ptr->argv[1]) == NULL)
		printf("delete OK");
	else
		printf("delete Err");

}

void getCmdFun(dict *d, parArg *ptr)
{
	if (ptr->number != 2)
		printf("invalid parameter \n");
	dictEntry *p = NULL;
	if ((p = dictFind(d, ptr->argv[1])) != NULL){
		hstring *hskey = (hstring*)p->key;
		hstring *hsval = (hstring*)p->val;

		printf("key: %s     val: %s\n", hskey->ptr, hsval->ptr);
	}
	else{
		printf("get val is empty \n");
	}
}

void replaceCmdFun(dict *d,parArg *ptr)
{
	if (ptr->number != 3)
		printf("invalid parameter\n");
	if (dictReplace(d, ptr->argv[1], ptr->argv[2]) == 0)
		printf("replace OK \n");
	else
		printf("replace err \n");
}

void DealRehash(dict *d,parArg *ptr)
{
	dictRehashMillisenconeds(d, 10);

	printf("rehash ok \n");
}

void shwoInfo(dict *d,parArg *ptr)
{
	int used = d->ht[0].used + d->ht[1].used;
	int isRehash = d->rehashidx;
	
	for (int i = 0; i <= 1; i++)
	{
		dictht ht = d->ht[i];
		for (int j = 0; j < ht.size;j++){
			dictEntry *entry = ht.table[j];
			while (entry){
				hstring *hskey = (hstring*)entry->key;
				hstring *hsval = (hstring*)entry->val;

				printf("key: %s     val: %s\n", hskey->ptr, hsval->ptr);

				entry = entry->next;
			}
		}
	}

	printf("rehashing ? %d\n",isRehash);
	printf("used number is : %d \n",used);
	printf("size of use table %d\n", isRehash == -1 ? d->ht[0].size:d->ht[1].size);
}

void  getParameter(parArg *par)
{
	char agr1[80], agr2[80], agr3[80];
	scanf("%s", agr1);

	char *cmd = (char*)malloc((strlen(agr1)+1)* sizeof(char));
	memcpy(cmd, agr1, strlen(agr1));
	cmd[strlen(agr1)] = '\0';
	par->argv[0] = cmd;
	par->number = 0;
	
	if (strcmp(agr1, "add") == 0){
		scanf("%s",agr2);
		scanf("%s",agr3);

		char *charkey1 = (char*)malloc((strlen(agr2) + 1)* sizeof(char));
		memcpy(charkey1, agr2, strlen(agr2));
		charkey1[strlen(agr2)] = '\0';

		char *charkey2 = (char*)malloc((strlen(agr3) + 1)* sizeof(char));
		memcpy(charkey2, agr3, strlen(agr3));
		charkey2[strlen(agr3)] = '\0';


		hstring *hskey = CreateHstring(charkey1);
		hstring *hsval = CreateHstring(charkey2);

		par->number = 3;
		par->argv[1] = hskey;
		par->argv[2] = hsval;
	}
	else if (strcmp(agr1, "delete") == 0){
		scanf("%s", agr2);
		
		char *charkey1 = (char*)malloc((strlen(agr2) + 1)* sizeof(char));
		memcpy(charkey1, agr2, strlen(agr2));
		charkey1[strlen(agr2)] = '\0';


		hstring *hskey = CreateHstring(charkey1);

		par->number = 2;
		par->argv[1] = hskey;

	}
	else if (strcmp(agr1, "get") == 0){
		scanf("%s", agr2);


		char *charkey1 = (char*)malloc((strlen(agr2) + 1)* sizeof(char));
		memcpy(charkey1, agr2, strlen(agr2));
		charkey1[strlen(agr2)] = '\0';


		hstring *hskey = CreateHstring(charkey1);

		par->number = 2;
		par->argv[1] = hskey;
	}
	else if (strcmp(agr1, "replace") == 0){
		scanf("%s", agr2);
		scanf("%s", agr3);

		char *charkey1 = (char*)malloc((strlen(agr2) + 1)* sizeof(char));
		memcpy(charkey1, agr2, strlen(agr2));
		charkey1[strlen(agr2)] = '\0';

		char *charkey2 = (char*)malloc((strlen(agr3) + 1)* sizeof(char));
		memcpy(charkey2, agr3, strlen(agr3));
		charkey2[strlen(agr3)] = '\0';


		hstring *hskey = CreateHstring(charkey1);
		hstring *hsval = CreateHstring(charkey2);

		par->number = 3;
		par->argv[1] = hskey;
		par->argv[2] = hsval;
	}
	else if (strcmp(agr1, "rehash") == 0){
		par->number = 1;
		return ;
	}
	else if (strcmp(agr1, "info") == 0){
		par->number = 1;
		return ;
	}
	else if (strcmp(agr1, "exit") == 0){
		par->number = 1;
		return ;
	}
	else{
		par->number = 0;
	}
}

typedef  void(*CmdFunction)(dict *d, parArg *par);

typedef struct dictCmd{
	char *strCmd;
	CmdFunction *fun;
}dictCmd;


dictCmd  TestCmd[] =
{
	{ "add", addCmdFun },
	{ "delete", deleteCmdFun },
	{ "get", getCmdFun },
	{ "replace", replaceCmdFun },
	{ "rehash", DealRehash },
	{ "info", shwoInfo }
};


void eatPar(dict *d,parArg *par)
{
	if (par->number == 0){
		printf("invilid par \n");
		return;
	}

	int allCmdNum = sizeof(TestCmd) / sizeof(dictCmd);
	for (int i = 0; i < allCmdNum; i++){
		if (strcmp(TestCmd[i].strCmd, par->argv[0]) == 0){
			CmdFunction cmdfun = TestCmd[i].fun;
			cmdfun(d,par);
			return ;
		}
	}

	printf("not find cmd error \n");

}

void printfHelp()
{
	printf("**********************************************************\n");
	printf("can test cmd : \n");
	printf("add key value \n");
	printf("delete key \n");
	printf("get key \n");
	printf("replace key value \n");
	printf("rehash \n");
	printf("info \n");
	printf("exit \n");
	printf("**********************************************************\n");
}


int main()
{
	printfHelp();

	dictType *TestType = (dictType*)malloc(sizeof(dictType));
	TestType->hashFunction = hashfuntion;
	TestType->keyDup = NULL;
	TestType->keyFree = TestKeyFree;
	TestType->valFree = TestValFree;
	TestType->keyCompare = TestCompare;

	dict *TestDict = dictCreate(TestType);

	parArg  *par = (parArg*)malloc(sizeof(parArg));



	while (1){
		getParameter(par);
		if (par->number == 0){
			printf("invalid parameter \n");
			continue;
		}
		if (strcmp(par->argv[0], "exit") == 0)
			return -1;


		eatPar(TestDict,par);

	}

}






