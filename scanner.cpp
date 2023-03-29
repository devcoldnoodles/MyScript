#include "scanner.h"
#include <string.h>

using namespace myscript;

bool Scanner::Tokenize(const char *src, std::vector<TokenDesc> &result)
{
    unsigned int lines = 1;
    while (*src)
    {
        switch (*src)
        {
        case '\n':
            ++lines;
            break;
        case '\r':
        case '\f':
        case '\t':
        case '\v':
        case ' ':
            break;
        case '0':
            if (*(src + 1) == 'x')
            {
                int integerValue = 0;
                src += 2;
                while (*src)
                {
                    if (*src >= '0' && *src <= '9')
                    {
                        integerValue *= 16;
                        integerValue += *src - '0';
                    }
                    else if (*src >= 'A' && *src <= 'F')
                    {
                        integerValue *= 16;
                        integerValue += *src - '0';
                    }
                    else
                    {
                        break;
                    }
                    ++src;
                }
                result.push_back(TokenDesc(Token::LITERAL_INTEGER, lines, (void*)integerValue));
                break;
            }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            short value = Token::LITERAL_INTEGER;
            Literal literal;
            int integerValue = 0;
            double floatValue = .0;
            while (*src >= '0' && *src <= '9')
            {
                integerValue *= 10;
                integerValue += *src - '0';
                ++src;
            }
            literal.l = integerValue;
            if (*src == '.')
            {
                double decimalIndex = 1;
                while (*src >= '0' && *src <= '9')
                {
                    decimalIndex *= 0.1;
                    floatValue += (*src - '0') * decimalIndex;
                    ++src;
                }
                value = Token::LITERAL_FLOAT;
                literal.d = integerValue + floatValue;
            }
            result.push_back(TokenDesc(value, lines, literal));
            continue;
        }
        break;
        case '\"':
        {
            const size_t capcity = 1024 * 1024;
            char temp[capcity];
            int pos = 0;
            while (*++src != '\"')
            {
                switch (*src)
                {
                case '\\':
                    switch (*(++src))
                    {
                    case '\\':
                        temp[pos++] = '\\';
                        break;
                    case 't':
                        temp[pos++] = '\t';
                        break;
                    case 'r':
                        temp[pos++] = '\r';
                        break;
                    case 'n':
                        temp[pos++] = '\n';
                        break;
                    case '\'':
                        temp[pos++] = '\'';
                        break;
                    case '\"':
                        temp[pos++] = '\"';
                        break;
                    }
                    break;
                default:
                    temp[pos++] = *src;
                    break;
                }
            }

            char *literal = new char[pos + 1];
            memcpy(literal, temp, pos);
            literal[pos] = 0;
            result.push_back(TokenDesc(Token::LITERAL_STRING, lines, literal));
        }
        break;
        case '\'':
        {
            char temp = *++src;
            if (temp == '\\')
            {
                switch (*(++src))
                {
                case '\\':
                    temp = '\\';
                    break;
                case 't':
                    temp = '\t';
                    break;
                case 'r':
                    temp = '\r';
                    break;
                case 'n':
                    temp = '\n';
                    break;
                case '\'':
                    temp = '\'';
                    break;
                case '\"':
                    temp = '\"';
                    break;
                default:
                    temp = '\\';
                    break;
                }
            }
            result.push_back(TokenDesc(Token::LITERAL_STRING, lines, temp));
        }
        break;
        case ':':
            result.push_back(TokenDesc(Token::COLON, lines, NULL));
            break;
        case ';':
            result.push_back(TokenDesc(Token::SEMICOLON, lines, NULL));
            break;
        case '.':
            result.push_back(TokenDesc(Token::PERIOD, lines, NULL));
            break;
        case ',':
            result.push_back(TokenDesc(Token::COMMA, lines, NULL));
            break;
        case '?':
            result.push_back(TokenDesc(Token::CONDITIONAL, lines, NULL));
            break;
        case '(':
            result.push_back(TokenDesc(Token::LPAREN, lines, NULL));
            break;
        case ')':
            result.push_back(TokenDesc(Token::RPAREN, lines, NULL));
            break;
        case '{':
            result.push_back(TokenDesc(Token::LBRACE, lines, NULL));
            break;
        case '}':
            result.push_back(TokenDesc(Token::RBRACE, lines, NULL));
            break;
        case '[':
            result.push_back(TokenDesc(Token::LBRACKET, lines, NULL));
            break;
        case ']':
            result.push_back(TokenDesc(Token::RBRACKET, lines, NULL));
            break;
        case '<':
        {
            short value = Token::LT;
            if (*(src + 1) == '=')
            {
                value = Token::LE;
                ++src;
            }
            result.push_back(TokenDesc(value, lines, NULL));
        }
        break;
        case '>':
        {
            short value = Token::GT;
            if (*(src + 1) == '=')
            {
                value = Token::GE;
                ++src;
            }
            result.push_back(TokenDesc(value, lines, NULL));
        }
        break;
        case '!':
        {
            short value = Token::NOT;
            if (*(src + 1) == '=')
            {
                value = Token::NEQ;
                ++src;
            }
            result.push_back(TokenDesc(value, lines, NULL));
        }
        break;
        case '=':
        {
            short value = Token::ASSIGN;
            if (*(src + 1) == '=')
            {
                value = Token::EQ;
                ++src;
            }
            result.push_back(TokenDesc(value, lines, NULL));
        }
        break;
        case '+':
        {
            short value = Token::ADD;
            switch (*(src + 1))
            {
            case '=':
                value = Token::ASSIGN_ADD;
                ++src;
                break;
            case '+':
                value = Token::INC;
                ++src;
                break;
            }
            result.push_back(TokenDesc(value, lines, NULL));
        }
        break;
        case '-':
        {
            short value = Token::SUB;
            switch (*(src + 1))
            {
            case '=':
                value = Token::ASSIGN_SUB;
                ++src;
                break;
            case '-':
                value = Token::DEC;
                ++src;
                break;
            }
            result.push_back(TokenDesc(value, lines, NULL));
        }
        break;
        case '*':
        {
            short value = Token::MUL;
            switch (*(src + 1))
            {
            case '=':
                value = Token::ASSIGN_MUL;
                ++src;
                break;
            case '-':
                value = Token::POW;
                ++src;
                break;
            }
            result.push_back(TokenDesc(value, lines, NULL));
        }
        break;
        case '/':
        {
            if (*(src + 1) == '*')
            {
                src += 2;
                while (*(src + 1) && !(*src == '*' && *(src + 1) == '/'))
                    ++src;
                src += 2;
            }
            else if (*(src + 1) == '/')
            {
                src += 2;
                while (*src && *src != '\n')
                    ++src;
            }
            else
            {
                short value = Token::DIV;
                if (*(src + 1) == '=')
                {
                    value = Token::ASSIGN_DIV;
                    ++src;
                }
                result.push_back(TokenDesc(value, lines, NULL));
            }
        }
        break;
        case '|':
        {
            short value = Token::BOR;
            switch (*(src + 1))
            {
            case '=':
                value = Token::ASSIGN_BOR;
                ++src;
                break;
            case '|':
                value = Token::OR;
                ++src;
                break;
            }
            result.push_back(TokenDesc(value, lines, NULL));
        }
        break;
        case '&':
        {
            short value = Token::BAND;
            switch (*(src + 1))
            {
            case '=':
                value = Token::ASSIGN_BAND;
                ++src;
                break;
            case '&':
                value = Token::AND;
                ++src;
                break;
            }
            result.push_back(TokenDesc(value, lines, NULL));
        }
        break;
        case '^':
        {
            short value = Token::BXOR;
            if (*(src + 1) == '=')
            {
                value = Token::ASSIGN_BXOR;
                ++src;
            }
            result.push_back(TokenDesc(value, lines, NULL));
        }
        break;
        case '%':
        {
            short value = Token::MOD;
            if (*(src + 1) == '=')
            {
                value = Token::ASSIGN_MOD;
                ++src;
            }
            result.push_back(TokenDesc(value, lines, NULL));
        }
        break;
        default:
        {
            const char *temp = src;
            while (*src >= 'a' && *src <= 'z' ||
                   *src >= 'A' && *src <= 'Z' ||
                   *src >= '0' && *src <= '9' ||
                   *src == '_')
                ++src;
                
            if (src != temp)
            {
                short value = Token::IDENTIFIER;
                char *literal = new char[src - temp + 1];
                memcpy(literal, temp, src - temp);
                literal[src - temp] = 0;
                for (int seq = Token::META; seq <= Token::NAMESPACE; ++seq)
                    if (!strcmp(literal, tokenlist[seq].str))
                        value = tokenlist[seq].type;
                result.push_back(TokenDesc(value, lines, literal));
                continue;
            }
        }
        break;
        }
        ++src;
    }

    result.push_back(TokenDesc(Token::EOT, lines, NULL));
    return true;
}

