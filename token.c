#include "token.h"

static struct Token
{
#define T(SIGN, STR, PREC) SIGN,
    const enum {TOKEN(T)EOT} type;
#undef T
	const int precedence;
	const char str[12];
#define T(SIGN, STR, PREC) {SIGN, PREC, STR},
} tokens[] = {TOKEN(T){EOT}};
#undef T

const char* GetString(TokenDesc* desc)
{
	return tokens[desc->value].str;
}

int GetPrecedence(TokenDesc* desc)
{
	return tokens[desc->value].precedence;
}

TokenDesc* InitTokenDesc(short value, TokenDesc* next, const char* literal)
{
    TokenDesc* temp = (TokenDesc*)malloc(sizeof(TokenDesc));
    if(!temp)
        return NULL;
    temp->value = value;
    temp->next = next;
    temp->literal = literal;
    return temp;
}
 
TokenDesc* Scan(const char* src)
{
    #define CAPACITY 1024
    char temp[CAPACITY];
    size_t size = 0, pos = 0, lines = 0;
    TokenDesc* pointer;
    while(*src)
    {
        switch (src[pos])
        {
        case '\n':
            ++pos, ++lines;
            break;
        case '\r':case ' ': case '\f': case '\t': case '\v':
            ++pos;
            break;
        case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
            if(src[pos] == '0' && src[pos + 1] == 'x')
            {
                
            }
            else
            {
                while(src[pos] >= '0' && src[pos] <= '9')
                {
                    
                }
            }
			break;
        case '\"':
            break;
        case '\'':
            break;
        case ':':
            pointer = InitTokenDesc(COLON, NULL, NULL);
            tokens.push_back({Token::COLON, ":", lines});
            marker = index + 1;
            break;
        case ';':
            tokens.push_back({Token::SEMICOLON, ";", lines});
            marker = index + 1;
            break;
        case '.':
            tokens.push_back({Token::DOT, ".", lines});
            marker = index + 1;
            break;
        case ',':
            tokens.push_back({Token::COMMA, ",", lines});
            marker = index + 1;
            break;
        case '?':
            tokens.push_back({Token::QUESTION, "?", lines});
            marker = index + 1;
            break;
        case '(':
            tokens.push_back({Token::LPARAM, "(", lines});
            marker = index + 1;
            break;
        case ')':
            tokens.push_back({Token::RPARAM, ")", lines});
            marker = index + 1;
            break;
        case '{':
            tokens.push_back({Token::LBRACKET, "{", lines});
            marker = index + 1;
            break;
        case '}':
            tokens.push_back({Token::RBRACKET, "}", lines});
            marker = index + 1;
            break;
        case '[':
            tokens.push_back({Token::LSUBRACKET, "[", lines});
            marker = index + 1;
            break;
        case ']':
            tokens.push_back({Token::RSUBRACKET, "]", lines});
            marker = index + 1;
            break;
        case '<':
            if (str[index + 1] == '=')
                tokens.push_back({Token::LE, "<=", lines}), ++index;
            else
                tokens.push_back({Token::LT, "<", lines});
            marker = index + 1;
            break;
        case '>':
            if (str[index + 1] == '=')
                tokens.push_back({Token::GE, ">=", lines}), ++index;
            else
                tokens.push_back({Token::GT, ">", lines});
            marker = index + 1;
            break;
        case '!':
            if (str[index + 1] == '=')
                tokens.push_back({Token::NEQ, "!=", lines}), ++index;
            else
                tokens.push_back({Token::NOT, "!", lines});
            marker = index + 1;
            break;
        case '=':
            if (str[index + 1] == '=')
                tokens.push_back({Token::EQ, "==", lines}), ++index;
            else
                tokens.push_back({Token::ASSIGN, "=", lines});
            marker = index + 1;
            break;
        case '+':
            if (str[index + 1] == '=')
                tokens.push_back({Token::ASSIGN_ADD, "+=", lines}), ++index;
            else if (str[index + 1] == '+')
                tokens.push_back({Token::INC, "++", lines}), ++index;
            else
                tokens.push_back({Token::ADD, "+", lines});
            marker = index + 1;
            break;
        case '-':
            if (str[index + 1] == '=')
                tokens.push_back({Token::ASSIGN_SUB, "-=", lines}), ++index;
            else if (str[index + 1] == '-')
                tokens.push_back({Token::DEC, "--", lines}), ++index;
            else
                tokens.push_back({Token::SUB, "-", lines});
            marker = index + 1;
            break;
        case '*':
            if (str[index + 1] == '=')
                tokens.push_back({Token::ASSIGN_MUL, "*=", lines}), ++index;
            else if (str[index + 1] == '*')
                tokens.push_back({Token::POW, "**", lines}), ++index;
            else
                tokens.push_back({Token::MUL, "*", lines});
            marker = index + 1;
            break;
        case '/':
            if (str[index + 1] == '=')
                tokens.push_back({Token::ASSIGN_DIV, "/=", lines}), ++index;
            else if (str[index + 1] == '*')
                predicted = Token::COMMENTBLOCK;
            else if (str[index + 1] == '/')
                predicted = Token::COMMENTLINE;
            else
                tokens.push_back({Token::DIV, "/", lines});
            marker = index + 1;
            break;
        case '|':
            if (str[index + 1] == '=')
                tokens.push_back({Token::ASSIGN_BOR, "|=", lines}), ++index;
            else if (str[index + 1] == '|')
                tokens.push_back({Token::OR, "||", lines}), ++index;
            else
                tokens.push_back({Token::BOR, "|", lines});
            marker = index + 1;
            break;
        case '&':
            if (str[index + 1] == '=')
                tokens.push_back({Token::ASSIGN_BAND, "&=", lines}), ++index;
            else if (str[index + 1] == '&')
                tokens.push_back({Token::AND, "&&", lines}), ++index;
            else
                tokens.push_back({Token::BAND, "&", lines});
            marker = index + 1;
            break;
        case '^':
            if (str[index + 1] == '=')
                tokens.push_back({Token::ASSIGNXOR, "^=", lines}), ++index;
            else
                tokens.push_back({Token::XOR, "^", lines});
            marker = index + 1;
            break;
        case '%':
            if (str[index + 1] == '=')
                tokens.push_back({Token::ASSIGN_MOD, "%=", lines}), ++index;
            else
                tokens.push_back({Token::MOD, "%", lines});
            marker = index + 1;
            break;
        default:
            break;
        }
    }
    return head;
}