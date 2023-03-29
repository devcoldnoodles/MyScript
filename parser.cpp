#include "parser.h"

using namespace myscript;
#define ParseGeneralSentences(iter, errors) ParseSentence(iter, errors, 4, ParseIf, ParseLoop, ParseExpr, ParseDeclare)

namespace myscript
{
	SyntaxExpr *ParseExpr(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		return ParseChain(iter, errors);
	}

	SyntaxExpr *ParseChain(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxExpr *lexp = nullptr;
		SyntaxExpr *rexp = nullptr;

		if ((lexp = ParseAssign(temp, errors)) == nullptr)
			return nullptr;

		while (temp->value == Token::COMMA)
		{
			if ((rexp = ParseAssign(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected expression", (--temp)->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxChain(lexp, rexp);
		}

		iter = temp;
		return lexp;
	ErrorHandle:
		delete lexp;
		delete rexp;
		return nullptr;
	}

	SyntaxExpr *ParseAssign(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxExpr *lexp = nullptr;
		SyntaxExpr *rexp = nullptr;

		if ((lexp = ParseTernaryOperator(temp, errors)) == nullptr)
			return nullptr;

		short value = temp->value;
		switch (value)
		{
		case Token::ASSIGN:
			if ((rexp = ParseAssign(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", (--temp)->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxAssign(lexp, rexp);
			break;
		case Token::ASSIGN_ADD:
		case Token::ASSIGN_SUB:
		case Token::ASSIGN_MUL:
		case Token::ASSIGN_DIV:
		case Token::ASSIGN_MOD:
			if ((rexp = ParseAssign(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", (--temp)->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxAssign(lexp, new SyntaxBinaryOperator(lexp, rexp, (OpCode)(value - (short)Token::ASSIGN_ADD + (short)OpCode::ADD)));
			break;
		}

		iter = temp;
		return lexp;
	ErrorHandle:
		delete lexp;
		return nullptr;
	}

	SyntaxExpr *ParseTernaryOperator(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxExpr *lexpr = nullptr;
		SyntaxExpr *expr = nullptr;
		SyntaxExpr *rexpr = nullptr;

		if ((lexpr = ParseLogicalOR(temp, errors)) == nullptr)
			return nullptr;

		if (temp->value == Token::CONDITIONAL)
		{
			if ((expr = ParseTernaryOperator(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected Expression", temp->lines});
				goto ErrorHandle;
			}

			if (temp->value != Token::COLON)
			{
				errors.push_back({"Expected colon(:)", temp->lines});
				return nullptr;
			}
			if ((rexpr = ParseTernaryOperator(++temp, errors)) == nullptr)
			{
				delete expr;
				goto ErrorHandle;
			}
			lexpr = new SyntaxTernaryOperator(lexpr, expr, rexpr);
		}

		iter = temp;
		return lexpr;
	ErrorHandle:
		delete lexpr;
		return nullptr;
	}

	SyntaxExpr *ParseLogicalOR(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxExpr *lexp = nullptr;
		SyntaxExpr *rexp = nullptr;

		if ((lexp = ParseLogicalAND(temp, errors)) == nullptr)
			return nullptr;

		while (temp->value == Token::OR)
		{
			if ((rexp = ParseLogicalAND(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", (--temp)->lines});
				delete lexp;
				return nullptr;
			}
			lexp = new SyntaxBinaryOperator(lexp, rexp, OR);
		}

		iter = temp;
		return lexp;
	}

	SyntaxExpr *ParseLogicalAND(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxExpr *lexp = nullptr;
		SyntaxExpr *rexp = nullptr;

		if ((lexp = ParseCompare(temp, errors)) == nullptr)
			return nullptr;

		while (temp->value == Token::AND)
		{
			if ((rexp = ParseCompare(++temp, errors)) != nullptr)
			{
				errors.push_back({"Expected identifier", (--temp)->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxBinaryOperator(lexp, rexp, AND);
		}

		iter = temp;
		return lexp;
	ErrorHandle:
		delete lexp;
		return nullptr;
	}
	SyntaxExpr *ParseCompare(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxExpr *lexp = nullptr;
		SyntaxExpr *rexp = nullptr;

		if ((lexp = ParseAdd(temp, errors)) == nullptr)
			return nullptr;

		short value = temp->value;
		switch (value)
		{
		case Token::EQ:
		case Token::NEQ:
		case Token::GT:
		case Token::GE:
		case Token::LT:
		case Token::LE:
			if ((rexp = ParseAdd(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", (--temp)->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxBinaryOperator(lexp, rexp, (OpCode)(value - (short)Token::EQ + (short)OpCode::EQ));
			break;
		}

		iter = temp;
		return lexp;
	ErrorHandle:
		delete lexp;
		return nullptr;
	}

	SyntaxExpr *ParseAdd(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxExpr *lexp = nullptr;
		SyntaxExpr *rexp = nullptr;

		if ((lexp = ParseMul(temp, errors)) == nullptr)
			return nullptr;

		while (temp->value == Token::ADD ||
			   temp->value == Token::SUB)
		{
			short value = temp->value;
			if ((rexp = ParseMul(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected expression", (--temp)->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxBinaryOperator(lexp, rexp, (OpCode)(value - (short)Token::ADD + (short)OpCode::ADD));
		}

		iter = temp;
		return lexp;
	ErrorHandle:
		delete lexp;
		return nullptr;
	}

	SyntaxExpr *ParseMul(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxExpr *lexp = nullptr;
		SyntaxExpr *rexp = nullptr;

		if ((lexp = ParsePrefix(temp, errors)) == nullptr)
			return nullptr;

		while (temp->value == Token::MUL ||
			   temp->value == Token::DIV ||
			   temp->value == Token::MOD ||
			   temp->value == Token::POW)
		{
			short value = temp->value;
			if ((rexp = ParseMul(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected expression", (--temp)->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxBinaryOperator(lexp, rexp, (OpCode)(value - (short)Token::MUL + (short)OpCode::MUL));
		}

		iter = temp;
		return lexp;
	ErrorHandle:
		delete lexp;
		return nullptr;
	}

	SyntaxExpr *ParsePrefix(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxExpr *expr = nullptr;
		switch (temp->value)
		{
		case Token::INC:
			if ((expr = ParseSuffix(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", (--temp)->lines});
				return nullptr;
			}
			expr = new SyntaxPrefixPlusx2(expr);
			break;
		case Token::DEC:
			if ((expr = ParseSuffix(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", (--temp)->lines});
				return nullptr;
			}
			expr = new SyntaxPostfixMinusx2(expr);
			break;
		case Token::ADD:
			if ((expr = ParseSuffix(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", (--temp)->lines});
				return nullptr;
			}
			expr = new SyntaxUnaryOperator(expr, OpCode::NONE);
			break;
		case Token::SUB:
			if ((expr = ParseSuffix(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", (--temp)->lines});
				return nullptr;
			}
			expr = new SyntaxUnaryOperator(expr, OpCode::SIGN);
			break;
		case Token::NOT:
			if ((expr = ParseSuffix(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", (--temp)->lines});
				return nullptr;
			}
			expr = new SyntaxNot(expr);
			break;
		case Token::NEW:
			if ((expr = ParseSuffix(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", (--temp)->lines});
				return nullptr;
			}
			expr = new SyntaxNew(expr);
			break;
		default:
			if ((expr = ParseSuffix(temp, errors)) == nullptr)
				return nullptr;
			break;
		}
		iter = temp;
		return expr;
	}

	SyntaxExpr *ParseSuffix(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxExpr *lexp = nullptr;
		SyntaxExpr *rexp = nullptr;
		if ((lexp = ParseElement(temp, errors)) == nullptr)
			return nullptr;
	_loop_:
		switch (temp->value)
		{
		case Token::INC:
			lexp = new SyntaxPostfixPlusx2(lexp), ++temp;
			break;
		case Token::DEC:
			lexp = new SyntaxPostfixMinusx2(lexp), ++temp;
			break;
		case Token::AS:
			if ((rexp = ParseElement(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", (temp + 1)->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxAs(lexp, rexp);
			break;
		case Token::IS:
			if ((rexp = ParseElement(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", (temp + 1)->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxIs(lexp, rexp);
			break;
		case Token::PERIOD:
			if ((++temp)->value != Token::IDENTIFIER)
			{
				errors.push_back({"Expected identifier", temp->lines});
				goto ErrorHandle;
			}
			lexp = new SyntaxAccess(lexp, new SyntaxLiteral(*(temp++)));
			goto _loop_;
		case Token::LBRACKET:
			if ((rexp = ParseAssign(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected identifier", (temp + 1)->lines});
				goto ErrorHandle;
			}
			if ((temp++)->value != Token::RBRACKET)
			{
				errors.push_back({"Expected ]", (temp + 1)->lines});
				delete rexp;
				goto ErrorHandle;
			}
			lexp = new SyntaxRef(lexp, rexp);
			goto _loop_;
		case Token::LPAREN:
			SyntaxCall *expr = new SyntaxCall(lexp);
			lexp = expr;
			if ((rexp = ParseAssign(++temp, errors)) != nullptr)
			{
				expr->params.push_back(rexp);
				while (temp->value == Token::COMMA)
				{
					if ((rexp = ParseAssign(++temp, errors)) == nullptr)
					{
						errors.push_back({"Expected expression", (--temp)->lines});
						goto ErrorHandle;
					}
					expr->params.push_back(rexp);
				}
			}
			if ((temp++)->value != Token::RPAREN)
			{
				errors.push_back({"Expected )", (--temp)->lines});
				delete rexp;
				goto ErrorHandle;
			}
			goto _loop_;
		}
		iter = temp;
		return lexp;
	ErrorHandle:
		delete lexp;
		return nullptr;
	}
	SyntaxExpr *ParseElement(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxExpr *expr = nullptr;
		switch (temp->value)
		{
		case Token::LPAREN:
			if ((expr = ParseExpr(++temp, errors)) == nullptr)
			{
				errors.push_back({"Expected expression", temp->lines});
				goto ErrorHandle;
			}
			if ((temp++)->value != Token::RPAREN)
			{
				errors.push_back({"Expected )", temp->lines});
				goto ErrorHandle;
			}
			break;
		case Token::SELF:
			expr = new SyntaxSelf((temp++)->lines);
			break;
		case Token::IDENTIFIER:
			expr = new SyntaxIdentifier(*temp++);
			break;
		case Token::FUNCTION:
			if ((expr = ParseFunction(temp, errors)) == nullptr)
				goto ErrorHandle;
			break;
		case Token::CONTINUE:
			expr = new SyntaxContinue((temp++)->lines);
			break;
		case Token::BREAK:
			expr = new SyntaxBreak((temp++)->lines);
			break;
		case Token::RETURN:
			expr = new SyntaxReturn(ParseAssign(++temp, errors));
			break;
		case Token::NEW:
			if ((++temp)->value != Token::IDENTIFIER)
			{
				errors.push_back({"Operator 'new' must place behind type", temp->lines});
				goto ErrorHandle;
			}
			expr = new SyntaxNew(new SyntaxIdentifier(*temp++));
			break;
		default:
			if ((expr = ParseLiteral(temp, errors)) == nullptr)
				return nullptr;
			break;
		}
		iter = temp;
		return expr;
	ErrorHandle:
		delete expr;
		return nullptr;
	}

	SyntaxExpr *ParseLiteral(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxExpr *expr = nullptr;
		if ((expr = ParseSimpleLiteral(temp, errors)) || (expr = ParseArray(temp, errors)) || (expr = ParseObject(temp, errors)))
			iter = temp;
		return expr;
	}

	SyntaxExpr *ParseSimpleLiteral(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		switch (iter->value)
		{
		case Token::LITERAL_INTEGER:
		case Token::LITERAL_FLOAT:
		case Token::LITERAL_STRING:
		case Token::LITERAL_TRUE:
		case Token::LITERAL_FALSE:
		case Token::LITERAL_CHAR:
		case Token::LITERAL_NULL:
			return new SyntaxLiteral(*iter++);
		default:
			return nullptr;
		}
	}
	SyntaxExpr *ParseArray(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxArray *expr = new SyntaxArray();
		SyntaxExpr *element;
		if ((temp++)->value != Token::LBRACKET)
			goto ErrorHandle;
		if ((element = ParseAssign(temp, errors)) != nullptr)
		{
			expr->elements.push_back(element);
			while (temp->value == Token::COMMA)
			{
				element = ParseAssign(++temp, errors);
				if (element == nullptr)
				{
					errors.push_back({"Expected element", temp->lines});
					goto ErrorHandle;
				}
				expr->elements.push_back(element);
			}
		}
		if ((temp++)->value != Token::RBRACKET)
		{
			errors.push_back({"Expected ]", temp->lines});
			goto ErrorHandle;
		}
		if (temp->value == Token::LT)
		{
			if ((++temp)->value == Token::LITERAL_INTEGER)
			{
				size_t init = (temp++)->literal.l;
				for (size_t index = expr->elements.size(); index < init; ++index)
					expr->elements.push_back(new SyntaxLiteral(Token::LITERAL_NULL, temp->lines, Literal()));
			}
			else
			{
				errors.push_back({"Expected number", temp->lines});
				goto ErrorHandle;
			}
			if ((temp++)->value != Token::GT)
				goto ErrorHandle;
		}
		iter = temp;
		return expr;
	ErrorHandle:
		delete expr;
		return nullptr;
	}
	std::pair<SyntaxExpr *, SyntaxExpr *> *ParseKeyVal(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxExpr *key;
		SyntaxExpr *value;
		if (temp->value == Token::IDENTIFIER)
			key = new SyntaxLiteral(TokenDesc{Token::LITERAL_STRING, temp->lines, temp->literal.s}), ++temp;
		else if ((key = ParseLiteral(temp, errors)) == nullptr)
			return nullptr;
		if ((temp++)->value != Token::COLON)
			goto ErrorHandle;
		if ((value = ParseAssign(temp, errors)) == nullptr)
		{
			errors.push_back({"Expected value", temp->lines});
			goto ErrorHandle;
		}
		iter = temp;
		return new std::pair<SyntaxExpr *, SyntaxExpr *>(key, value);
	ErrorHandle:
		delete key;
		return nullptr;
	}
	SyntaxExpr *ParseObject(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxObject *object = new SyntaxObject();
		std::pair<SyntaxExpr *, SyntaxExpr *> *key;
		if ((temp++)->value != Token::LBRACE)
			goto ErrorHandle;
		if ((key = ParseKeyVal(temp, errors)) != nullptr)
		{
			object->elements.push_back(*key);
			while (temp->value == Token::COMMA)
			{
				if ((key = ParseKeyVal(++temp, errors)) == nullptr)
				{
					errors.push_back({"Expected key and value", temp->lines});
					goto ErrorHandle;
				}
				object->elements.push_back(*key);
			}
		}
		if ((temp++)->value != Token::RBRACE)
		{
			errors.push_back({"Expected }", temp->lines});
			goto ErrorHandle;
		}
		iter = temp;
		return object;
	ErrorHandle:
		delete object;
		return nullptr;
	}
	SyntaxExpr *ParseFunction(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxFunction *expr = new SyntaxFunction();
		if ((temp++)->value != Token::FUNCTION)
			goto ErrorHandle;
		if ((temp++)->value != Token::LPAREN)
			goto ErrorHandle;
		if (temp->value == Token::IDENTIFIER)
		{
			expr->params.push_back((temp++)->literal.s);
			while (temp->value == Token::COMMA)
			{
				if ((++temp)->value != Token::IDENTIFIER)
				{
					errors.push_back({"Expected identifier", (--temp)->lines});
					goto ErrorHandle;
				}
				expr->params.push_back((temp++)->literal.s);
			}
		}
		if ((temp++)->value != Token::RPAREN)
		{
			errors.push_back({"Expected )", (--temp)->lines});
			goto ErrorHandle;
		}
		if ((expr->sents = ParseBlock(temp, errors)) == nullptr)
			goto ErrorHandle;
		iter = temp;
		return expr;
	ErrorHandle:
		delete expr;
		return nullptr;
	}
	SyntaxExpr *ParseSentence(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors, size_t args_count...)
	{
		auto temp = iter;
		SyntaxExpr *sent = nullptr;
		va_list args_ptr;
		va_start(args_ptr, args_count);
		for (size_t count = 0; count < args_count; ++count)
			if ((sent = va_arg(args_ptr, SyntaxExpr * (*)(std::vector<TokenDesc>::iterator &, std::vector<Error> &))(temp, errors)) != nullptr)
				break;
		va_end(args_ptr);
		while (temp->value == Token::SEMICOLON)
			++temp;
		iter = temp;
		return sent;
	}
	SyntaxExpr *ParseIf(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxIf *sent = new SyntaxIf();

		if ((temp++)->value != Token::IF)
			goto ErrorHandle;

		if ((temp++)->value != Token::LPAREN)
		{
			errors.push_back({"Expected (", (--temp)->lines});
			goto ErrorHandle;
		}
		if ((sent->cond = ParseExpr(temp, errors)) == nullptr)
		{
			errors.push_back({"Expected expression", (--temp)->lines});
			goto ErrorHandle;
		}
		if ((temp++)->value != Token::RPAREN)
		{
			errors.push_back({"Expected )", (--temp)->lines});
			goto ErrorHandle;
		}
		if ((sent->truesents = ParseBlock(temp, errors)) == nullptr)
			goto ErrorHandle;
		if (temp->value == Token::ELSE && (sent->falsesents = ParseBlock(++temp, errors)) == nullptr)
			goto ErrorHandle;
		iter = temp;
		return sent;
	ErrorHandle:
		delete sent;
		return nullptr;
	}

	SyntaxExpr *ParseLoop(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxLoop *sent = new SyntaxLoop();
		if ((temp++)->value != Token::LOOP)
			goto ErrorHandle;
		if (temp->value == Token::LPAREN)
		{
			sent->prefix_condition = ParseExpr(++temp, errors);
			if (sent->prefix_condition == nullptr)
				sent->prefix_condition = ParseDeclare(temp, errors);
			if (temp->value == Token::SEMICOLON)
			{
				sent->init = sent->prefix_condition;
				sent->prefix_condition = ParseExpr(++temp, errors);
				if ((temp++)->value != Token::SEMICOLON)
				{
					errors.push_back({"Expected semicolon", (--temp)->lines});
					goto ErrorHandle;
				}
				sent->loop = ParseExpr(temp, errors);
			}
			if ((temp++)->value != Token::RPAREN)
			{
				errors.push_back({"Expected )", (--temp)->lines});
				goto ErrorHandle;
			}
		}
		if ((sent->sents = ParseBlock(temp, errors)) == nullptr)
			goto ErrorHandle;
		if (temp->value == Token::LPAREN)
		{
			if (sent->init || sent->prefix_condition || sent->loop)
			{
				errors.push_back({"Can`t overlap with dislocation declaration", temp->lines});
				goto ErrorHandle;
			}
			sent->postfix_condition = ParseExpr(++temp, errors);
			if ((temp++)->value != Token::RPAREN)
			{
				errors.push_back({"Expected )", (--temp)->lines});
				goto ErrorHandle;
			}
		}
		iter = temp;
		return sent;
	ErrorHandle:
		delete sent;
		return nullptr;
	}
	SyntaxExpr *ParseDeclare(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxDeclare *expr = new SyntaxDeclare();
		VarDesc desc;
		desc.option = VarDesc::VAR;
		expr->line = temp->lines;
		switch ((temp++)->value)
		{
		case Token::CONST:
			desc.option = VarDesc::CONST;
		case Token::VAR:
		{
			SyntaxExpr *init = nullptr;
			do
			{
				if (temp->value != Token::IDENTIFIER)
					goto ErrorHandle;
				desc.name = (temp++)->literal.s;
				if (temp->value == Token::ASSIGN)
				{
					if ((init = ParseAssign(++temp, errors)) == nullptr)
					{
						errors.push_back({"Expected expression", temp->lines});
						goto ErrorHandle;
					}
				}
				expr->elements.push_back(std::make_pair(desc, init));
			} while (temp->value == Token::COMMA);
		}
		break;
		case Token::FUNCTION:
		{
			if (temp->value != Token::IDENTIFIER)
				goto ErrorHandle;
			SyntaxFunction *func = new SyntaxFunction();
			expr->elements.push_back(std::make_pair(desc = {(temp++)->literal.s, VarDesc::CONST}, func));
			if ((temp++)->value != Token::LPAREN)
			{
				errors.push_back({"Expected (", (--temp)->lines});
				goto ErrorHandle;
			}
			if (temp->value == Token::IDENTIFIER)
			{
				func->params.push_back((temp++)->literal.s);
				while (temp->value == Token::COMMA)
				{
					if ((++temp)->value != Token::IDENTIFIER)
					{
						errors.push_back({"Expected identifier", (--temp)->lines});
						goto ErrorHandle;
					}
					func->params.push_back((temp++)->literal.s);
				}
			}
			if ((temp++)->value != Token::RPAREN)
			{
				errors.push_back({"Expected )", (--temp)->lines});
				goto ErrorHandle;
			}
			if ((func->sents = ParseBlock(temp, errors)) == nullptr)
				goto ErrorHandle;
		}
		break;
		case Token::STRUCT:
		{
			if (temp->value != Token::IDENTIFIER)
				goto ErrorHandle;
			SyntaxClass *clsinf = new SyntaxClass();
			expr->elements.push_back(std::make_pair(desc = {(temp++)->literal.s, VarDesc::CONST}, clsinf));
			if ((temp++)->value != Token::LBRACE)
			{
				errors.push_back({"Expected {", (--temp)->lines});
				goto ErrorHandle;
			}
			while (SyntaxExpr *expr = ParseDeclare(temp, errors))
			{
				clsinf->member.push_back(expr);
				while (temp->value == Token::SEMICOLON)
					++temp;
			}
			if ((temp++)->value != Token::RBRACE)
			{
				errors.push_back({"Expected }", (--temp)->lines});
				goto ErrorHandle;
			}
		}
		break;
		default:
			goto ErrorHandle;
		}
		iter = temp;
		return expr;
	ErrorHandle:
		delete expr;
		return nullptr;
	}
	SyntaxBlock *ParseBlock(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors)
	{
		auto temp = iter;
		SyntaxBlock *blocks = new SyntaxBlock();
		SyntaxExpr *expr;
		bool check = false;
		if (temp->value == Token::LBRACE)
		{
			check = true;
			++temp;
		}
		do
		{
			if (temp->value == Token::EOT)
			{
				errors.push_back({"Expected }", (--temp)->lines});
				goto ErrorHandle;
			}
			if ((expr = ParseGeneralSentences(temp, errors)) == nullptr)
				goto ErrorHandle;
			blocks->sents.push_back(expr);
		} while (check && temp->value != Token::RBRACE);
		if (temp->value == Token::RBRACE)
			++temp;
		iter = temp;
		return blocks;
	ErrorHandle:
		delete blocks;
		return nullptr;
	}

	bool SyntaxTree::ParseText(SyntaxTree &code, const std::string &str)
	{
		std::vector<TokenDesc> result;
		if (!Scanner::Tokenize(str.c_str(), result))
			return false;

		// for (TokenDesc &token : result)
		// 	printf("[%s]\n", token.GetInfo());

		auto iter = result.begin();
		while (iter->value != Token::EOT)
		{
			SyntaxExpr *sent;
			if ((sent = ParseGeneralSentences(iter, code.errors)) == nullptr)
				return false;
			code.sents.push_back(sent);
		}

		return true;
	}
}