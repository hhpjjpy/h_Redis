/*
¼òµ¥·â×°×Ö·û´®²Ù×÷
*/
#ifndef HSTRING_H
#define HSTRING_H

#include <stdlib.h>
#include <malloc.h>
#include <string.h>

typedef struct hstring
{
	char *ptr;
	int length;
}hstring;

#define GETHSTRINGLENGTH(hs)  (hs->length)

int getCharLength(char *ptr)
{
	int length = 0;
	while (*ptr != '\0')
	{
		length++;
		ptr++;
	}

	return length;
}


hstring* CreateHstring(char *ptr)
{
	hstring *hs = (hstring*)malloc(sizeof(hstring));
	hs->ptr = ptr;
	hs->length = getCharLength(ptr);

	return hs;
}


void setHstring(hstring *hs, char *ptr)
{
	if (hs->ptr == ptr) return ;

	if (!hs->ptr)
	{
		hs->ptr = ptr;
		hs->length = getCharLength(ptr);
	}
	else{
		free(hs->ptr);
		hs->ptr = ptr;
		hs->length = getCharLength(ptr);
	}
}

hstring* spliceChar(hstring *hs,char *ptr)
{
	int charLength = getCharLength(ptr);
	int length = hs->length + charLength;

	char *newptr = (char*)realloc(hs->ptr, length*sizeof(char)+1);
	if (!newptr) return NULL;
	else{
		memcpy(newptr + (hs->length), ptr, charLength*sizeof(char));
		newptr[length] = '\0';
		hs->length = length;
		hs->ptr = newptr;
	}

	return hs;
}

void FreeHstring(hstring *hs){
	if (hs->ptr) free(hs->ptr);
	free(hs);
 }


#endif