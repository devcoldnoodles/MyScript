#include "scanner.h"

using namespace script;

void Scanner::Scan(TokenDesc* desc)
{
    while(src[pos])
    {
        // desc->value = GetSingleToken(desc);
        // desc = desc->next;
        // desc = desc->next = new TokenDesc;
    }
}

short Scanner::Advance(bool increase_lines)
{
    if(increase_lines)  ++lines;
    ++pos;
    return 0;
}
bool Scanner::Advance(const char* dest)
{
    size_t temp = pos;
    
}
short Scanner::Advance(char32_t cond, short true_sign, short false_sign)
{
    return src[++pos] == cond ? true_sign : false_sign;
}

short Scanner::GetSingleToken(TokenDesc* desc)
{
    switch (src[pos])
    {
    case '\r': case '\n':
        return Advance(true);
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
TokenDesc* Scanner::ScanString(TokenDesc* desc)
{
    std::string temp;
    while(src[++pos])
    {
        switch(src[pos])
        {
            case '\n':
                ++lines;
                break;
            case '\\':
                switch (src[++pos])
                {
                case '\\':
                    temp += '\\';
                    break;
                case 't':
                    temp += '\t';
                    break;
                case 'r':
                    temp += '\r';
                    break;
                case 'n':
                    temp += '\n';
                    break;
                case '\'':
                    temp += '\'';
                    break;
                case '\"':
                    temp += '\"';
                    break;
                case '\f': 
                    temp += '\f';
                    break;
                case '\t': 
                    temp += '\t';
                    break;
                case '\v':
                    temp += '\v';
                    break;
                }
                break;
            case '\"':
                desc->next = new TokenDesc{Token::LITERAL, temp};
                return desc->next;
            default:
                temp += src[pos];
                break;
        }
    }
}
TokenDesc* Scanner::ScanNumber(TokenDesc* desc)
{

}
TokenDesc* Scanner::ScanChar(TokenDesc* desc)
{

}