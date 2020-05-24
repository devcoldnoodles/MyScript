#include "token.h"

using namespace script;

struct ScanDesc
{
    char32_t* src;
    size_t pos;
    size_t lines;
};

TokenDesc* script::Scan(char32_t* src)
{
    ScanDesc desc{src};
}

static void Advance(ScanDesc* desc)
{
    switch (desc->src[desc->pos])
    {
    case '\r': case '\n':
        ++desc->lines;
        break;
    case ' ': case '\f': case '\t': case '\v':
        break;
    default:
        break;
    }
    ++desc->pos;
}

static TokenDesc* ScanNumber(ScanDesc* desc)
{
    if(desc->src[desc->pos])
    ;
}

template<typename T>
TokenDesc* basic_scan(T* src)
{
    TokenDesc* head = (TokenDesc*)malloc(sizeof(TokenDesc));
    TokenDesc* pointer = head;
    char32_t a;
    while(*src)
    {
        
    }
    return head;
}