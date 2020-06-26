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


TokenDesc* tokendesc(short value, void* literal)
{
    TokenDesc* temp = (TokenDesc*)malloc(sizeof(TokenDesc));
    if(!temp)
        return NULL;
    temp->value = value;
    temp->literal.s = (const char*)literal;
    return temp;
}
 
TokenDesc* Scan(const char* src)
{
    #define CAPACITY 1024
    char temp[CAPACITY];
    size_t lines = 0;
    TokenDesc head;
    TokenDesc* desc = &head;
    while(*src)
    {
        switch (*src)
        {
        case '\n':
            ++lines;  
            break;
        case '\r':case ' ': case '\f': case '\t': case '\v':
            break;
        case '0':
        if(*(src + 1) == 'x')
        {
            int temp = 0;
            src += 2;
            while(*src)
            {
                if (*src >= '0' && *src <= '9')
                {
                    temp *= 16;
                    temp += *src - '0';
                }
                else if (src >= 'A' && src <= 'F')
                {
                    temp *= 16;
                    temp += *src - '0';
                }
                else
                    break;
                ++src;
            }
            desc->next = tokendesc(LITERAL_INTEGER, temp);
            desc = desc->next;
            break;
        }
        case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
        {
            int temp = 0;
            while (*src >= '0' && *src <= '9')
            {
                temp *= 16;
                temp += *src - '0';
                ++src;
            }
            desc->next = tokendesc(LITERAL_INTEGER, temp);
            desc = desc->next;
        }
        break;
        case '\"':
            break;
        case '\'':
            break;
        case ':':
            desc->next = tokendesc(COLON, NULL);
            desc = desc->next;
            break;
        case ';':
            desc->next = tokendesc(SEMICOLON, NULL);
            desc = desc->next;
            break;
        case '.':
            desc->next = tokendesc(PERIOD, NULL);
            desc = desc->next;
            break;
        case ',':
            desc->next = tokendesc(COMMA, NULL);
            desc = desc->next;
            break;
        case '?':
            desc->next = tokendesc(CONDITIONAL, NULL);
            desc = desc->next;
            break;
        case '(':
            desc->next = tokendesc(LPAREN, NULL);
            desc = desc->next;
            break;
        case ')':
            desc->next = tokendesc(RPAREN, NULL);
            desc = desc->next;
            break;
        case '{':
            desc->next = tokendesc(LBRACE, NULL);
            desc = desc->next;
            break;
        case '}':
            desc->next = tokendesc(RBRACE, NULL);
            desc = desc->next;
            break;
        case '[':
            desc->next = tokendesc(LBRACKET, NULL);
            desc = desc->next;
            break;
        case ']':
            desc->next = tokendesc(RBRACKET, NULL);
            desc = desc->next;
            break;
        case '<':
            if(*(src + 1) == '=')       desc->next = tokendesc(LE, NULL), ++src;
            else                        desc->next = tokendesc(RBRACKET, NULL);
            desc = desc->next;
            break;
        case '>':
            if(*(src + 1) == '=')       desc->next = tokendesc(GE, NULL), ++src;
            else                        desc->next = tokendesc(GT, NULL);
            desc = desc->next;
            break;
        case '!':
            if(*(src + 1) == '=')       desc->next = tokendesc(NEQ, NULL), ++src;
            else                        desc->next = tokendesc(NOT, NULL);
            desc = desc->next;
            break;
        case '=':
            if(*(src + 1) == '=')       desc->next = tokendesc(EQ, NULL), ++src;
            else                        desc->next = tokendesc(ASSIGN, NULL);
            desc = desc->next;
            break;
        case '+':
            if(*(src + 1) == '=')       desc->next = tokendesc(ASSIGN_ADD, NULL), ++src;
            else if(*(src + 1) == '+')  desc->next = tokendesc(INC, NULL), ++src;
            else                        desc->next = tokendesc(ADD, NULL);
            desc = desc->next;
            break;
        case '-':
            if(*(src + 1) == '=')       desc->next = tokendesc(ASSIGN_SUB, NULL), ++src;
            else if(*(src + 1) == '-')  desc->next = tokendesc(DEC, NULL), ++src;
            else                        desc->next = tokendesc(SUB, NULL);
            desc = desc->next;
            break;
        case '*':
            if(*(src + 1) == '=')       desc->next = tokendesc(ASSIGN_MUL, NULL), ++src;
            else if(*(src + 1) == '*')  desc->next = tokendesc(POW, NULL), ++src;
            else                        desc->next = tokendesc(MUL, NULL);
            desc = desc->next;
            break;
        case '/':
            if(*(src + 1) == '=')       desc->next = tokendesc(ASSIGN_DIV, NULL), ++src;
            else if(*(src + 1) == '*')  {src += 2; while(!(*src == '*' && *(src + 1) == "/") && *(src + 1) != NULL) ++src;}
            else if(*(src + 1) == '/')  {src += 2; while(*src != '\n' && *src != NULL) ++src;}
            else                        desc->next = tokendesc(DIV, NULL);
            desc = desc->next;
            break;
        case '|':
            if(*(src + 1) == '=')       desc->next = tokendesc(ASSIGN_BOR, NULL), ++src;
            else if(*(src + 1) == '*')  desc->next = tokendesc(OR, NULL), ++src;
            else                        desc->next = tokendesc(BOR, NULL);
            desc = desc->next;
            break;
        case '&':
            if(*(src + 1) == '=')       desc->next = tokendesc(ASSIGN_BAND, NULL), ++src;
            else if(*(src + 1) == '*')  desc->next = tokendesc(AND, NULL), ++src;
            else                        desc->next = tokendesc(BAND, NULL);
            desc = desc->next;
            break;
        case '^':
            if(*(src + 1) == '=')       desc->next = tokendesc(ASSIGN_BXOR, NULL), ++src;
            else                        desc->next = tokendesc(BXOR, NULL);
            desc = desc->next;
            break;
        case '%':
            if(*(src + 1) == '=')       desc->next = tokendesc(ASSIGN_MOD, NULL), ++src;
            else                        desc->next = tokendesc(MOD, NULL);
            desc = desc->next;
            break;
        default:

            break;
        }
        ++src;
    }
    return head.next;
}