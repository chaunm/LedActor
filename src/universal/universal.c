#include "universal.h"
#include <string.h>
#include <stdlib.h>

VOID CopyMemory (PBYTE pDest, PBYTE pSource, BYTE nLength)
{
	BYTE nIndex;
	if ((pDest == NULL) || (pSource == NULL))
			return;
	for (nIndex = 0; nIndex < nLength; nIndex++)
	{
		pDest[nIndex] = pSource[nIndex];
	}
}


char* StrDup(const char* string)
{
	char* pDest = malloc(strlen(string) + 1);
	memset(pDest, 0, strlen(string) + 1);
	memcpy(pDest, string, strlen(string));
	return pDest;
}

