#include"pch.h"
#include"parser.h"

#define ParseGeneralSentences(tokens, index, errors) ParseSentence(tokens, index, errors, 4, ParseIf, ParseLoop, ParseExpr, ParseDeclare)

namespace myscript 
{
	SyntaxExpr* ParseExpr(std::vector<Token> &tokens, size_t &index, std::vector<Error> &errors)
	{
		return ParseComma(tokens, index, errors);
	}
	SyntaxExpr* ParseComma(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParseAssign(tokens, temp, errors)) == nullptr)
			return nullptr;
		while (tokens[temp].type == Token::COMMA)
		{
			if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected expression", tokens[--temp].line});
				delete lexp;
				return nullptr;
			}
			lexp = new SyntaxComma(lexp, rexp);
		}
		index = temp;
		return lexp;
	}
	SyntaxExpr* ParseAssign(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParseOr(tokens, temp, errors)) == nullptr)
			return nullptr;
		switch (tokens[temp].type)
		{
		case Token::ASSIGN:
			if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
				goto ErrorHandle;
			}
			lexp = new SyntaxAssign(lexp, rexp);
			break;
		case Token::ASSIGN_ADD:
			if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
				goto ErrorHandle;
			}
			lexp = new SyntaxAssign(lexp, new SyntaxAdd(lexp, rexp));
			break;
		case Token::ASSIGN_SUB:
			if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
				goto ErrorHandle;
			}
			lexp = new SyntaxAssign(lexp, new SyntaxSub(lexp, rexp));
			break;
		case Token::ASSIGN_MUL:
			if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
				goto ErrorHandle;
			}
			lexp = new SyntaxAssign(lexp, new SyntaxMul(lexp, rexp));
			break;
		case Token::ASSIGN_DIV:
			if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
				goto ErrorHandle;
			}
			lexp = new SyntaxAssign(lexp, new SyntaxDiv(lexp, rexp));
			break;
		case Token::ASSIGN_MOD:
			if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
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
	SyntaxExpr* ParseOr(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParseAnd(tokens, temp, errors)) == nullptr)
			return nullptr;
		while (tokens[temp].type == Token::OR)
		{
			if ((rexp = ParseAnd(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
				delete lexp;
				return nullptr;
			}
			lexp = new SyntaxOr(lexp, rexp);
		}
		index = temp;
		return lexp;
	}
	SyntaxExpr* ParseAnd(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParseCmp(tokens, temp, errors)) == nullptr)
			return nullptr;
		while (tokens[temp].type == Token::AND)
		{
			if ((rexp = ParseCmp(tokens, ++temp, errors)) != nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
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
	SyntaxExpr* ParseCmp(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParseAdd(tokens, temp, errors)) == nullptr)
			return nullptr;
		switch (tokens[temp].type)
		{
		case Token::EQ:
			if ((rexp = ParseAdd(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
				goto ErrorHandle;
			}
			lexp = new SyntaxEqual(lexp, rexp);
			break;
		case Token::NEQ:
			if ((rexp = ParseAdd(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
				goto ErrorHandle;
			}
			lexp = new SyntaxNotEqual(lexp, rexp);
			break;
		case Token::GT:
			if ((rexp = ParseAdd(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
				goto ErrorHandle;
			}
			lexp = new SyntaxGreatThan(lexp, rexp);
			break;
		case Token::GE:
			if ((rexp = ParseAdd(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
				goto ErrorHandle;
			}
			lexp = new SyntaxGreatEqual(lexp, rexp);
			break;
		case Token::LT:
			if ((rexp = ParseAdd(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
				goto ErrorHandle;
			}
			lexp = new SyntaxLessThan(lexp, rexp);
			break;
		case Token::LE:
			if ((rexp = ParseAdd(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
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
	SyntaxExpr* ParseAdd(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParseMul(tokens, temp, errors)) == nullptr)
			return nullptr;
	_loop_:
		switch (tokens[temp].type)
		{
		case Token::ADD:
			if ((rexp = ParseMul(tokens, ++temp, errors)) != nullptr)
			{
				lexp = new SyntaxAdd(lexp, rexp);
				goto _loop_;
			}
			errors.push_back({"Expected expression", tokens[--temp].line});
			break;
		case Token::SUB:
			if ((rexp = ParseMul(tokens, ++temp, errors)) != nullptr)
			{
				lexp = new SyntaxSub(lexp, rexp);
				goto _loop_;
			}
			errors.push_back({"Expected expression", tokens[--temp].line});
			break;
		}
		index = temp;
		return lexp;
	}
	SyntaxExpr* ParseMul(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParsePow(tokens, temp, errors)) == nullptr)
			return nullptr;
	_loop_:
		switch (tokens[temp].type)
		{
		case Token::MUL:
			if ((rexp = ParseMul(tokens, ++temp, errors)) != nullptr)
			{
				lexp = new SyntaxMul(lexp, rexp);
				goto _loop_;
			}
			errors.push_back({"Expected expression", tokens[--temp].line});
			break;
		case Token::DIV:
			if ((rexp = ParseMul(tokens, ++temp, errors)) != nullptr)
			{
				lexp = new SyntaxDiv(lexp, rexp);
				goto _loop_;
			}
			errors.push_back({"Expected expression", tokens[--temp].line});
			break;
		case Token::MOD:
			if ((rexp = ParseMul(tokens, ++temp, errors)) != nullptr)
			{
				lexp = new SyntaxMod(lexp, rexp);
				goto _loop_;
			}
			errors.push_back({"Expected expression", tokens[--temp].line});
			break;
		}
		index = temp;
		return lexp;
	}
	SyntaxExpr* ParsePow(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParsePrefix(tokens, temp, errors)) == nullptr)
			return nullptr;
		while (tokens[temp].type == Token::POW)
		{
			if ((rexp = ParseMul(tokens, ++temp, errors)) != nullptr)
			{
				errors.push_back({"Expected expression", tokens[--temp].line});
				break;
			}
			lexp = new SyntaxPow(lexp, rexp);
		}
		index = temp;
		return lexp;
	}
	SyntaxExpr* ParsePrefix(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* expr = nullptr;
		switch (tokens[temp].type)
		{
		case Token::INC:
			if ((expr = ParsePostfix(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
				return nullptr;
			}
			expr = new SyntaxPrefixPlusx2(expr);
			break;
		case Token::DEC:
			if ((expr = ParsePostfix(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
				return nullptr;
			}
			expr = new SyntaxPostfixMinusx2(expr);
			break;
		case Token::ADD:
			if ((expr = ParsePostfix(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
				return nullptr;
			}
			expr = new SyntaxPrefixPlus(expr);
			break;
		case Token::SUB:
			if ((expr = ParsePostfix(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
				return nullptr;
			}
			expr = new SyntaxPrefixMinus(expr);
			break;
		case Token::NOT:
			if ((expr = ParsePostfix(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
				return nullptr;
			}
			expr = new SyntaxNot(expr);
			break;
		case Token::NEW:
			if ((expr = ParsePostfix(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[--temp].line});
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
	SyntaxExpr* ParsePostfix(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* lexp = nullptr;
		SyntaxExpr* rexp = nullptr;
		if ((lexp = ParseElement(tokens, temp, errors)) == nullptr)
			return nullptr;
	_loop_:
		switch (tokens[temp].type)
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
				errors.push_back({"Expected identifier", tokens[temp + 1].line});
				goto ErrorHandle;
			}
			lexp = new SyntaxAs(lexp, rexp);
			break;
		case Token::IS:
			if ((rexp = ParseElement(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[temp + 1].line});
				goto ErrorHandle;
			}
			lexp = new SyntaxIs(lexp, rexp);
			break;
		case Token::DOT:
			if (tokens[++temp].type != Token::IDENTIFIER)
			{
				errors.push_back({"Expected identifier", tokens[temp + 1].line});
				goto ErrorHandle;
			}
			lexp = new SyntaxDot(lexp, tokens[temp++].str);
			goto _loop_;
		case Token::LSUBRACKET:
			if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", tokens[temp + 1].line});
				goto ErrorHandle;
			}
			if (tokens[temp++].type != Token::RSUBRACKET)
			{
				errors.push_back({"Expected ]", tokens[temp + 1].line});
				delete rexp;
				goto ErrorHandle;
			}
			lexp = new SyntaxRef(lexp, rexp);
			goto _loop_;
		case Token::LPARAM:
			SyntaxCall* expr = new SyntaxCall(lexp);
			lexp = expr;
			if ((rexp = ParseAssign(tokens, ++temp, errors)) != nullptr)
			{
				expr->params.push_back(rexp);
				while (tokens[temp].type == Token::COMMA)
				{
					if ((rexp = ParseAssign(tokens, ++temp, errors)) == nullptr)
					{
						errors.push_back({"Expected expression", tokens[--temp].line});
						goto ErrorHandle;
					}
					expr->params.push_back(rexp);
				}
			}
			if (tokens[temp++].type != Token::RPARAM)
			{
				errors.push_back({"Expected )", tokens[--temp].line});
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
	SyntaxExpr* ParseElement(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* expr = nullptr;
		switch (tokens[temp].type)
		{
		case Token::LPARAM:
			if ((expr = ParseExpr(tokens, ++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected expression", tokens[temp].line});
				goto ErrorHandle;
			}
			if (tokens[temp++].type != Token::RPARAM)
			{
				errors.push_back({"Expected )", tokens[temp].line});
				goto ErrorHandle;
			}
			break;
		case Token::SELF:
			expr = new SyntaxSelf(tokens[temp++].line);
			break;
		case Token::IDENTIFIER:
			expr = new SyntaxIdentifier(tokens[temp++]);
			break;
		case Token::FUNCTION:
			if ((expr = ParseFunction(tokens, temp, errors)) == nullptr)
				goto ErrorHandle;
			break;
		case Token::CONTINUE:
			expr = new SyntaxContinue(tokens[temp++].line);
			break;
		case Token::BREAK:
			expr = new SyntaxBreak(tokens[temp++].line);
			break;
		case Token::RETURN:
			expr = new SyntaxReturn(ParseAssign(tokens, ++temp, errors));
			break;
		case Token::NEW:
			if ((tokens[++temp].type != Token::IDENTIFIER))
			{
				errors.push_back({"Operator 'new' must place behind type", tokens[temp].line});
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
	SyntaxExpr* ParseLiteral(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* expr = nullptr;
		if ((expr = ParseSimpleLiteral(tokens, temp, errors)) || (expr = ParseArray(tokens, temp, errors)) || (expr = ParseDictionary(tokens, temp, errors)))
			index = temp;
		return expr;
	}
	SyntaxExpr* ParseSimpleLiteral(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		switch (tokens[index].type)
		{
		case Token::NUMBER:
		case Token::STRING:
		case Token::TRUE:
		case Token::FALSE:
		case Token::NULLPTR:
			return new SyntaxLiteral(tokens[index++]);
		}
		return nullptr;
	}
	SyntaxExpr* ParseArray(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxArray* expr = new SyntaxArray();
		SyntaxExpr* element;
		if (tokens[temp++].type != Token::LSUBRACKET)
			goto ErrorHandle;
		if ((element = ParseAssign(tokens, temp, errors)) != nullptr)
		{
			expr->elements.push_back(element);
			while (tokens[temp].type == Token::COMMA)
			{
				element = ParseAssign(tokens, ++temp, errors);
				if (element == nullptr)
				{
					errors.push_back({"Expected element", tokens[temp].line});
					goto ErrorHandle;
				}
				expr->elements.push_back(element);
			}
		}
		if (tokens[temp++].type != Token::RSUBRACKET)
		{
			errors.push_back({"Expected ]", tokens[temp].line});
			goto ErrorHandle;
		}
		if (tokens[temp].type == Token::LT)
		{
			if (tokens[++temp].type == Token::NUMBER)
			{
				size_t init = static_cast<size_t>(stoi(tokens[temp++].str));
				for (size_t index = expr->elements.size(); index < init; ++index)
					expr->elements.push_back(new SyntaxLiteral({Token::NULLPTR, "", 0}));
			}
			else
			{
				errors.push_back({"Expected number", tokens[temp].line});
				goto ErrorHandle;
			}
			if (tokens[temp++].type != Token::GT)
				goto ErrorHandle;
		}
		index = temp;
		return expr;
	ErrorHandle:
		delete expr;
		return nullptr;
	}
	std::pair<SyntaxExpr*, SyntaxExpr*>* ParseKeyVal(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxExpr* key;
		SyntaxExpr* value;
		if (tokens[temp].type == Token::IDENTIFIER)
			key = new SyntaxLiteral({Token::STRING, tokens[temp].str, tokens[temp].line}), ++temp;
		else if ((key = ParseLiteral(tokens, temp, errors)) == NULL)
			return NULL;
		if (tokens[temp++].type != Token::COLON)
			goto ErrorHandle;
		if ((value = ParseAssign(tokens, temp, errors)) == NULL)
		{
			errors.push_back({"Expected value", tokens[temp].line});
			goto ErrorHandle;
		}
		index = temp;
		return new std::pair<SyntaxExpr*, SyntaxExpr*>(key, value);
	ErrorHandle:
		delete key;
		return NULL;
	}
	SyntaxExpr* ParseDictionary(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxDictionary* dict = new SyntaxDictionary();
		std::pair<SyntaxExpr*, SyntaxExpr*>* key;
		if (tokens[temp++].type != Token::LBRACKET)
			goto ErrorHandle;
		if ((key = ParseKeyVal(tokens, temp, errors)) != NULL)
		{
			dict->elements.push_back(*key);
			while (tokens[temp].type == Token::COMMA)
			{
				if ((key = ParseKeyVal(tokens, ++temp, errors)) == NULL)
				{
					errors.push_back({"Expected key and value", tokens[temp].line});
					goto ErrorHandle;
				}
				dict->elements.push_back(*key);
			}
		}
		if (tokens[temp++].type != Token::RBRACKET)
		{
			errors.push_back({"Expected }", tokens[temp].line});
			goto ErrorHandle;
		}
		index = temp;
		return dict;
	ErrorHandle:
		delete dict;
		return NULL;
	}
	SyntaxExpr* ParseFunction(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxFunction* expr = new SyntaxFunction();
		if (tokens[temp++].type != Token::FUNCTION)
			goto ErrorHandle;
		if (tokens[temp++].type != Token::LPARAM)
			goto ErrorHandle;
		if (tokens[temp].type == Token::IDENTIFIER)
		{
			expr->params.push_back(tokens[temp++].str);
			while (tokens[temp].type == Token::COMMA)
			{
				if (tokens[++temp].type != Token::IDENTIFIER)
				{
					errors.push_back({"Expected identifier", tokens[--temp].line});
					goto ErrorHandle;
				}
				expr->params.push_back(tokens[temp++].str);
			}
		}
		if (tokens[temp++].type != Token::RPARAM)
		{
			errors.push_back({"Expected )", tokens[--temp].line});
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
	SyntaxExpr* ParseSentence(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors, size_t args_count ...)
	{
		size_t temp = index;
		SyntaxExpr* sent = nullptr;
		va_list args_ptr;
		va_start(args_ptr, args_count);
		for (size_t count = 0; count < args_count; ++count)
			if ((sent = va_arg(args_ptr, SyntaxExpr* (*)(std::vector<Token>&, size_t&, std::vector<Error>&))(tokens, temp, errors)) != nullptr)
				break;
		va_end(args_ptr);
		while (tokens[temp].type == Token::SEMICOLON)	++temp;
		index = temp;
		return sent;
	}
	SyntaxExpr* ParseIf(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxIf* sent = new SyntaxIf();
		if (tokens[temp++].type != Token::IF)
			goto ErrorHandle;
		if (tokens[temp++].type != Token::LPARAM)
		{
			errors.push_back({"Expected (", tokens[--temp].line});
			goto ErrorHandle;
		}
		if ((sent->cond = ParseExpr(tokens, temp, errors)) == nullptr)
		{
			errors.push_back({"Expected expression", tokens[--temp].line});
			goto ErrorHandle;
		}
		if (tokens[temp++].type != Token::RPARAM)
		{
			errors.push_back({"Expected )", tokens[--temp].line});
			goto ErrorHandle;
		}
		if ((sent->truesents = ParseBlock(tokens, temp, errors)) == nullptr)
			goto ErrorHandle;
		if (tokens[temp].type == Token::ELSE && (sent->falsesents = ParseBlock(tokens, ++temp, errors)) == nullptr)
			goto ErrorHandle;
		index = temp;
		return sent;
	ErrorHandle:
		delete sent;
		return nullptr;
	}
	SyntaxExpr* ParseLoop(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxLoop* sent = new SyntaxLoop();
		if (tokens[temp++].type != Token::LOOP)
			goto ErrorHandle;
		if (tokens[temp].type == Token::LPARAM)
		{
			sent->prefix_condition = ParseExpr(tokens, ++temp, errors);
			if(sent->prefix_condition == nullptr)
				sent->prefix_condition = ParseDeclare(tokens, temp, errors);
			if (tokens[temp].type == Token::SEMICOLON)
			{
				sent->init = sent->prefix_condition;
				sent->prefix_condition = ParseExpr(tokens, ++temp, errors);
				if (tokens[temp++].type != Token::SEMICOLON)
				{
					errors.push_back({"Expected semicolon", tokens[--temp].line});
					goto ErrorHandle;
				}
				sent->loop = ParseExpr(tokens, temp, errors);
			}
			if (tokens[temp++].type != Token::RPARAM)
			{
				errors.push_back({"Expected )", tokens[--temp].line});
				goto ErrorHandle;
			}
		}
		if ((sent->sents = ParseBlock(tokens, temp, errors)) == nullptr)
			goto ErrorHandle;
		if (tokens[temp].type == Token::LPARAM)
		{
			if (sent->init || sent->prefix_condition || sent->loop)
			{
				errors.push_back({"Can`t overlap with dislocation declaration", tokens[temp].line});
				goto ErrorHandle;
			}
			sent->postfix_condition = ParseExpr(tokens, ++temp, errors);
			if (tokens[temp++].type != Token::RPARAM)
			{
				errors.push_back({"Expected )", tokens[--temp].line});
				goto ErrorHandle;
			}
		}
		index = temp;
		return sent;
	ErrorHandle:
		delete sent;
		return nullptr;
	}
	SyntaxExpr* ParseDeclare(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxDeclare* expr = new SyntaxDeclare();
		VarDesc desc;
		desc.option = VarDesc::VAR;
		expr->line = tokens[temp].line;
		switch (tokens[temp++].type)
		{
		case Token::CONST:
			desc.option = VarDesc::CONST;
		case Token::VAR:
		{
			SyntaxExpr* init = nullptr;
			do
			{
				if (tokens[temp].type != Token::IDENTIFIER)
					goto ErrorHandle;
				desc.name = tokens[temp++].str;
				if (tokens[temp].type == Token::ASSIGN)
				{
					if ((init = ParseAssign(tokens, ++temp, errors)) == nullptr)
					{
						errors.push_back({"Expected expression", tokens[temp].line});
						goto ErrorHandle;
					}
				}
				expr->elements.push_back(std::make_pair(desc, init));
			} while (tokens[temp].type == Token::COMMA);
		}
		break;
		case Token::FUNCTION:
		{
			if (tokens[temp].type != Token::IDENTIFIER)
				goto ErrorHandle;
			SyntaxFunction* func = new SyntaxFunction();
			expr->elements.push_back(std::make_pair(desc = {tokens[temp++].str, VarDesc::CONST}, func));
			if (tokens[temp++].type != Token::LPARAM)
			{
				errors.push_back({"Expected (", tokens[--temp].line});
				goto ErrorHandle;
			}
			if (tokens[temp].type == Token::IDENTIFIER)
			{
				func->params.push_back(tokens[temp++].str);
				while (tokens[temp].type == Token::COMMA)
				{
					if (tokens[++temp].type != Token::IDENTIFIER)
					{
						errors.push_back({"Expected identifier", tokens[--temp].line});
						goto ErrorHandle;
					}
					func->params.push_back(tokens[temp++].str);
				}
			}
			if (tokens[temp++].type != Token::RPARAM)
			{
				errors.push_back({"Expected )", tokens[--temp].line});
				goto ErrorHandle;
			}
			if ((func->sents = ParseBlock(tokens, temp, errors)) == nullptr)
				goto ErrorHandle;
		}
		break;
		case Token::CLASS:
		{
			if (tokens[temp].type != Token::IDENTIFIER)
				goto ErrorHandle;
			SyntaxClass* clsinf = new SyntaxClass();
			expr->elements.push_back(std::make_pair(desc = {tokens[temp++].str, VarDesc::CONST}, clsinf));
			if (tokens[temp++].type != Token::LBRACKET)
			{
				errors.push_back({"Expected {", tokens[--temp].line});
				goto ErrorHandle;
			}
			while (SyntaxExpr* expr = ParseDeclare(tokens, temp, errors))
			{
				clsinf->member.push_back(expr);
				while (tokens[temp].type == Token::SEMICOLON)	++temp;
			}
			if (tokens[temp++].type != Token::RBRACKET)
			{
				errors.push_back({"Expected }", tokens[--temp].line});
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
	SyntaxBlock* ParseBlock(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors)
	{
		size_t temp = index;
		SyntaxBlock* blocks = new SyntaxBlock();
		SyntaxExpr* expr;
		bool check = false;
		if (tokens[temp].type == Token::LBRACKET)
		{
			check = true;
			++temp;
		}
		do
		{
			if (tokens[temp].type == Token::EOT)
			{
				errors.push_back({"Expected }", tokens[temp - 1].line});
				goto ErrorHandle;
			}
			if ((expr = ParseGeneralSentences(tokens, temp, errors)) == nullptr)
				goto ErrorHandle;
			blocks->sents.push_back(expr);
		} while (check && tokens[temp].type != Token::RBRACKET);
		if (tokens[temp].type == Token::RBRACKET)
			++temp;
		index = temp;
		return blocks;
	ErrorHandle:
		delete blocks;
		return nullptr;
	}
	static void Tokenize(std::vector<Token>& tokens, const std::string& str)
	{
		size_t str_size = str.size();
		std::string temp = "";
		size_t marker = 0;
		size_t lines = 1;
		Token::Type predicted = Token::NONE;
		for (size_t index = 0; index < str_size; ++index)
		{
			switch (predicted)
			{
			case Token::NONE:
				switch (str[index])
				{
				case ' ': case '\f': case '\t': case '\v':
					marker = index + 1;
					break;
				case '\"':
					predicted = Token::STRING;
					marker = index + 1;
					break;
				case '\r':case '\n':
					marker = index + 1;
					++lines;
					break;
				case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':
					predicted = Token::NUMBER;
					marker = index;
					break;
				case ':':
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
					marker = index;
					predicted = Token::IDENTIFIER;
					break;
				}
				break;
			case Token::STRING:
				switch (str[index])
				{
				case '\"':
					tokens.push_back({predicted, temp, lines});
					predicted = Token::NONE;
					marker = index + 1;
					temp.clear();
					break;
				case '\n':
					++lines;
					break;
				case '\\':
					switch (str[index + 1])
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
					}
					++index;
					break;
				default:
					temp += str[index];
					break;
				}
				break;
			case Token::NUMBER:
				if(!(str[index] >= '0' && str[index] <= '9'))
				{
					tokens.push_back({predicted, std::string(str, marker, index - marker), lines});
					predicted = Token::NONE;
					index--;
				}
				break;
			case Token::IDENTIFIER:
				if(!(str[index] >= 'a' && str[index] <= 'z') && !(str[index] >= 'A' && str[index] <= 'Z') && !(str[index] >= '0' && str[index] <= '9') && str[index] != '_')
				{
					std::string id = std::string(str, marker, index - marker);
					tokens.push_back({id == "if" ? Token::IF :
		 			id == "else" ? Token::ELSE :
					id == "match" ? Token::MATCh :
					id == "loop" ? Token::LOOP :
					id == "default" ? Token::DEFAULT :
					id == "continue" ? Token::CONTINUE :
					id == "break" ? Token::BREAK :
					id == "return" ? Token::RETURN :
					id == "true" ? Token::TRUE :
					id == "false" ? Token::FALSE :
					id == "null" ? Token::NULLPTR :
					id == "new" ? Token::NEW :
					id == "self" ? Token::SELF :
					id == "is" ? Token::IS :
					id == "as" ? Token::AS :
					id == "var" ? Token::VAR :
					id == "const" ? Token::CONST :
					id == "static" ? Token::STATIC :
					id == "function" ? Token::FUNCTION :
					id == "class" ? Token::CLASS :
					id == "public" ? Token::PUBLIC :
					id == "private" ? Token::PRIVATE :
					id == "protect" ? Token::PROTECT : Token::IDENTIFIER, id, lines});
					predicted = Token::NONE;
					index--;
				}
				break;
			case Token::COMMENTLINE:
				if (str[index] == '\n')
				{
					predicted = Token::NONE;
					marker = index + 1;
					++lines;
				}
				break;
			case Token::COMMENTBLOCK:
				if (str[index] == '*' && str[index + 1] == '/')
				{
					predicted = Token::NONE;
					marker = ++index + 1;
				}
				else if (str[index] == '\n')
					++lines;
				break;
			}
		}
		if (marker < str_size)
			tokens.push_back({predicted, std::string(str, marker, str_size - marker), lines});
		tokens.push_back({Token::EOT, "[eof]", lines});
	}
	bool SyntaxTree::ParseText(SyntaxTree& code, const std::string& str)
	{
		std::vector<Token> tokens;
		Tokenize(tokens, str);
		// for (auto& token : tokens)
		// 	printf("[%d] %s\n", token.type, token.str.c_str());
		size_t index = 0;
		while (tokens[index].type != Token::EOT)
		{
			SyntaxExpr* sent;
			if ((sent = ParseGeneralSentences(tokens, index, code.errors)) == nullptr)
				return false;
			code.sents.push_back(sent);
		}
		return true;
	}
}

