#include "../hstring.h"

#include <stdio.h>

int main()
{
	char *p_const = "hello world";
	char *ptr = (char*)malloc(12 * sizeof(char));
	
	memcpy(ptr, p_const, 11 * sizeof(char));
	ptr[11] = '\0';
	
	hstring *hs = CreateHstring(ptr);

	int length = GETHSTRINGLENGTH(hs);

	printf("hs length is : %d \n ",length);
	printf("hs value is : %s \n",hs->ptr);
	
	hstring *newhs = spliceChar(hs,p_const);

	if (!newhs) return  -1;

	int newlenth = GETHSTRINGLENGTH(newhs);

	printf("new hs length is : %d \n",newlenth);
	printf("new hs value is : %s \n",newhs->ptr);

}
