#include "swap.h"

void Swap(char *left, char *right)
{
	char middle=*left;
	*left=*right;
	*right=middle;
}
