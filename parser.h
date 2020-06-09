#ifndef __PARSER_H__
#define __PARSER_H__
#include "data.h"

namespace myscript 
{
	static short* float1 = (short*)new float(1);
	
	struct SyntaxNode
	{
		size_t line;
		virtual std::string GetType() { return "empty"; }
		virtual bool CreateCode(CompliationDesc* cd) { return false; }
	};
	struct SyntaxExpr : SyntaxNode
	{
		virtual bool CreateLCode(CompliationDesc* cd) { return false; }
		virtual bool CreateRCode(CompliationDesc* cd) { return false; }
	};
	struct SyntaxLiteral : SyntaxExpr
	{
		Token data;
		SyntaxLiteral(const Token _data) : data(_data) { line = _data.line; }
		bool CreateRCode(CompliationDesc* cd)
		{
			switch (data.type)
			{
			case Token::NULLPTR:
				cd->code.push_back(OpCode::PUSHNULL);
				break;
			case Token::TRUE:
				cd->code.push_back(OpCode::PUSHTRUE);
				break;
			case Token::FALSE:
				cd->code.push_back(OpCode::PUSHFALSE);
				break;
			case Token::NUMBER:
			{
				double value = stod(data.str);
				if (value <= std::numeric_limits<float>::max())
				{
					short* fvalue = (short*)new float(value);
					cd->code.push_back(OpCode::PUSHDWORD);
					cd->code.push_back(fvalue[0]);
					cd->code.push_back(fvalue[1]);
					delete fvalue;
				}
				else
				{
					short* dvalue = (short*)new double(value);
					cd->code.push_back(OpCode::PUSHQWORD);
					cd->code.push_back(dvalue[0]);
					cd->code.push_back(dvalue[1]);
					cd->code.push_back(dvalue[2]);
					cd->code.push_back(dvalue[3]);
					delete dvalue;
				}
			}
			break;
			case Token::STRING:
			{
				std::string& str = data.str;
				size_t str_size = str.size();
				size_t index = 0;
				cd->code.push_back(OpCode::PUSHSTR);
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
		
		SyntaxIdentifier(const Token& _token) : id(_token.str) { line = _token.line; }
		bool CreateRCode(CompliationDesc* cd)
		{
			uint16_t value = cd->Identify(id);
			if (value == 0xFFFF)
			{
				size_t findex = -1;
				size_t ids_size = cd->global.size();
				for (size_t index = 0; index < ids_size; ++index)
					if (cd->global[index].name == id)
					{
						findex = index;
						break;
					}
				if (findex == -1)
				{
					cd->errors.push_back({"Undefined identifier " + id, line});
					return false;
				}
				cd->code.push_back(OpCode::PUSH);
				cd->code.push_back(findex);
				return true;
			}
			cd->code.push_back(OpCode::READ);
			cd->code.push_back(value);
			return true;
		}
		bool CreateLCode(CompliationDesc* cd)
		{
			uint16_t value = cd->Identify(id);
			if (value == 0xFFFF)
			{
				size_t findex = -1;
				size_t ids_size = cd->global.size();
				for (size_t index = 0; index < ids_size; ++index)
					if (cd->global[index].name == id)
					{
						findex = index;
						break;
					}
				if (findex == -1)
				{
					cd->errors.push_back({"Undefined identifier " + id, line});
					return false;
				}
				if (cd->global[findex].option & VarDesc::CONST)
				{
					cd->errors.push_back({"'" + id + "' is  value", line});
					return false;
				}
				cd->code.push_back(OpCode::STORE);
				cd->code.push_back(findex);
				return true;
			}
			if( cd->GetScopeVariable(value)->option & VarDesc::CONST)
			{
				cd->errors.push_back({"'" + id + "' is constants value", line});
				return false;
			}
			cd->code.push_back(OpCode::WRITE);
			cd->code.push_back(value);
			return true;
		}
	};
	struct SyntaxSelf : SyntaxExpr
	{
		SyntaxSelf(size_t _line) { line = _line; }
		std::string GetType() const { return "self"; }
	};
	struct SyntaxComma : SyntaxExpr // ,
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxComma(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) { line = lexpr->line; }
		~SyntaxComma() { delete lexpr;	delete rexpr; }
		
		bool CreateCode(CompliationDesc* cd)
		{
			return CreateRCode(cd);
		}
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!rexpr->CreateRCode(cd))
				return false;
			if (!lexpr->CreateRCode(cd))
				return false;
			return true;
		}
	};
	struct SyntaxNot : SyntaxExpr // !
	{
		SyntaxExpr* expr;
		SyntaxNot(SyntaxExpr* _expr) : expr(_expr) { line = _expr->line; }
		~SyntaxNot() { delete expr; }
		
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::NOT);
			return true;
		}
	};
	struct SyntaxOr : SyntaxExpr // ||
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxOr(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) { line = lexpr->line; }
		~SyntaxOr() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::OR);
			return true;
		}
	};
	struct SyntaxAnd : SyntaxExpr // &&
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxAnd(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxAnd() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::AND);
			return true;
		}
	};
	struct SyntaxXor : SyntaxExpr // ^
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxXor(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxXor() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::XOR);
			return true;
		}
	};
	struct SyntaxEqual : SyntaxExpr // ==
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxEqual(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxEqual() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::EQ);
			return true;
		}
	};
	struct SyntaxNotEqual : SyntaxExpr // !=
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxNotEqual(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxNotEqual() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::NEQ);
			return true;
		}
	};
	struct SyntaxGreatThan : SyntaxExpr // >
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxGreatThan(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxGreatThan() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::GT);
			return true;
		}
	};
	struct SyntaxGreatEqual : SyntaxExpr // >=
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxGreatEqual(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxGreatEqual() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::GE);
			return true;
		}
	};
	struct SyntaxLessThan : SyntaxExpr // <
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxLessThan(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxLessThan() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::LT);
			return true;
		}
	};
	struct SyntaxLessEqual : SyntaxExpr // <=
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxLessEqual(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxLessEqual() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::LE);
			return true;
		}
	};
	struct SyntaxUnaryOperator : SyntaxExpr
	{
		SyntaxExpr* expr;
		OpCode operation;
		~SyntaxUnaryOperator() { if(expr) delete expr; }

		bool CreateRCode(CompliationDesc* cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(operation);
			return true;
		}
	};
	struct SyntaxBinaryOperator : SyntaxExpr
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		OpCode operation;
		~SyntaxBinaryOperator() { if(lexpr) delete lexpr; if(rexpr) delete rexpr; }

		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(operation);
			return true;
		}
	};
	struct SyntaxTernaryOperator : SyntaxExpr
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* expr;
		SyntaxExpr* rexpr;
		OpCode operation;
		SyntaxTernaryOperator() {}
		SyntaxTernaryOperator(SyntaxExpr* _lexpr, SyntaxExpr* _expr, SyntaxExpr* _rexpr) : lexpr(_lexpr), expr(_expr), rexpr(_rexpr) {}
		~SyntaxTernaryOperator() {if(lexpr) delete lexpr; if(expr) delete expr; if(rexpr) delete rexpr; }

		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::CFJMP);
			cd->code.push_back(OpCode::NONE);
			size_t tdelta = cd->code.size();
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::FJMP);
			cd->code.push_back(OpCode::NONE);
			cd->code[tdelta - 1] = cd->code.size() - tdelta;
			size_t fdelta = cd->code.size();
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code[fdelta - 1] = cd->code.size() - fdelta;
			return true;
		}
	};
	struct SyntaxAdd : SyntaxExpr // +
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxAdd(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxAdd() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::ADD);
			return true;
		}
	};
	struct SyntaxSub : SyntaxExpr // -
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxSub(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxSub() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::SUB);
			return true;
		}
	};
	struct SyntaxMul : SyntaxExpr // *
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxMul(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxMul() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::MUL);
			return true;
		}
	};
	struct SyntaxDiv : SyntaxExpr // /
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxDiv(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxDiv() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::DIV);
			return true;
		}
	};
	struct SyntaxMod : SyntaxExpr // %
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxMod(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxMod() { delete lexpr, rexpr; }
		
		std::string GetType() const { return "mod"; }
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::MOD);
			return true;
		}
	};
	struct SyntaxPow : SyntaxExpr // **
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxPow(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxPow() { delete lexpr, rexpr; }
		
		std::string GetType() const { return "pow"; }
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::POW);
			return true;
		}
	};
	struct SyntaxAssign : SyntaxExpr
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxAssign(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxAssign() { delete lexpr, rexpr; }
		
		std::string GetType() const { return "assign"; }
		bool CreateCode(CompliationDesc* cd)
		{
			if (!rexpr->CreateRCode(cd))
				return false;
			if (!lexpr->CreateLCode(cd))
				return false;
			cd->code.push_back(OpCode::POP);
			return true;
		}
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!rexpr->CreateRCode(cd))
				return false;
			if (!lexpr->CreateLCode(cd))
				return false;
			return true;
		}
	};
	struct SyntaxPrefixPlus : SyntaxExpr // +
	{
		SyntaxExpr* expr;
		SyntaxPrefixPlus(SyntaxExpr* _expr) : expr(_expr) {}
		~SyntaxPrefixPlus() { delete expr; }
		
		std::string GetType() const { return "prefix plus"; }
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			return true;
		}
	};
	struct SyntaxPrefixMinus : SyntaxExpr // -
	{
		SyntaxExpr* expr;
		SyntaxPrefixMinus(SyntaxExpr* _expr) : expr(_expr) {}
		~SyntaxPrefixMinus() { delete expr; }
		
		std::string GetType() const { return "prefix minus"; }
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::SIGN);
			return true;
		}
	};
	struct SyntaxPrefixPlusx2 : SyntaxExpr // ++
	{
		SyntaxExpr* expr;
		SyntaxPrefixPlusx2(SyntaxExpr* _expr) : expr(_expr) {}
		~SyntaxPrefixPlusx2() { delete expr; }
		
		std::string GetType() const { return "prefix plusx2"; }
		bool CreateCode(CompliationDesc* cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(OpCode::ADD);
			if (!expr->CreateLCode(cd))
				return false;
			cd->code.push_back(OpCode::POP);
			return true;
		}
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(OpCode::ADD);
			if (!expr->CreateLCode(cd))
				return false;
			return true;
		}
	};
	struct SyntaxPostfixPlusx2 : SyntaxExpr // ++
	{
		SyntaxExpr* expr;
		SyntaxPostfixPlusx2(SyntaxExpr* _expr) : expr(_expr) {}
		~SyntaxPostfixPlusx2() { delete expr; }
		
		std::string GetType() const { return "postfix plusx2"; }
		bool CreateCode(CompliationDesc* cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(OpCode::ADD);
			if (!expr->CreateLCode(cd))
				return false;
			cd->code.push_back(OpCode::POP);
			return true;
		}
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(OpCode::ADD);
			if (!expr->CreateLCode(cd))
				return false;
			cd->code.push_back(OpCode::PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(OpCode::SUB);
			return true;
		}
	};
	struct SyntaxPrefixMinusx2 : SyntaxExpr // --
	{
		SyntaxExpr* expr;
		SyntaxPrefixMinusx2(SyntaxExpr* _expr) : expr(_expr) {}
		~SyntaxPrefixMinusx2() { delete expr; }
		
		std::string GetType() const { return "minusx2"; }
		bool CreateCode(CompliationDesc* cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(OpCode::SUB);
			if (!expr->CreateLCode(cd))
				return false;
			cd->code.push_back(OpCode::POP);
			return true;
		}
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(OpCode::SUB);
			if (!expr->CreateLCode(cd))
				return false;
			return true;
		}
	};
	struct SyntaxPostfixMinusx2 : SyntaxExpr // --
	{
		SyntaxExpr* expr;
		SyntaxPostfixMinusx2(SyntaxExpr* _expr) : expr(_expr) {}
		~SyntaxPostfixMinusx2() { delete expr; }
		
		std::string GetType() const { return "minusx2"; }
		bool CreateCode(CompliationDesc* cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(OpCode::SUB);
			if (!expr->CreateLCode(cd))
				return false;
			cd->code.push_back(OpCode::POP);
			return true;
		}
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(OpCode::SUB);
			if (!expr->CreateLCode(cd))
				return false;
			cd->code.push_back(OpCode::PUSHDWORD);
			cd->code.push_back(float1[0]);
			cd->code.push_back(float1[1]);
			cd->code.push_back(OpCode::ADD);
			return true;
		}
	};
	struct SyntaxNew : SyntaxExpr // new
	{
		SyntaxExpr* expr;
		SyntaxNew(SyntaxExpr* _expr) : expr(_expr) {}
		~SyntaxNew() { delete expr; }
		
		std::string GetType() const { return "new"; }
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.back() = OpCode::NEW;
			return true;
		}
	};
	struct SyntaxAs : SyntaxExpr // as
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxAs(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxAs() { delete lexpr, rexpr; }
		
		std::string GetType() const { return "as"; }
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::AS);
			return true;
		}
	};
	struct SyntaxIs : SyntaxExpr // is
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxIs(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxIs() { delete lexpr, rexpr; }
		
		std::string GetType() const { return "is"; }
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::IS);
			return true;
		}
	};
	struct SyntaxDot : SyntaxExpr // .
	{
		SyntaxExpr* expr;
		std::string str;
		SyntaxDot(SyntaxExpr* _expr, std::string _str) : expr(_expr), str(_str) {}
		~SyntaxDot() { delete expr; }
		
		std::string GetType() const { return "dot"; }
		bool CreateLCode(CompliationDesc* cd)
		{
			size_t str_size = str.size();
			size_t index = 0;
			cd->code.push_back(OpCode::PUSHSTR);
			cd->code.push_back(str_size);
			while (index + 2 <= str_size)
				cd->code.push_back(str[index++] << 8 | str[index++]);
			if (index == str_size)
				cd->code.push_back(0);
			else
				cd->code.push_back(str[index] << 8);
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::REFSET);
			return true;
		}
		bool CreateRCode(CompliationDesc* cd)
		{
			size_t str_size = str.size();
			size_t index = 0;
			cd->code.push_back(OpCode::PUSHSTR);
			cd->code.push_back(str_size);
			while (index + 2 <= str_size)
				cd->code.push_back(str[index++] << 8 | str[index++]);
			if (index == str_size)
				cd->code.push_back(0);
			else
				cd->code.push_back(str[index] << 8);
			if (!expr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::REFGET);
			return true;
		}
	};
	struct SyntaxRef : SyntaxExpr // arr[0]
	{
		SyntaxExpr* lexpr;
		SyntaxExpr* rexpr;
		SyntaxRef(SyntaxExpr* _lexpr, SyntaxExpr* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxRef() { delete lexpr, rexpr; }
		
		std::string GetType() const { return "ref"; }
		bool CreateLCode(CompliationDesc* cd)
		{
			if (!rexpr->CreateRCode(cd))
				return false;
			if (!lexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::ARRSET);
			return true;
		}
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!rexpr->CreateRCode(cd))
				return false;
			if (!lexpr->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::ARRGET);
			return true;
		}
	};
	struct SyntaxCall : SyntaxExpr
	{
		SyntaxExpr* expr;
		std::vector<SyntaxExpr*> params;
		SyntaxCall(SyntaxExpr* _func) : expr(_func) {}
		~SyntaxCall() { delete expr; for (auto param : params) delete param; }
		
		std::string GetType() const { return "call"; }
		bool CreateCode(CompliationDesc* cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			size_t param_size = params.size();
			for (size_t index = 0; index < param_size; ++index)
				if (!params[index]->CreateRCode(cd))
					return false;
			cd->code.push_back(OpCode::CALL);
			cd->code.push_back(param_size);
			cd->code.push_back(OpCode::POP);
			return true;
		}
		bool CreateRCode(CompliationDesc* cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			size_t param_size = params.size();
			for (size_t index = 0; index < param_size; ++index)
				if (!params[index]->CreateRCode(cd))
					return false;
			cd->code.push_back(OpCode::CALL);
			cd->code.push_back(param_size);
			return true;
		}
	};
	struct SyntaxArray : SyntaxExpr
	{
		std::vector<SyntaxExpr*> elements;
		SyntaxArray() {}
		~SyntaxArray() { for (auto element : elements) delete element; }
		
		std::string GetType() const { return "array"; }
		bool CreateRCode(CompliationDesc* cd)
		{
			size_t elements_size = elements.size();
			for (size_t index = 0; index < elements_size; ++index)
				if (!elements[elements_size - index - 1]->CreateRCode(cd))
					return false;
			cd->code.push_back(OpCode::INSTARR);
			cd->code.push_back(elements_size);
			return true;
		}
	};
	struct SyntaxDictionary : SyntaxExpr
	{
		std::vector<std::pair<SyntaxExpr*, SyntaxExpr*>> elements;
		
		std::string GetType() const { return "dictionary"; }

		bool CreateRCode(CompliationDesc* cd)
		{
			size_t elements_size = elements.size();
			for (size_t index = 0; index < elements_size; ++index)
			{
				if (!elements[elements_size - index - 1].second->CreateRCode(cd))
					return false;
				if (!elements[elements_size - index - 1].first->CreateRCode(cd))
					return false;
			}
			cd->code.push_back(OpCode::INSTDIC);
			cd->code.push_back(elements_size);
			return true;
		}
	};
	struct SyntaxDeclare : SyntaxExpr
	{
		std::vector<std::pair<VarDesc, SyntaxExpr*>> elements;

		~SyntaxDeclare() { for (auto element : elements) delete element.second; }
		std::string GetType() const { return "declare"; }
		bool CreateCode(CompliationDesc* cd)
		{
			for (auto element : elements)
			{
				if(cd->scope.empty())
				{
					size_t global_size = cd->global.size();
					for(size_t index = 0; index < global_size; ++index)
						if(cd->global[index].name == element.first.name)
						{
							cd->errors.push_back({"Already declared " + element.first.name, line});
							return false;
						}
					cd->global.push_back({element.first});
					if(element.second != nullptr)
					{
						if (!element.second->CreateRCode(cd))
							return false;
						cd->code.push_back(OpCode::STORE);
						cd->code.push_back(global_size);
						cd->code.push_back(OpCode::POP);
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
						cd->code.push_back(OpCode::PUSHNULL);
					if (index != 0xFFFF)
						cd->code.push_back(OpCode::WRITE), cd->code.push_back(index);
				}
			}
			return true;
		}
		bool CreateRCode(CompliationDesc* cd)
		{
			for (auto element : elements)
			{
				if(cd->scope.empty())
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
					cd->code.push_back(OpCode::PUSHNULL);
				if (index != 0xFFFF)
				{
					cd->code.push_back(OpCode::WRITE);
					cd->code.push_back(index);
				}
				cd->code.push_back(OpCode::READ);
				cd->code.push_back(cd->scope.back().variables.size() - 1);
			}
			return true;
		}
	};
	struct SyntaxBlock : SyntaxExpr
	{
		std::vector<SyntaxExpr*> sents;
		virtual ~SyntaxBlock() { for (auto sent : sents) delete sent; }
		
		bool CreateCode(CompliationDesc* cd)
		{
			cd->scope.push_back(LocalScope());
			size_t sents_size = sents.size();
			for (size_t index = 0; index < sents_size; ++index)
				if (sents[index]->CreateCode(cd) == false)
					return false;
			size_t list_size = cd->scope.back().variables.size();
			if (list_size > 0)
			{
				cd->code.push_back(OpCode::POPTO);
				cd->code.push_back(list_size);
			}
			cd->scope.pop_back();
			return true;
		}
	};
	struct SyntaxTree : SyntaxBlock
	{
		std::vector<Error> errors;
		static bool ParseText(SyntaxTree& code, const std::string& str);
		bool CreateCode(CompliationDesc* cd)
		{
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
				cd->code.push_back(OpCode::RETURN);
			else if (cd->code.back() != OpCode::RETURN)
				cd->code.push_back(OpCode::RETURN);
			return true;
		}
	};
	struct SyntaxFunction : SyntaxExpr
	{
		std::vector<std::string> params;
		SyntaxBlock* sents;
		~SyntaxFunction() { if (sents) delete sents; }
		
		std::string GetType() const { return "function"; }
		bool CreateRCode(CompliationDesc* cd)
		{
			cd->scope.push_back(LocalScope());
			size_t param_size = params.size();
			for (size_t index = 0; index < param_size; ++index)
				cd->scope.back().variables.push_back({params[index], VarDesc::VAR});
			cd->code.push_back(OpCode::PUSHFUNC);
			cd->code.push_back(OpCode::NONE);
			size_t delta = cd->code.size();
			if(param_size > 0)
			{
				cd->code.push_back(OpCode::PARAMSET);
				cd->code.push_back(param_size);
			}
			size_t sents_size = sents->sents.size();
			for (size_t index = 0; index < sents_size; ++index)
				if (!sents->sents[index]->CreateCode(cd))
					return false;
			if (cd->code.back() != OpCode::RETURN)
				cd->code.push_back(OpCode::RETURNNULL);
			cd->code[delta - 1] = cd->code.size() - delta;
			cd->scope.pop_back();
			return true;
		}
	};
	struct SyntaxSingleSentence : SyntaxExpr
	{
		SyntaxExpr* expr;
		SyntaxSingleSentence(SyntaxExpr* _expr = nullptr) : expr(_expr) {}
		~SyntaxSingleSentence() { delete expr; }
		
		bool CreateCode(CompliationDesc* cd)
		{
			if (expr != nullptr)
				return expr->CreateCode(cd);
			return true;
		}
	};
	struct SyntaxIf : SyntaxExpr
	{
		SyntaxExpr* cond;
		SyntaxBlock* truesents;
		SyntaxBlock* falsesents;
		~SyntaxIf() { delete cond; delete truesents; if (falsesents) delete falsesents; }
		
		std::string GetType() const { return "if"; }
		bool CreateCode(CompliationDesc* cd)
		{
			if (!cond->CreateRCode(cd))
				return false;
			cd->code.push_back(OpCode::CFJMP);
			cd->code.push_back(OpCode::NONE);
			size_t tdelta = cd->code.size();
			if (!truesents->CreateCode(cd))
				return false;
			if (falsesents != nullptr)
			{
				cd->code.push_back(OpCode::FJMP);
				cd->code.push_back(OpCode::NONE);
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
		SyntaxExpr* init;
		SyntaxExpr* prefix_condition;
		SyntaxExpr* postfix_condition;
		SyntaxExpr* loop;
		SyntaxBlock* sents;
		~SyntaxLoop() { if (init) delete init; if (prefix_condition) delete prefix_condition; if (postfix_condition) delete postfix_condition; if (loop) delete loop; }
		
		std::string GetType() const { return "loop"; }
		bool CreateCode(CompliationDesc* cd)
		{
			cd->scope.push_back(LocalScope());
			if (init != nullptr)
			{
				if (!init->CreateCode(cd))
					return false;
			}
			cd->code.push_back(OpCode::FJMP);
			cd->code.push_back(OpCode::NONE);
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
				cd->code.push_back(OpCode::CFJMP);
				cd->scope.back().endpoint.push_back(cd->code.size());
				cd->code.push_back(OpCode::NONE);
			}
			if (postfix_condition != nullptr)
			{
				if (!postfix_condition->CreateRCode(cd))
					return false;
				cd->code.push_back(OpCode::CFJMP);
				cd->scope.back().endpoint.push_back(cd->code.size());
				cd->code.push_back(OpCode::NONE);
				cd->code[delta - 1] = cd->code.size() - delta;
			}
			if (!sents->CreateCode(cd))
				return false;
			cd->code.push_back(OpCode::BJMP);
			cd->scope.back().startpoint.push_back(cd->code.size());
			cd->code.push_back(OpCode::NONE);
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
				cd->code.push_back(OpCode::POPTO);
				cd->code.push_back(list_size);
			}
			cd->scope.pop_back();
			return true;
		}
	};
	struct SyntaxReturn : SyntaxExpr
	{
		SyntaxExpr* value;
		SyntaxReturn(SyntaxExpr* _value) : value(_value) { }
		~SyntaxReturn() { delete value; }
		std::string GetType() const { return "return"; }
		bool CreateCode(CompliationDesc* cd)
		{
			if (cd->scope.empty())
			{
				cd->errors.push_back({"Incorrect scope", line});
				return false;
			}
			if (value)
			{
				if (!value->CreateRCode(cd))
					return false;
				cd->code.push_back(OpCode::RETURN);
			}
			else
				cd->code.push_back(OpCode::RETURNNULL);
			return true;
		}
	};
	struct SyntaxContinue : SyntaxExpr
	{
		SyntaxContinue(size_t _line) { line = _line; }
		std::string GetType() const { return "continue"; }
		bool CreateCode(CompliationDesc* cd)
		{
			if (cd->scope.empty())
			{
				cd->errors.push_back({"Incorrect scope", line});
				return false;
			}
			cd->code.push_back(OpCode::BJMP);
			cd->scope.back().startpoint.push_back(cd->code.size());
			cd->code.push_back(OpCode::NONE);
			return true;
		}
	};
	struct SyntaxBreak : SyntaxExpr
	{
		SyntaxBreak(size_t _line) { line = _line; }
		std::string GetType() const { return "break"; }
		bool CreateCode(CompliationDesc* cd)
		{
			if (cd->scope.empty())
			{
				cd->errors.push_back({"Incorrect scope", line});
				return false;
			}
			cd->code.push_back(OpCode::FJMP);
			cd->scope.back().endpoint.push_back(cd->code.size());
			cd->code.push_back(OpCode::NONE);
			return true;
		}
	};
	struct SyntaxClass : SyntaxExpr
	{
		std::vector<SyntaxExpr*> member;
		
		std::string GetType() const { return "class"; }
		bool CreateRCode(CompliationDesc* cd)
		{
			auto delta = cd->code.size();
			cd->scope.push_back(LocalScope());
			for(auto iter : member)
				iter->CreateCode(cd);
			cd->code.push_back(OpCode::DEF);
			cd->code.push_back(cd->code.size() - delta);
			return false;
		}
	};
	SyntaxExpr* ParseExpr(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseComma(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseTernaryOperator(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseAssign(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseOr(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseAnd(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseCmp(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseAdd(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseMul(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParsePow(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParsePrefix(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParsePostfix(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseElement(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseLiteral(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseSimpleLiteral(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseArray(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	std::pair<SyntaxExpr*, SyntaxExpr*>* ParseKeyVal(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseDictionary(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseFunction(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseSentence(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors, size_t args_count ...);
	SyntaxExpr* ParseIf(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseLoop(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseFlowControl(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxExpr* ParseDeclare(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
	SyntaxBlock* ParseBlock(std::vector<Token>& tokens, size_t& index, std::vector<Error>& errors);
};
#endif