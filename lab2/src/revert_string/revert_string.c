#include "revert_string.h"
#include <stdio.h>

void RevertString(char *str)
{
	char temp;
	char* str2=str;
	while(*str2!='\0')
	{
		str2++;
	}
	str2--;
	while(str<str2)
	{
		temp=*str;
		*str=*str2;
		*str2=temp;
		str2--;
		str++;
	}
	
}

