#ifndef __TOKEN_H__
#define __TOKEN_H__
#include <stdlib.h>
#include <string.h>

typedef struct TokenDesc
{
	short value;
	size_t line;
	const char* literal;
	struct TokenDesc* next;
} TokenDesc;

TokenDesc* Scan(const char* src);

#endif