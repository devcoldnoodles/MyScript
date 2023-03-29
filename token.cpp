#include "token.h"
#include <iostream>

using namespace myscript;

TokenDesc::TokenDesc(short value, unsigned int lines, Literal literal) : value(value),
                                                                         lines(lines),
                                                                         literal(literal)
{

}

TokenDesc::TokenDesc(short value, unsigned int lines, void *literal) : value(value),
                                                                       lines(lines)
{
    this->literal.p = literal;
}

TokenDesc::TokenDesc(short value, unsigned int lines, char literal) : value(value),
                                                                      lines(lines)
{
    this->literal.c = literal;
}

TokenDesc::TokenDesc(short value, unsigned int lines, char *literal) : value(value),
                                                                       lines(lines)
{
    this->literal.s = literal;
}

TokenDesc::TokenDesc(short value, unsigned int lines, long long literal) : value(value),
                                                                           lines(lines)
{
    this->literal.l = literal;
}

TokenDesc::TokenDesc(short value, unsigned int lines, double literal) : value(value),
                                                                        lines(lines)
{
    this->literal.d = literal;
}

TokenDesc::TokenDesc(TokenDesc &&other)
{
    *this = std::move(other);
}

TokenDesc &TokenDesc::operator=(TokenDesc &&other)
{
    if (this != &other)
    {
        this->value = other.value;
        this->lines = other.lines;
        this->literal = other.literal;
    }

    return *this;
}

const char *TokenDesc::GetInfo()
{
    return value == Token::IDENTIFIER || value == Token::LITERAL_STRING ? literal.s : tokenlist[value].str;
}
