#include "parser.h"

using namespace myscript;
#define ParseGeneralSentences(tokens, index, errors) ParseSentence(tokens, index, errors, 4, ParseIf, ParseLoop, ParseExpr, ParseDeclare)

namespace myscript 
{
	SyntaxExpr* ParseExpr(std::vector<TokenDesc*> &tokens, size_t &index, std::vector<Error> &errors)
	{
		return ParseComma(tokens, index, errors);
	}
	SyntaxExpr* ParseComma(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParseAssign(tokens, temp, errors)) == nullptr)
			return nullptr;
		while (tokens[temp]->value == Token::COMMA)
		{
			if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected expression", tokens[--temp]->lines});
				delete lexp;
				return nullptr;
			}
			lexp = new SyntaxComma(lexp, rexp);
		}
		index = temp;
		return lexp;
	}
	SyntaxExpr* ParseAssign(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParseTernaryOperator(tokens, temp, errors)) == nullptr)
			return nullptr;
		switch (tokens[temp]->value)
		{
		case Token::ASSIGN:
			if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxAssign(lexp, rexp);
			break;
		case Token::ASSIGN_ADD:
			if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxAssign(lexp, new SyntaxAdd(lexp, rexp));
			break;
		case Token::ASSIGN_SUB:
			if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxAssign(lexp, new SyntaxSub(lexp, rexp));
			break;
		case Token::ASSIGN_MUL:
			if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxAssign(lexp, new SyntaxMul(lexp, rexp));
			break;
		case Token::ASSIGN_DIV:
			if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxAssign(lexp, new SyntaxDiv(lexp, rexp));
			break;
		case Token::ASSIGN_MOD:
			if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxAssign(lexp, new SyntaxMod(lexp, rexp));
			break;
		}
		index = temp;
		return lexp;
	ErrorHandle:
		delete lexp;
		return nullptr;
	}
	SyntaxExpr* ParseTernaryOperator(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr *lexpr, *expr, *rexpr;
		if ((lexpr = ParseOr(tokens, temp, errors)) == nullptr)
			return nullptr;
		if (tokens[temp]->value == Token::CONDITIONAL)
		{
			if ((expr = ParseTernaryOperator(tokens, ++temp, errors)) == nullptr)
				goto ErrorHandle;
			if (tokens[temp]->value != Token::COLON)
			{
				errors.push_back({"Expected colon(:)", tokens[temp]->lines});
				return nullptr;
			}
			if ((rexpr = ParseTernaryOperator(tokens, ++temp, errors)) == nullptr)
			{
				delete expr;
				goto ErrorHandle;
			}
			lexpr = new SyntaxTernaryOperator(lexpr, expr, rexpr);
		}
		index = temp;
		return lexpr;
	ErrorHandle:
		delete lexpr;
		return nullptr;
	}
	SyntaxExpr* ParseOr(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParseAnd(tokens, temp, errors)) == nullptr)
			return nullptr;
		while (tokens[temp]->value == Token::OR)
		{
			if ((rexp = ParseAnd(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				delete lexp;
				return nullptr;
			}
			lexp = new SyntaxOr(lexp, rexp);
		}
		index = temp;
		return lexp;
	}
	SyntaxExpr* ParseAnd(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParseCmp(tokens, temp, errors)) == nullptr)
			return nullptr;
		while (tokens[temp]->value == Token::AND)
		{
			if ((rexp = ParseCmp(tokens, ++temp, errors)) != nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxAnd(lexp, rexp);
		}
		index = temp;
		return lexp;
	ErrorHandle:
		delete lexp;
		return nullptr;
	}
	SyntaxExpr* ParseCmp(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParseAdd(tokens, temp, errors)) == nullptr)
			return nullptr;
		switch (tokens[temp]->value)
		{
		case Token::EQ:
			if ((rexp = ParseAdd(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxEqual(lexp, rexp);
			break;
		case Token::NEQ:
			if ((rexp = ParseAdd(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxNotEqual(lexp, rexp);
			break;
		case Token::GT:
			if ((rexp = ParseAdd(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxGreatThan(lexp, rexp);
			break;
		case Token::GE:
			if ((rexp = ParseAdd(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxGreatEqual(lexp, rexp);
			break;
		case Token::LT:
			if ((rexp = ParseAdd(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxLessThan(lexp, rexp);
			break;
		case Token::LE:
			if ((rexp = ParseAdd(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxLessEqual(lexp, rexp);
			break;
		}
		index = temp;
		return lexp;
	ErrorHandle:
		delete lexp;
		return nullptr;
	}
	SyntaxExpr* ParseAdd(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParseMul(tokens, temp, errors)) == nullptr)
			return nullptr;
	_loop_:
		switch (tokens[temp]->value)
		{
		case Token::ADD:
			if ((rexp = ParseMul(tokens, ++temp, errors)) != nullptr)
			{
				lexp = new SyntaxAdd(lexp, rexp);
				goto _loop_;
			}
			errors.push_back({"Expected expression", tokens[--temp]->lines});
			break;
		case Token::SUB:
			if ((rexp = ParseMul(tokens, ++temp, errors)) != nullptr)
			{
				lexp = new SyntaxSub(lexp, rexp);
				goto _loop_;
			}
			errors.push_back({"Expected expression", tokens[--temp]->lines});
			break;
		}
		index = temp;
		return lexp;
	}
	SyntaxExpr* ParseMul(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParsePow(tokens, temp, errors)) == nullptr)
			return nullptr;
	_loop_:
		switch (tokens[temp]->value)
		{
		case Token::MUL:
			if ((rexp = ParseMul(tokens, ++temp, errors)) != nullptr)
			{
				lexp = new SyntaxMul(lexp, rexp);
				goto _loop_;
			}
			errors.push_back({"Expected expression", tokens[--temp]->lines});
			break;
		case Token::DIV:
			if ((rexp = ParseMul(tokens, ++temp, errors)) != nullptr)
			{
				lexp = new SyntaxDiv(lexp, rexp);
				goto _loop_;
			}
			errors.push_back({"Expected expression", tokens[--temp]->lines});
			break;
		case Token::MOD:
			if ((rexp = ParseMul(tokens, ++temp, errors)) != nullptr)
			{
				lexp = new SyntaxMod(lexp, rexp);
				goto _loop_;
			}
			errors.push_back({"Expected expression", tokens[--temp]->lines});
			break;
		}
		index = temp;
		return lexp;
	}
	SyntaxExpr* ParsePow(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParsePrefix(tokens, temp, errors)) == nullptr)
			return nullptr;
		while (tokens[temp]->value == Token::POW)
		{
			if ((rexp = ParseMul(tokens, ++temp, errors)) != nullptr)
			{
				errors.push_back({"Expected expression", tokens[--temp]->lines});
				break;
			}
			lexp = new SyntaxPow(lexp, rexp);
		}
		index = temp;
		return lexp;
	}
	SyntaxExpr* ParsePrefix(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* expr = nullptr;
		switch (tokens[temp]->value)
		{
		case Token::INC:
			if ((expr = ParsePostfix(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				return nullptr;
			}
			expr = new SyntaxPrefixPlusx2(expr);
			break;
		case Token::DEC:
			if ((expr = ParsePostfix(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				return nullptr;
			}
			expr = new SyntaxPostfixMinusx2(expr);
			break;
		case Token::ADD:
			if ((expr = ParsePostfix(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				return nullptr;
			}
			expr = new SyntaxPrefixPlus(expr);
			break;
		case Token::SUB:
			if ((expr = ParsePostfix(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				return nullptr;
			}
			expr = new SyntaxPrefixMinus(expr);
			break;
		case Token::NOT:
			if ((expr = ParsePostfix(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				return nullptr;
			}
			expr = new SyntaxNot(expr);
			break;
		case Token::NEW:
			if ((expr = ParsePostfix(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp]->lines});
				return nullptr;
			}
			expr = new SyntaxNew(expr);
			break;
		default:
			if ((expr = ParsePostfix(tokens, temp, errors)) == nullptr)
				return nullptr;
			break;
		}
		index = temp;
		return expr;
	}
	SyntaxExpr* ParsePostfix(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParseElement(tokens, temp, errors)) == nullptr)
			return nullptr;
	_loop_:
		switch (tokens[temp]->value)
		{
		case Token::INC:
			lexp = new SyntaxPostfixPlusx2(lexp), ++temp;
			break;
		case Token::DEC:
			lexp = new SyntaxPostfixMinusx2(lexp), ++temp;
			break;
		case Token::AS:
			if ((rexp = ParseElement(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[temp + 1]->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxAs(lexp, rexp);
			break;
		case Token::IS:
			if ((rexp = ParseElement(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[temp + 1]->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxIs(lexp, rexp);
			break;
		case Token::PERIOD:
			if (tokens[++temp]->value != Token::IDENTIFIER)
			{
				errors.push_back({"Expected identifier", tokens[temp]->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxDot(lexp, new SyntaxLiteral(tokens[temp++]));
			goto _loop_;
		case Token::LBRACKET:
			if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[temp + 1]->lines});
				goto ErrorHandle;
			}
			if (tokens[temp++]->value != Token::RBRACKET)
			{
				errors.push_back({"Expected ]", tokens[temp + 1]->lines});
				delete rexp;
				goto ErrorHandle;
			}
			lexp = new SyntaxRef(lexp, rexp);
			goto _loop_;
		case Token::LPAREN:
			SyntaxCall* expr = new SyntaxCall(lexp);
			lexp = expr;
			if ((rexp = ParseAssign(tokens, ++temp, errors)) != nullptr)
			{
				expr->params.push_back(rexp);
				while (tokens[temp]->value == Token::COMMA)
				{
					if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
					{
						errors.push_back({"Expected expression", tokens[--temp]->lines});
						goto ErrorHandle;
					}
					expr->params.push_back(rexp);
				}
			}
			if (tokens[temp++]->value != Token::RPAREN)
			{
				errors.push_back({"Expected )", tokens[--temp]->lines});
				delete rexp;
				goto ErrorHandle;
			}
			goto _loop_;
		}
		index = temp;
		return lexp;
	ErrorHandle:
		delete lexp;
		return nullptr;
	}
	SyntaxExpr* ParseElement(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* expr = nullptr;
		switch (tokens[temp]->value)
		{
		case Token::LPAREN:
			if ((expr = ParseExpr(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected expression", tokens[temp]->lines});
				goto ErrorHandle;
			}
			if (tokens[temp++]->value != Token::RPAREN)
			{
				errors.push_back({"Expected )", tokens[temp]->lines});
				goto ErrorHandle;
			}
			break;
		case Token::SELF:
			expr = new SyntaxSelf(tokens[temp++]->lines);
			break;
		case Token::IDENTIFIER:
			expr = new SyntaxIdentifier(tokens[temp++]);
			break;
		case Token::FUNCTION:
			if ((expr = ParseFunction(tokens, temp, errors)) == nullptr)
				goto ErrorHandle;
			break;
		case Token::CONTINUE:
			expr = new SyntaxContinue(tokens[temp++]->lines);
			break;
		case Token::BREAK:
			expr = new SyntaxBreak(tokens[temp++]->lines);
			break;
		case Token::RETURN:
			expr = new SyntaxReturn(ParseAssign(tokens, ++temp, errors));
			break;
		case Token::NEW:
			if ((tokens[++temp]->value != Token::IDENTIFIER))
			{
				errors.push_back({"Operator 'new' must place behind type", tokens[temp]->lines});
				goto ErrorHandle;
			}
			expr = new SyntaxNew(new SyntaxIdentifier(tokens[temp++]));
			break;
		default:
			if ((expr = ParseLiteral(tokens, temp, errors)) == nullptr)
				return nullptr;
			break;
		}
		index = temp;
		return expr;
	ErrorHandle:
		delete expr;
		return nullptr;
	}
	SyntaxExpr* ParseLiteral(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* expr = nullptr;
		if ((expr = ParseSimpleLiteral(tokens, temp, errors)) || (expr = ParseArray(tokens, temp, errors)) || (expr = ParseDictionary(tokens, temp, errors)))
			index = temp;
		return expr;
	}
	SyntaxExpr* ParseSimpleLiteral(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		switch (tokens[index]->value)
		{
		case Token::LITERAL_INTEGER:
		case Token::LITERAL_FLOAT:
		case Token::LITERAL_STRING:
		case Token::LITERAL_TRUE:
		case Token::LITERAL_FALSE:
		case Token::LITERAL_CHAR:
		case Token::LITERAL_NULL:
			return new SyntaxLiteral(tokens[index++]);
		}
		return nullptr;
	}
	SyntaxExpr* ParseArray(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxArray* expr = new SyntaxArray();
		SyntaxExpr* element;
		if (tokens[temp++]->value != Token::LBRACKET)
			goto ErrorHandle;
		if ((element = ParseAssign(tokens, temp, errors)) != nullptr)
		{
			expr->elements.push_back(element);
			while (tokens[temp]->value == Token::COMMA)
			{
				element = ParseAssign(tokens, ++temp, errors);
				if (element == nullptr)
				{
					errors.push_back({"Expected element", tokens[temp]->lines});
					goto ErrorHandle;
				}
				expr->elements.push_back(element);
			}
		}
		if (tokens[temp++]->value != Token::RBRACKET)
		{
			errors.push_back({"Expected ]", tokens[temp]->lines});
			goto ErrorHandle;
		}
		if (tokens[temp]->value == Token::LT)
		{
			if (tokens[++temp]->value == Token::LITERAL_INTEGER)
			{
				size_t init = tokens[temp++]->literal.i;
				for (size_t index = expr->elements.size(); index < init; ++index)
					expr->elements.push_back(new SyntaxLiteral(new TokenDesc{Token::LITERAL_NULL, tokens[temp]->lines, NULL}));
			}
			else
			{
				errors.push_back({"Expected number", tokens[temp]->lines});
				goto ErrorHandle;
			}
			if (tokens[temp++]->value != Token::GT)
				goto ErrorHandle;
		}
		index = temp;
		return expr;
	ErrorHandle:
		delete expr;
		return nullptr;
	}
	std::pair<SyntaxExpr*, SyntaxExpr*>* ParseKeyVal(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* key;
		SyntaxExpr* value;
		if (tokens[temp]->value == Token::IDENTIFIER)
			key = new SyntaxLiteral(new TokenDesc{Token::LITERAL_STRING, tokens[temp]->lines, tokens[temp]->literal.s}), ++temp;
		else if ((key = ParseLiteral(tokens, temp, errors)) == NULL)
			return NULL;
		if (tokens[temp++]->value != Token::COLON)
			goto ErrorHandle;
		if ((value = ParseAssign(tokens, temp, errors)) == NULL)
		{
			errors.push_back({"Expected value", tokens[temp]->lines});
			goto ErrorHandle;
		}
		index = temp;
		return new std::pair<SyntaxExpr*, SyntaxExpr*>(key, value);
	ErrorHandle:
		delete key;
		return NULL;
	}
	SyntaxExpr* ParseDictionary(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxDictionary* dict = new SyntaxDictionary();
		std::pair<SyntaxExpr*, SyntaxExpr*>* key;
		if (tokens[temp++]->value != Token::LBRACE)
			goto ErrorHandle;
		if ((key = ParseKeyVal(tokens, temp, errors)) != NULL)
		{
			dict->elements.push_back(*key);
			while (tokens[temp]->value == Token::COMMA)
			{
				if ((key = ParseKeyVal(tokens, ++temp, errors)) == NULL)
				{
					errors.push_back({"Expected key and value", tokens[temp]->lines});
					goto ErrorHandle;
				}
				dict->elements.push_back(*key);
			}
		}
		if (tokens[temp++]->value != Token::RBRACE)
		{
			errors.push_back({"Expected }", tokens[temp]->lines});
			goto ErrorHandle;
		}
		index = temp;
		return dict;
	ErrorHandle:
		delete dict;
		return NULL;
	}
	SyntaxExpr* ParseFunction(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxFunction* expr = new SyntaxFunction();
		if (tokens[temp++]->value != Token::FUNCTION)
			goto ErrorHandle;
		if (tokens[temp++]->value != Token::LPAREN)
			goto ErrorHandle;
		if (tokens[temp]->value == Token::IDENTIFIER)
		{
			expr->params.push_back(tokens[temp++]->literal.s);
			while (tokens[temp]->value == Token::COMMA)
			{
				if (tokens[++temp]->value != Token::IDENTIFIER)
				{
					errors.push_back({"Expected identifier", tokens[--temp]->lines});
					goto ErrorHandle;
				}
				expr->params.push_back(tokens[temp++]->literal.s);
			}
		}
		if (tokens[temp++]->value != Token::RPAREN)
		{
			errors.push_back({"Expected )", tokens[--temp]->lines});
			goto ErrorHandle;
		}
		if ((expr->sents = ParseBlock(tokens, temp, errors)) == nullptr)
			goto ErrorHandle;
		index = temp;
		return expr;
	ErrorHandle:
		delete expr;
		return nullptr;
	}
	SyntaxExpr* ParseSentence(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors, size_t args_count ...)
	{
		size_t temp = index;
		SyntaxExpr* sent = nullptr;
		va_list args_ptr;
		va_start(args_ptr, args_count);
		for (size_t count = 0; count < args_count; ++count)
			if ((sent = va_arg(args_ptr, SyntaxExpr* (*)(std::vector<TokenDesc*>&, size_t&, std::vector<Error>&))(tokens, temp, errors)) != nullptr)
				break;
		va_end(args_ptr);
		while (tokens[temp]->value == Token::SEMICOLON)	++temp;
		index = temp;
		return sent;
	}
	SyntaxExpr* ParseIf(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxIf* sent = new SyntaxIf();
		if (tokens[temp++]->value != Token::IF)
			goto ErrorHandle;
		if (tokens[temp++]->value != Token::LPAREN)
		{
			errors.push_back({"Expected (", tokens[--temp]->lines});
			goto ErrorHandle;
		}
		if ((sent->cond = ParseExpr(tokens, temp, errors)) == nullptr)
		{
			errors.push_back({"Expected expression", tokens[--temp]->lines});
			goto ErrorHandle;
		}
		if (tokens[temp++]->value != Token::RPAREN)
		{
			errors.push_back({"Expected )", tokens[--temp]->lines});
			goto ErrorHandle;
		}
		if ((sent->truesents = ParseBlock(tokens, temp, errors)) == nullptr)
			goto ErrorHandle;
		if (tokens[temp]->value == Token::ELSE && (sent->falsesents = ParseBlock(tokens, ++temp, errors)) == nullptr)
			goto ErrorHandle;
		index = temp;
		return sent;
	ErrorHandle:
		delete sent;
		return nullptr;
	}
	
	SyntaxExpr* ParseLoop(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxLoop* sent = new SyntaxLoop();
		if (tokens[temp++]->value != Token::LOOP)
			goto ErrorHandle;
		if (tokens[temp]->value == Token::LPAREN)
		{
			sent->prefix_condition = ParseExpr(tokens, ++temp, errors);
			if(sent->prefix_condition == nullptr)
				sent->prefix_condition = ParseDeclare(tokens, temp, errors);
			if (tokens[temp]->value == Token::SEMICOLON)
			{
				sent->init = sent->prefix_condition;
				sent->prefix_condition = ParseExpr(tokens, ++temp, errors);
				if (tokens[temp++]->value != Token::SEMICOLON)
				{
					errors.push_back({"Expected semicolon", tokens[--temp]->lines});
					goto ErrorHandle;
				}
				sent->loop = ParseExpr(tokens, temp, errors);
			}
			if (tokens[temp++]->value != Token::RPAREN)
			{
				errors.push_back({"Expected )", tokens[--temp]->lines});
				goto ErrorHandle;
			}
		}
		if ((sent->sents = ParseBlock(tokens, temp, errors)) == nullptr)
			goto ErrorHandle;
		if (tokens[temp]->value == Token::LPAREN)
		{
			if (sent->init || sent->prefix_condition || sent->loop)
			{
				errors.push_back({"Can`t overlap with dislocation declaration", tokens[temp]->lines});
				goto ErrorHandle;
			}
			sent->postfix_condition = ParseExpr(tokens, ++temp, errors);
			if (tokens[temp++]->value != Token::RPAREN)
			{
				errors.push_back({"Expected )", tokens[--temp]->lines});
				goto ErrorHandle;
			}
		}
		index = temp;
		return sent;
	ErrorHandle:
		delete sent;
		return nullptr;
	}
	SyntaxExpr* ParseDeclare(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxDeclare* expr = new SyntaxDeclare();
		VarDesc desc;
		desc.option = VarDesc::VAR;
		expr->line = tokens[temp]->lines;
		switch (tokens[temp++]->value)
		{
		case Token::CONST:
			desc.option = VarDesc::CONST;
		case Token::VAR:
		{
			SyntaxExpr* init = nullptr;
			do
			{
				if (tokens[temp]->value != Token::IDENTIFIER)
					goto ErrorHandle;
				desc.name = tokens[temp++]->literal.s;
				if (tokens[temp]->value == Token::ASSIGN)
				{
					if ((init = ParseAssign(tokens, ++temp, errors)) == nullptr)
					{
						errors.push_back({"Expected expression", tokens[temp]->lines});
						goto ErrorHandle;
					}
				}
				expr->elements.push_back(std::make_pair(desc, init));
			} while (tokens[temp]->value == Token::COMMA);
		}
		break;
		case Token::FUNCTION:
		{
			if (tokens[temp]->value != Token::IDENTIFIER)
				goto ErrorHandle;
			SyntaxFunction* func = new SyntaxFunction();
			expr->elements.push_back(std::make_pair(desc = {tokens[temp++]->literal.s, VarDesc::CONST}, func));
			if (tokens[temp++]->value != Token::LPAREN)
			{
				errors.push_back({"Expected (", tokens[--temp]->lines});
				goto ErrorHandle;
			}
			if (tokens[temp]->value == Token::IDENTIFIER)
			{
				func->params.push_back(tokens[temp++]->literal.s);
				while (tokens[temp]->value == Token::COMMA)
				{
					if (tokens[++temp]->value != Token::IDENTIFIER)
					{
						errors.push_back({"Expected identifier", tokens[--temp]->lines});
						goto ErrorHandle;
					}
					func->params.push_back(tokens[temp++]->literal.s);
				}
			}
			if (tokens[temp++]->value != Token::RPAREN)
			{
				errors.push_back({"Expected )", tokens[--temp]->lines});
				goto ErrorHandle;
			}
			if ((func->sents = ParseBlock(tokens, temp, errors)) == nullptr)
				goto ErrorHandle;
		}
		break;
		case Token::STRUCT:
		{
			if (tokens[temp]->value != Token::IDENTIFIER)
				goto ErrorHandle;
			SyntaxClass* clsinf = new SyntaxClass();
			expr->elements.push_back(std::make_pair(desc = {tokens[temp++]->literal.s, VarDesc::CONST}, clsinf));
			if (tokens[temp++]->value != Token::LBRACE)
			{
				errors.push_back({"Expected {", tokens[--temp]->lines});
				goto ErrorHandle;
			}
			while (SyntaxExpr* expr = ParseDeclare(tokens, temp, errors))
			{
				clsinf->member.push_back(expr);
				while (tokens[temp]->value == Token::SEMICOLON)	++temp;
			}
			if (tokens[temp++]->value != Token::RBRACE)
			{
				errors.push_back({"Expected }", tokens[--temp]->lines});
				goto ErrorHandle;
			}
		}
		break;
		default:
			goto ErrorHandle;
		}		
		index = temp;
		return expr;
	ErrorHandle:
		delete expr;
		return nullptr;
	}
	SyntaxBlock* ParseBlock(std::vector<TokenDesc*>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxBlock* blocks = new SyntaxBlock();
		SyntaxExpr* expr;
		bool check = false;
		if (tokens[temp]->value == Token::LBRACE)
		{
			check = true;
			++temp;
		}
		do
		{
			if (tokens[temp]->value == Token::EOT)
			{
				errors.push_back({"Expected }", tokens[temp - 1]->lines});
				goto ErrorHandle;
			}
			if ((expr = ParseGeneralSentences(tokens, temp, errors)) == nullptr)
				goto ErrorHandle;
			blocks->sents.push_back(expr);
		} while (check && tokens[temp]->value != Token::RBRACE);
		if (tokens[temp]->value == Token::RBRACE)
			++temp;
		index = temp;
		return blocks;
	ErrorHandle:
		delete blocks;
		return nullptr;
	}

	bool SyntaxTree::ParseText(SyntaxTree& code, const std::string& str)
	{
		std::vector<TokenDesc*> result;
		if (!Scanner::Tokenize(str.c_str(), result))
			return false;

		// for (TokenDesc *token : result)
		// 	printf("[%s]\n", token->GetInfo());

		size_t index = 0;
		while (result[index]->value != Token::EOT)
		{
			SyntaxExpr* sent;
			if ((sent = ParseGeneralSentences(result, index, code.errors)) == nullptr)
				return false;
			code.sents.push_back(sent);
		}
		return true;
	}
}