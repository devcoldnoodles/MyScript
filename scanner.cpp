#include "scanner.h"

using namespace script;

void Scanner::Scan(TokenDesc* desc)
{
    desc->value = GetSingleToken(desc);
}

ushort Scanner::Advance(bool increase_lines)
{
    if(increase_lines)  ++lines;
    ++pos;
    return 0;
}

ushort Scanner::GetSingleToken(TokenDesc* desc)
{
    switch (src[pos])
    {
    case '\r': case '\n':
        return Advance(true);
        break;
    case ' ': case '\f': case '\t': case '\v':
        return Advance();
    case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
        return ScanNumber(desc)->value;
    case '\"':
        return ScanString(desc)->value;
    case '\'':
        return ScanChar(desc)->value;
    default:
        break;
    }
    return 0;
}