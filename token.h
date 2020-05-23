#ifndef __TOKEN_H__
#define __TOKEN_H__
#include <string>
#include <vector>
#include <cstdlib>

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

namespace script
{
static struct Token
{
#define T(SIGN, STR, PREC) SIGN,
    enum Type : short {TOKEN(T)EOT};
#undef T
    short type;
	short precedence;
    std::string str;
	//const char str[12];
#define T(SIGN, STR, PREC) {Token::SIGN, PREC, STR},
} tokens[] = {TOKEN(T){Token::EOT}};
#undef T

struct TokenDesc
{
	short value;
	std::string literal;
	TokenDesc* next;
};
} // namespace script
#endif