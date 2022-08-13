#include "token.h"

using namespace myscript;

TokenDesc::TokenDesc(short value, unsigned int lines, void *literal) : value(value),
                                                                       lines(lines)
{
    this->literal.p = literal;
}

TokenDesc::~TokenDesc()
{
    switch (value)
    {
    case Token::IDENTIFIER:
    case Token::LITERAL_STRING:
        delete literal.s;
        break;
    default:
        break;
    }
}

const char *TokenDesc::GetInfo()
{
    if (value == Token::IDENTIFIER ||
        value == Token::LITERAL_STRING)
        return literal.s;

    return tokenlist[value].str;
}
