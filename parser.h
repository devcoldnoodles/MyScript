#ifndef __PARSER_H__
#define __PARSER_H__

#include <cstdarg>
#include <limits>

#include "data.h"
#include "scanner.h"

namespace myscript
{
	static float tf1 = 1;
	static short *float1 = (short *)&tf1;

	struct SyntaxNode
	{
		size_t line;
		virtual std::string GetType() { return "empty"; }
		virtual bool CreateCode(CompliationDesc *cd) { return false; }
	};

	struct SyntaxExpr : SyntaxNode
	{
		virtual bool CreateLCode(CompliationDesc *cd) { return false; }
		virtual bool CreateRCode(CompliationDesc *cd) { return false; }
	};

	struct SyntaxLiteral : SyntaxExpr
	{
		short value;
		Literal literal;
		SyntaxLiteral(TokenDesc &desc) : value(desc.value), literal(desc.literal) { line = desc.lines; }
		SyntaxLiteral(TokenDesc &&desc) : value(desc.value), literal(desc.literal) { line = desc.lines; }
		SyntaxLiteral(short _value, size_t _line, Literal _literal) : value(_value), literal(_literal) { line = _line; }
		~SyntaxLiteral() { if (value == Token::LITERAL) delete literal.s;}
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			switch (value)
			{
			case Token::LITERAL_NULL:
				cd->code.push_back(OpCode::PUSHNULL);
				break;
			case Token::LITERAL_TRUE:
				cd->code.push_back(OpCode::PUSHTRUE);
				break;
			case Token::LITERAL_FALSE:
				cd->code.push_back(OpCode::PUSHFALSE);
				break;
			case Token::LITERAL_FLOAT:
			{
				double value = literal.d;
				if (value <= std::numeric_limits<float>::max())
				{
					float temp = value;
					short *fvalue = (short *)&temp;
					cd->code.push_back(OpCode::PUSHDWORD);
					cd->code.push_back(fvalue[0]);
					cd->code.push_back(fvalue[1]);
				}
				else
				{
					short *dvalue = (short *)&value;
					cd->code.push_back(OpCode::PUSHQWORD);
					cd->code.push_back(dvalue[0]);
					cd->code.push_back(dvalue[1]);
					cd->code.push_back(dvalue[2]);
					cd->code.push_back(dvalue[3]);
				}
			}
			break;
			case Token::LITERAL_INTEGER:
			{
				double value = literal.l;
				if (value <= std::numeric_limits<float>::max())
				{
					float temp = value;
					short *fvalue = (short *)&temp;
					cd->code.push_back(OpCode::PUSHDWORD);
					cd->code.push_back(fvalue[0]);
					cd->code.push_back(fvalue[1]);
				}
				else
				{
					short *dvalue = (short *)&value;
					cd->code.push_back(OpCode::PUSHQWORD);
					cd->code.push_back(dvalue[0]);
					cd->code.push_back(dvalue[1]);
					cd->code.push_back(dvalue[2]);
					cd->code.push_back(dvalue[3]);
				}
			}
			break;
			case Token::IDENTIFIER:
			case Token::LITERAL_STRING:
			{
				std::string str(literal.s);
				size_t str_size = str.size();
				size_t index = 0;
				cd->code.push_back(PUSHSTR);
				cd->code.push_back(str_size);
				while (index + 2 <= str_size)
					cd->code.push_back(str[index + 1] << 8 | str[index]), index += 2;
				if (index == str_size)
					cd->code.push_back(0);
				else
					cd->code.push_back(str[index]);
			}
			break;
			default:
				cd->errors.push_back({"Invalid literal type", line});
				return false;
			}
			return true;
		}
	};

	struct SyntaxIdentifier : SyntaxExpr
	{
		std::string id;

		SyntaxIdentifier(const TokenDesc &_desc) : id(_desc.literal.s) { line = _desc.lines; }
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			uint16_t value = cd->Identify(id);
			if (value == 0xFFFF)
			{
				size_t findex = -1;
				size_t ids_size = cd->global.size();
				for (size_t index = 0; index < ids_size; ++index)
				{
					if (cd->global[index].name == id)
					{
						findex = index;
						break;
					}
				}
				if (findex == -1)
				{
					cd->errors.push_back({"Undefined identifier " + id, line});
					return false;
				}
				cd->code.push_back(PUSH);
				cd->code.push_back(findex);
				return true;
			}
			cd->code.push_back(READ);
			cd->code.push_back(value);
			return true;
		}
		bool CreateLCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			uint16_t value = cd->Identify(id);
			if (value == 0xFFFF)
			{
				size_t findex = -1;
				size_t ids_size = cd->global.size();
				for (size_t index = 0; index < ids_size; ++index)
				{
					if (cd->global[index].name == id)
					{
						findex = index;
						break;
					}
				}
				if (findex == -1)
				{
					cd->errors.push_back({"Undefined identifier " + id, line});
					return false;
				}
				if (cd->global[findex].option & VarDesc::CONST)
				{
					cd->errors.push_back({"'" + id + "' is constants value", line});
					return false;
				}
				cd->code.push_back(STORE);
				cd->code.push_back(findex);
				return true;
			}
			if (cd->GetScopeVariable(value)->option & VarDesc::CONST)
			{
				cd->errors.push_back({"'" + id + "' is constants value", line});
				return false;
			}
			cd->code.push_back(WRITE);
			cd->code.push_back(value);
			return true;
		}
	};

	struct SyntaxSelf : SyntaxExpr
	{
		SyntaxSelf(size_t _line) { line = _line; }
		std::string GetType() const { return "self"; }
	};

	struct SyntaxChain : SyntaxExpr
	{
		SyntaxExpr *lexpr;
		SyntaxExpr *rexpr;
		SyntaxChain(SyntaxExpr *_lexpr, SyntaxExpr *_rexpr) : lexpr(_lexpr), rexpr(_rexpr) { line = lexpr->line; }
		~SyntaxChain() { delete lexpr, rexpr; }

		bool CreateCode(CompliationDesc *cd)
		{
			return CreateRCode(cd);
		}

		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			return rexpr->CreateRCode(cd) && lexpr->CreateRCode(cd);
		}
	};

	struct SyntaxNot : SyntaxExpr // !
	{
		SyntaxExpr *expr;
		SyntaxNot(SyntaxExpr *_expr) : expr(_expr) { line = _expr->line; }
		~SyntaxNot() { delete expr; }

		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(NOT);
			return true;
		}
	};

	struct SyntaxUnaryOperator : SyntaxExpr
	{
		SyntaxExpr *expr;
		OpCode opcode;
		SyntaxUnaryOperator(SyntaxExpr *_expr, OpCode _opcode) : expr(_expr), opcode(_opcode) {}
		~SyntaxUnaryOperator() { delete expr; }

		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!expr->CreateRCode(cd))
				return false;
			if (opcode != OpCode::NONE)
				cd->code.push_back(opcode);
			return true;
		}
	};

	struct SyntaxBinaryOperator : SyntaxExpr
	{
		SyntaxExpr *lexpr;
		SyntaxExpr *rexpr;
		OpCode opcode;
		SyntaxBinaryOperator(SyntaxExpr *_lexpr, SyntaxExpr *_rexpr, OpCode _opcode) : lexpr(_lexpr), rexpr(_rexpr), opcode(_opcode) {}
		~SyntaxBinaryOperator()	{delete lexpr; delete rexpr;}

		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			if (opcode != OpCode::NONE)
				cd->code.push_back(opcode);
			return true;
		}
	};

	struct SyntaxTernaryOperator : SyntaxExpr
	{
		SyntaxExpr *lexpr;
		SyntaxExpr *expr;
		SyntaxExpr *rexpr;
		SyntaxTernaryOperator(SyntaxExpr *_lexpr, SyntaxExpr *_expr, SyntaxExpr *_rexpr) : lexpr(_lexpr), expr(_expr), rexpr(_rexpr) {}
		~SyntaxTernaryOperator() { delete lexpr; delete expr; delete rexpr; }

		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!lexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(CFJMP);
			cd->code.push_back(NONE);
			size_t tdelta = cd->code.size();
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(FJMP);
			cd->code.push_back(NONE);
			cd->code[tdelta - 1] = cd->code.size() - tdelta;
			size_t fdelta = cd->code.size();
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code[fdelta - 1] = cd->code.size() - fdelta;
			return true;
		}
	};

	struct SyntaxAssign : SyntaxExpr
	{
		SyntaxExpr *lexpr;
		SyntaxExpr *rexpr;
		SyntaxAssign(SyntaxExpr *_lexpr, SyntaxExpr *_rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxAssign() { delete lexpr, rexpr; }

		std::string GetType() const { return "assign"; }
		bool CreateCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			if (!lexpr->CreateLCode(cd))
				return false;
			cd->code.push_back(POP);
			return true;
		}
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			if (!lexpr->CreateLCode(cd))
				return false;
			return true;
		}
	};
	
	struct SyntaxPrefixPlusx2 : SyntaxExpr // ++expr
	{
		SyntaxExpr *expr;
		SyntaxPrefixPlusx2(SyntaxExpr *_expr) : expr(_expr) {}
		~SyntaxPrefixPlusx2() { delete expr; }

		std::string GetType() const { return "prefix plusx2"; }
		bool CreateCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(ADD);
			if (!expr->CreateLCode(cd))
				return false;
			cd->code.push_back(POP);
			return true;
		}
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(ADD);
			if (!expr->CreateLCode(cd))
				return false;
			return true;
		}
	};

	struct SyntaxSuffixPlusx2 : SyntaxExpr // expr++
	{
		SyntaxExpr *expr;
		SyntaxSuffixPlusx2(SyntaxExpr *_expr) : expr(_expr) {}
		~SyntaxSuffixPlusx2() { delete expr; }

		std::string GetType() const { return "suffix plusx2"; }
		bool CreateCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(ADD);
			if (!expr->CreateLCode(cd))
				return false;
			cd->code.push_back(POP);
			return true;
		}
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(ADD);
			if (!expr->CreateLCode(cd))
				return false;
			cd->code.push_back(PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(SUB);
			return true;
		}
	};

	struct SyntaxPrefixMinusx2 : SyntaxExpr // --
	{
		SyntaxExpr *expr;
		SyntaxPrefixMinusx2(SyntaxExpr *_expr) : expr(_expr) {}
		~SyntaxPrefixMinusx2() { delete expr; }

		std::string GetType() const { return "minusx2"; }
		bool CreateCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(SUB);
			if (!expr->CreateLCode(cd))
				return false;

			cd->code.push_back(POP);
			return true;
		}
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(SUB);
			if (!expr->CreateLCode(cd))
				return false;
			return true;
		}
	};

	struct SyntaxSuffixMinusx2 : SyntaxExpr // --
	{
		SyntaxExpr *expr;
		SyntaxSuffixMinusx2(SyntaxExpr *_expr) : expr(_expr) {}
		~SyntaxSuffixMinusx2() { delete expr; }

		std::string GetType() const { return "minusx2"; }
		bool CreateCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(SUB);
			if (!expr->CreateLCode(cd))
				return false;
			cd->code.push_back(POP);
			return true;
		}
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(SUB);
			if (!expr->CreateLCode(cd))
				return false;
			cd->code.push_back(PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(ADD);
			return true;
		}
	};

	struct SyntaxNew : SyntaxExpr // new
	{
		SyntaxExpr *expr;
		SyntaxNew(SyntaxExpr *_expr) : expr(_expr) {}
		~SyntaxNew() { delete expr; }

		std::string GetType() const { return "new"; }
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.back() = NEW;
			return true;
		}
	};

	struct SyntaxAs : SyntaxExpr // as
	{
		SyntaxExpr *lexpr;
		SyntaxExpr *rexpr;
		SyntaxAs(SyntaxExpr *_lexpr, SyntaxExpr *_rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxAs() { delete lexpr, rexpr; }

		std::string GetType() const { return "as"; }
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(AS);
			return true;
		}
	};

	struct SyntaxIs : SyntaxExpr // is
	{
		SyntaxExpr *lexpr;
		SyntaxExpr *rexpr;
		SyntaxIs(SyntaxExpr *_lexpr, SyntaxExpr *_rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxIs() { delete lexpr, rexpr; }

		std::string GetType() const { return "is"; }
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(IS);
			return true;
		}
	};

	struct SyntaxAccess : SyntaxExpr // object.member
	{
		SyntaxExpr *lexpr;
		SyntaxExpr *rexpr;
		SyntaxAccess(SyntaxExpr *_lexpr, SyntaxExpr *_rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxAccess() { delete lexpr; }

		std::string GetType() const { return "access"; }
		bool CreateLCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			if (!lexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(REFSET);
			return true;
		}
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			if (!lexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(REFGET);
			return true;
		}
	};

	struct SyntaxRef : SyntaxExpr // arr[0]
	{
		SyntaxExpr *lexpr;
		SyntaxExpr *rexpr;
		SyntaxRef(SyntaxExpr *_lexpr, SyntaxExpr *_rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxRef() { delete lexpr, rexpr; }

		std::string GetType() const { return "ref"; }
		bool CreateLCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			if (!lexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(ARRSET);
			return true;
		}
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			if (!lexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(ARRGET);
			return true;
		}
	};

	struct SyntaxCall : SyntaxExpr
	{
		SyntaxExpr *expr;
		std::vector<SyntaxExpr *> params;
		SyntaxCall(SyntaxExpr *_func) : expr(_func) {}
		~SyntaxCall()
		{
			delete expr;
			for (auto param : params)
				delete param;
		}

		std::string GetType() const { return "call"; }
		bool CreateCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!expr->CreateRCode(cd))
				return false;
			size_t param_size = params.size();
			for (size_t index = 0; index < param_size; ++index)
				if (!params[index]->CreateRCode(cd))
					return false;
			cd->code.push_back(CALL);
			cd->code.push_back(param_size);
			cd->code.push_back(POP);
			return true;
		}
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!expr->CreateRCode(cd))
				return false;
			size_t param_size = params.size();
			for (size_t index = 0; index < param_size; ++index)
				if (!params[index]->CreateRCode(cd))
					return false;
			cd->code.push_back(CALL);
			cd->code.push_back(param_size);
			return true;
		}
	};

	struct SyntaxArray : SyntaxExpr
	{
		std::vector<SyntaxExpr *> elements;
		SyntaxArray() {}
		~SyntaxArray()
		{
			for (auto element : elements)
				delete element;
		}

		std::string GetType() const { return "array"; }
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			size_t elements_size = elements.size();
			for (size_t index = 0; index < elements_size; ++index)
				if (!elements[elements_size - index - 1]->CreateRCode(cd))
					return false;

			cd->code.push_back(INSTARR);
			cd->code.push_back(elements_size);
			return true;
		}
	};

	struct SyntaxObject : SyntaxExpr
	{
		std::map<std::string, SyntaxExpr *> elements2;
		std::vector<std::pair<SyntaxExpr *, SyntaxExpr *>> elements;

		std::string GetType() const { return "object"; }

		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			cd->scope.push_back(LocalScope());	
			size_t elements_size = elements.size();
			for (size_t index = 0; index < elements_size; ++index)
			{
				if (!elements[elements_size - index - 1].second->CreateRCode(cd))
					return false;

				if (!elements[elements_size - index - 1].first->CreateRCode(cd))
					return false;
			}

			cd->code.push_back(INSTOBJ);
			cd->code.push_back(elements_size);
			return true;
		}
	};

	struct SyntaxDeclare : SyntaxExpr
	{
		std::vector<std::pair<VarDesc, SyntaxExpr *>> elements;

		~SyntaxDeclare()
		{
			for (auto element : elements)
				delete element.second;
		}
		std::string GetType() const { return "declare"; }
		bool CreateCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			for (auto element : elements)
			{
				if (cd->scope.empty())
				{
					size_t global_size = cd->global.size();
					for (size_t index = 0; index < global_size; ++index)
					{
						if (cd->global[index].name == element.first.name)
						{
							cd->errors.push_back({"Already declared " + element.first.name, line});
							return false;
						}
					}
					cd->global.push_back({element.first});
					if (element.second != nullptr)
					{
						if (!element.second->CreateRCode(cd))
							return false;
						cd->code.push_back(STORE);
						cd->code.push_back(global_size);
						cd->code.push_back(POP);
					}
				}
				else
				{
					uint16_t index = cd->Identify(element.first.name);
					if (index == 0xFFFF)
						cd->scope.back().variables.push_back({element.first});
					if (element.second)
					{
						if (!element.second->CreateRCode(cd))
							return false;
					}
					else
						cd->code.push_back(PUSHNULL);
					if (index != 0xFFFF)
						cd->code.push_back(WRITE), cd->code.push_back(index);
				}
			}

			return true;
		}
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			for (auto element : elements)
			{
				if (cd->scope.empty())
				{
					cd->errors.push_back({"Incorrect right expr declare must in scope range", line});
					return false;
				}
				uint16_t index = cd->Identify(element.first.name);
				if (index == 0xFFFF)
					cd->scope.back().variables.push_back({element.first});
				if (element.second)
				{
					if (!element.second->CreateRCode(cd))
						return false;
				}
				else
					cd->code.push_back(PUSHNULL);
				if (index != 0xFFFF)
				{
					cd->code.push_back(WRITE);
					cd->code.push_back(index);
				}
				cd->code.push_back(READ);
				cd->code.push_back(cd->scope.back().variables.size() - 1);
			}
			
			return true;
		}
	};
	struct SyntaxBlock : SyntaxNode
	{
		std::vector<SyntaxNode *> sents;
		virtual ~SyntaxBlock()
		{
			for (auto sent : sents)
				delete sent;
		}

		bool CreateCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			cd->scope.push_back(LocalScope());
			size_t sents_size = sents.size();
			for (size_t index = 0; index < sents_size; ++index)
				if (sents[index]->CreateCode(cd) == false)
					return false;
			size_t list_size = cd->scope.back().variables.size();
			if (list_size > 0)
			{
				cd->code.push_back(POPTO);
				cd->code.push_back(list_size);
			}
			auto temp = cd->scope.back();
			cd->scope.pop_back();
			for (auto point : temp.startpoint)
				cd->scope.back().startpoint.push_back(point);
			for (auto point : temp.endpoint)
				cd->scope.back().endpoint.push_back(point);
			return true;
		}
	};
	struct SyntaxTree : SyntaxBlock
	{
		std::vector<Error> errors;
		static bool ParseText(SyntaxTree &code, const std::string &str);
		bool CreateCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			// for (auto sent = sents.begin(); sent != sents.end(); ++sent)
			// {
			// 	auto type = (*sent)->GetType();
			// 	if(type == "declare" || type == "function" || type == "class")
			// 		if ((*sent)->CreateCode(cd) == false)
			// 			return false;
			// 		sents.erase(sent);
			// }
			size_t sents_size = sents.size();
			for (size_t index = 0; index < sents_size; ++index)
				if (sents[index]->CreateCode(cd) == false)
					return false;
			if (cd->code.empty())
				cd->code.push_back(RETURN);
			else if (cd->code.back() != RETURN)
				cd->code.push_back(RETURN);
			return true;
		}
	};
	struct SyntaxFunction : SyntaxExpr
	{
		std::vector<std::string> params;
		SyntaxBlock *sents;
		~SyntaxFunction()
		{
			if (sents)
				delete sents;
		}

		std::string GetType() const { return "function"; }
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			cd->scope.push_back(LocalScope());
			size_t param_size = params.size();
			for (size_t index = 0; index < param_size; ++index)
				cd->scope.back().variables.push_back({params[index], VarDesc::VAR});
			cd->code.push_back(PUSHFUNC);
			cd->code.push_back(NONE);
			size_t delta = cd->code.size();
			if (param_size > 0)
			{
				cd->code.push_back(PARAMSET);
				cd->code.push_back(param_size);
			}
			size_t sents_size = sents->sents.size();
			for (size_t index = 0; index < sents_size; ++index)
				if (!sents->sents[index]->CreateCode(cd))
					return false;
			if (cd->code.back() != RETURN)
				cd->code.push_back(RETURNNULL);
			cd->code[delta - 1] = cd->code.size() - delta;
			cd->scope.pop_back();
			return true;
		}
	};
	struct SyntaxSingleSentence : SyntaxExpr
	{
		SyntaxExpr *expr;
		SyntaxSingleSentence(SyntaxExpr *_expr) : expr(_expr) {}
		~SyntaxSingleSentence() { delete expr; }

		bool CreateCode(CompliationDesc *cd)
		{
			if (!cd || !expr)
				return false;

			return expr->CreateCode(cd);
		}
	};
	struct SyntaxIf : SyntaxExpr
	{
		SyntaxExpr *cond;
		SyntaxBlock *truesents;
		SyntaxBlock *falsesents;
		~SyntaxIf()
		{
			delete cond;
			delete truesents;
			if (falsesents)
				delete falsesents;
		}

		std::string GetType() const { return "if"; }
		bool CreateCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (!cond->CreateRCode(cd))
				return false;
			cd->code.push_back(CFJMP);
			cd->code.push_back(NONE);
			size_t tdelta = cd->code.size();
			if (!truesents->CreateCode(cd))
				return false;
			if (falsesents != nullptr)
			{
				cd->code.push_back(FJMP);
				cd->code.push_back(NONE);
				cd->code[tdelta - 1] = cd->code.size() - tdelta;
				size_t fdelta = cd->code.size();
				if (!falsesents->CreateCode(cd))
					return false;
				cd->code[fdelta - 1] = cd->code.size() - fdelta;
			}
			else
				cd->code[tdelta - 1] = cd->code.size() - tdelta;
			return true;
		}
	};
	struct SyntaxLoop : SyntaxExpr
	{
		SyntaxExpr *init;
		SyntaxExpr *prefix_condition;
		SyntaxExpr *suffix_condition;
		SyntaxExpr *loop;
		SyntaxBlock *sents;
		~SyntaxLoop()
		{
			if (init)
				delete init;
			if (prefix_condition)
				delete prefix_condition;
			if (suffix_condition)
				delete suffix_condition;
			if (loop)
				delete loop;
		}

		std::string GetType() const { return "loop"; }
		bool CreateCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			cd->scope.push_back(LocalScope());
			if (init != nullptr)
			{
				if (!init->CreateCode(cd))
					return false;
			}
			cd->code.push_back(FJMP);
			cd->code.push_back(NONE);
			size_t delta = cd->code.size();
			if (loop != nullptr)
			{
				if (!loop->CreateCode(cd))
					return false;
				cd->code[delta - 1] = cd->code.size() - delta;
			}
			if (prefix_condition != nullptr)
			{
				if (!prefix_condition->CreateRCode(cd))
					return false;
				cd->code.push_back(CFJMP);
				cd->scope.back().endpoint.push_back(cd->code.size());
				cd->code.push_back(NONE);
			}
			if (suffix_condition != nullptr)
			{
				if (!suffix_condition->CreateRCode(cd))
					return false;
				cd->code.push_back(CFJMP);
				cd->scope.back().endpoint.push_back(cd->code.size());
				cd->code.push_back(NONE);
				cd->code[delta - 1] = cd->code.size() - delta;
			}
			if (!sents->CreateCode(cd))
				return false;
			cd->code.push_back(BJMP);
			cd->scope.back().startpoint.push_back(cd->code.size());
			cd->code.push_back(NONE);
			for (size_t index : cd->scope.back().startpoint)
			{
				cd->code[index] = index - delta + 1;
			}
			for (size_t index : cd->scope.back().endpoint)
			{
				cd->code[index] = cd->code.size() - index - 1;
			}
			size_t list_size = cd->scope.back().variables.size();
			if (list_size > 0)
			{
				cd->code.push_back(POPTO);
				cd->code.push_back(list_size);
			}
			cd->scope.pop_back();
			return true;
		}
	};
	struct SyntaxReturn : SyntaxExpr
	{
		SyntaxExpr *value;
		SyntaxReturn(SyntaxExpr *_value) : value(_value) {}
		~SyntaxReturn() { delete value; }
		std::string GetType() const { return "return"; }
		bool CreateCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (cd->scope.empty())
			{
				cd->errors.push_back({"Incorrect scope", line});
				return false;
			}
			if (value)
			{
				if (!value->CreateRCode(cd))
					return false;
				cd->code.push_back(RETURN);
			}
			else
				cd->code.push_back(RETURNNULL);
			return true;
		}
	};
	struct SyntaxContinue : SyntaxExpr
	{
		SyntaxContinue(size_t _line) { line = _line; }
		std::string GetType() const { return "continue"; }
		bool CreateCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (cd->scope.empty())
			{
				cd->errors.push_back({"Incorrect scope", line});
				return false;
			}
			cd->code.push_back(BJMP);
			cd->scope.back().startpoint.push_back(cd->code.size());
			cd->code.push_back(NONE);
			return true;
		}
	};
	struct SyntaxBreak : SyntaxExpr
	{
		SyntaxBreak(size_t _line) { line = _line; }
		std::string GetType() const { return "break"; }
		bool CreateCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			if (cd->scope.empty())
			{
				cd->errors.push_back({"Incorrect scope", line});
				return false;
			}
			cd->code.push_back(FJMP);
			cd->scope.back().endpoint.push_back(cd->code.size());
			cd->code.push_back(NONE);
			return true;
		}
	};
	struct SyntaxClass : SyntaxExpr
	{
		std::vector<SyntaxExpr *> member;

		std::string GetType() const { return "class"; }
		bool CreateRCode(CompliationDesc *cd)
		{
			if (!cd)
				return false;
			auto delta = cd->code.size();
			cd->scope.push_back(LocalScope());
			for (auto iter : member)
				iter->CreateCode(cd);
			cd->code.push_back(DEF);
			cd->code.push_back(cd->code.size() - delta);
			return true;
		}
	};

	SyntaxExpr *ParseExpr(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseTernaryOperator(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseAssign(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseLogicalOR(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseLogicalAND(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseCompare(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseAdd(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseMul(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParsePrefix(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseSuffix(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseElement(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseLiteral(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseSimpleLiteral(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseArray(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	std::pair<SyntaxExpr *, SyntaxExpr *> *ParseKeyVal(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseObject(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseFunction(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseSentence(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseSingleSentence(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseIf(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseLoop(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxExpr *ParseDeclare(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
	SyntaxBlock *ParseBlock(std::vector<TokenDesc>::iterator &iter, std::vector<Error> &errors);
};
#endif