void Scanner::Scan(TokenDesc *desc)
{
    // while (src[pos])
    // {
    //     desc->value = GetSingleToken(desc);
    //     desc = desc->next;
    //     desc = new TokenDesc;
    // }
}

short Scanner::Advance(bool increase_lines)
{
    if (increase_lines)
        ++lines;
    ++pos;
    return 0;
}
bool Scanner::Advance(const char *dest)
{
    size_t temp = pos;
    return true;
}
short Scanner::Advance(char32_t cond, short true_sign, short false_sign)
{
    return src[++pos] == cond ? true_sign : false_sign;
}

short Scanner::GetSingleToken(TokenDesc *desc)
{
    switch (src[pos])
    {
    case '\r':
    case '\n':
        return Advance(true);
    case ' ':
    case '\f':
    case '\t':
    case '\v':
        return Advance();
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
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
TokenDesc *Scanner::ScanString(TokenDesc *desc)
{
    // std::string temp;
    // while (src[++pos])
    // {
    //     switch (src[pos])
    //     {
    //     case '\n':
    //         ++lines;
    //         break;
    //     case '\\':
    //         switch (src[++pos])
    //         {
    //         case '\\':
    //             temp += '\\';
    //             break;
    //         case 't':
    //             temp += '\t';
    //             break;
    //         case 'r':
    //             temp += '\r';
    //             break;
    //         case 'n':
    //             temp += '\n';
    //             break;
    //         case '\'':
    //             temp += '\'';
    //             break;
    //         case '\"':
    //             temp += '\"';
    //             break;
    //         case '\f':
    //             temp += '\f';
    //             break;
    //         case '\t':
    //             temp += '\t';
    //             break;
    //         case '\v':
    //             temp += '\v';
    //             break;
    //         }
    //         break;
    //     case '\"':
    //         desc->next = new TokenDesc(Token::LITERAL, lines, (void *)temp.c_str());
    //         return desc->next;
    //     default:
    //         temp += src[pos];
    //         break;
    //     }
    // }
    return NULL;
}

TokenDesc *Scanner::ScanNumber(TokenDesc *desc)
{
    return NULL;
}

TokenDesc *Scanner::ScanChar(TokenDesc *desc)
{
    return NULL;
}