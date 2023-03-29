#ifndef __TOKEN_H__
#define __TOKEN_H__

namespace myscript
{
#define TOKEN(_ELEM_)                       \
	/* category : symbols */                \
	_ELEM_(LBRACE, "{", 0)                  \
	_ELEM_(LBRACKET, "[", 0)                \
	_ELEM_(LPAREN, "(", 0)                  \
	_ELEM_(RBRACE, "}", 0)                  \
	_ELEM_(RBRACKET, "]", 0)                \
	_ELEM_(RPAREN, ")", 0)                  \
	_ELEM_(PERIOD, ".", 0)                  \
	_ELEM_(COMMA, ",", 1)                   \
	_ELEM_(COLON, ":", 0)                   \
	_ELEM_(SEMICOLON, ";", 0)               \
	/* unary operator */                    \
	_ELEM_(NOT, "!", 0)                     \
	_ELEM_(BNOT, "~", 0)                    \
	_ELEM_(INC, "++", 0)                    \
	_ELEM_(DEC, "--", 0)                    \
	/* binary operator */                   \
	_ELEM_(ARROW, "=>", 0)                  \
	_ELEM_(BOR, "|", 6)                     \
	_ELEM_(BAND, "&", 8)                    \
	_ELEM_(BXOR, "^", 7)                    \
	_ELEM_(ADD, "+", 12)                    \
	_ELEM_(SUB, "-", 12)                    \
	_ELEM_(MUL, "*", 11)                    \
	_ELEM_(DIV, "/", 11)                    \
	_ELEM_(MOD, "%", 11)                    \
	_ELEM_(POW, "**", 11)                   \
	_ELEM_(ASSIGN, "=", 2)                  \
	_ELEM_(ASSIGN_BOR, "|=", 3)             \
	_ELEM_(ASSIGN_BAND, "&=", 3)            \
	_ELEM_(ASSIGN_BXOR, "^=", 3)            \
	_ELEM_(ASSIGN_ADD, "+=", 3)             \
	_ELEM_(ASSIGN_SUB, "-=", 3)             \
	_ELEM_(ASSIGN_MUL, "*=", 3)             \
	_ELEM_(ASSIGN_DIV, "/=", 3)             \
	_ELEM_(ASSIGN_MOD, "%=", 3)             \
	_ELEM_(OR, "||", 4)                     \
	_ELEM_(AND, "&&", 5)                    \
	_ELEM_(EQ, "==", 5)                     \
	_ELEM_(NEQ, "!=", 5)                    \
	_ELEM_(LT, "<", 5)                      \
	_ELEM_(LE, "<=", 5)                     \
	_ELEM_(GT, ">", 5)                      \
	_ELEM_(GE, ">=", 5)                     \
	/* tarnery operator */                  \
	_ELEM_(CONDITIONAL, "?", 3)             \
	/* keywords */                          \
	_ELEM_(META, "meta", 0)                 \
	_ELEM_(NEW, "new", 0)                   \
	_ELEM_(BYTE, "byte", 0)                 \
	_ELEM_(IF, "if", 0)                     \
	_ELEM_(ELSE, "else", 0)                 \
	_ELEM_(MATCH, "match", 0)               \
	_ELEM_(LOOP, "loop", 0)                 \
	_ELEM_(DEFAULT, "default", 0)           \
	_ELEM_(CONTINUE, "continue", 0)         \
	_ELEM_(BREAK, "break", 0)               \
	_ELEM_(RETURN, "return", 0)             \
	_ELEM_(SELF, "self", 0)                 \
	_ELEM_(VAR, "var", 0)					\
	_ELEM_(CONST, "const", 0)               \
	_ELEM_(STATIC, "static", 0)             \
	_ELEM_(ENUM, "enum", 0)                 \
	_ELEM_(STRUCT, "struct", 0)             \
	_ELEM_(OPERATOR, "operator", 0)         \
	_ELEM_(GET, "get", 0)                   \
	_ELEM_(SET, "set", 0)                   \
	_ELEM_(AS, "as", 0)						\
	_ELEM_(IS, "is", 0)						\
	_ELEM_(FUNCTION, "function", 0)			\
	_ELEM_(NAMESPACE, "namespace", 0)       \
	/* literals */                          \
	_ELEM_(LITERAL, "", 0)                  \
	_ELEM_(LITERAL_INTEGER, "{integer}", 0) \
	_ELEM_(LITERAL_FLOAT, "{float}", 0)     \
	_ELEM_(LITERAL_CHAR, "{char}", 0)       \
	_ELEM_(LITERAL_STRING, "{string}", 0)   \
	_ELEM_(LITERAL_TRUE, "true", 0)         \
	_ELEM_(LITERAL_FALSE, "false", 0)       \
	_ELEM_(LITERAL_NULL, "null", 0)         \
	_ELEM_(IDENTIFIER, "{identifer}", 0)

	static struct Token
	{
#define T(SIGN, STR, PREC) SIGN,
		const enum Type : short { TOKEN(T) EOT } type;
#undef T
		const int precedence;
		const char str[12];
#define T(SIGN, STR, PREC) {Token::SIGN, PREC, STR},
	} tokenlist[] = {TOKEN(T){Token::EOT}};
#undef T

	union Literal
	{
		void *p;
		char c;
		char *s;
		long long l;
		double d;
	};
	
	struct TokenDesc
	{
		short value;
		unsigned int lines;
		Literal literal;

		TokenDesc(short value, unsigned int lines, Literal literal);
		TokenDesc(short value, unsigned int lines, void *literal);
		TokenDesc(short value, unsigned int lines, char literal);
		TokenDesc(short value, unsigned int lines, char *literal);
		TokenDesc(short value, unsigned int lines, long long literal);
		TokenDesc(short value, unsigned int lines, double literal);
		TokenDesc(TokenDesc &&other);
		TokenDesc &operator=(TokenDesc &&other);
		const char *GetInfo();
	};
}

#endif