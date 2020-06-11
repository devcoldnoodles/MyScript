#include "token.h"

#define TOKEN(_ELEM_)                	\
	/* category : symbols */          	\
	_ELEM_(LBRACE, "{", 0)            	\
	_ELEM_(LBRACKET, "[", 0)          	\
	_ELEM_(LPAREN, "(", 0)            	\
	_ELEM_(RBRACE, "}", 0)            	\
	_ELEM_(RBRACKET, "]", 0)          	\
	_ELEM_(RPAREN, ")", 0)            	\
	_ELEM_(PERIOD, ".", 0)            	\
	_ELEM_(COMMA, ",", 1)             	\
	_ELEM_(COLON, ":", 0)             	\
	_ELEM_(SEMICOLON, ";", 0)         	\
	/* unary operator */              	\
	_ELEM_(NOT, "!", 0)               	\
	_ELEM_(BNOT, "~", 0)              	\
	_ELEM_(INC, "++", 0)              	\
	_ELEM_(DEC, "--", 0)              	\
	/* binary operator */             	\
	_ELEM_(ARROW, "=>", 0)            	\
	_ELEM_(ASSIGN, "=", 2)            	\
	_ELEM_(ADD, "+", 12)              	\
	_ELEM_(SUB, "-", 12)              	\
	_ELEM_(MUL, "*", 11)              	\
	_ELEM_(DIV, "/", 11)              	\
	_ELEM_(MOD, "%", 11)              	\
	_ELEM_(BOR, "|", 6)               	\
	_ELEM_(BAND, "&", 8)              	\
	_ELEM_(BXOR, "^", 7)              	\
	_ELEM_(OR, "||", 4)               	\
	_ELEM_(AND, "&&", 5)              	\
	/* tarnery operator */            	\
	_ELEM_(CONDITIONAL, "?", 3)       	\
	/* keywords */          			\
	_ELEM_(BYTE, "byte", 0)           	\
	_ELEM_(IF, "if", 0)               	\
	_ELEM_(ELSE, "else", 0)           	\
	_ELEM_(MATCH, "match", 0)         	\
	_ELEM_(LOOP, "loop", 0)           	\
	_ELEM_(DEFAULT, "default", 0)     	\
	_ELEM_(RETURN, "return", 0)       	\
	_ELEM_(SELF, "self", 0)           	\
	_ELEM_(CONST, "const", 0)         	\
	_ELEM_(STATIC, "static", 0)       	\
	_ELEM_(ENUM, "enum", 0)           	\
	_ELEM_(STRUCT, "struct", 0)       	\
	_ELEM_(OPERATOR, "operator", 0)   	\
	_ELEM_(GET, "get", 0)             	\
	_ELEM_(SET, "set", 0)             	\
	_ELEM_(NAMESPACE, "namespace", 0) 	\
	/* literals */						\
	_ELEM_(LITERAL, "", 0)				\

static struct Token
{
#define T(SIGN, STR, PREC) SIGN,
    const enum Type {TOKEN(T)EOT} type;
#undef T
	const int precedence;
	const char str[12];
#define T(SIGN, STR, PREC) {SIGN, PREC, STR},
} tokens[] = {TOKEN(T){EOT}};
#undef T

TokenDesc* Scan(const char* src)
{
    #define CAPACITY 1024
    char temp[CAPACITY];
    size_t size = 0, pos = 0, lines = 0;
    TokenDesc* head = (TokenDesc*)malloc(sizeof(TokenDesc));
    TokenDesc* pointer = head;
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
			break;
        case '\"':
            break;
        case '\'':
            break;
        default:
            break;
        }
    }
    return head;
}

int ScanNumber(const char* src, int* index)
{
    while(1)
    {
        
    }
}