#ifndef PARSER_H
#define PARSER_H
#include "data.h"

namespace myscript 
{
	static short* float1 = (short*)new float(1);
	static short* double1 = (short*)new double(1);
	
	struct SyntaxNode
	{
		size_t line;
		virtual string GetType() { return "node"; }
		virtual bool CreateCode(CompliationData& cd) { return false; }
		virtual bool CreateLCode(CompliationData& cd) { return false; } 
		virtual bool CreateRCode(CompliationData& cd) { return false; }
	};
	struct SyntaxLiteral : SyntaxNode
	{
		Token data;
		SyntaxLiteral(const Token _data) : data(_data) { line = _data.line; }
		bool CreateRCode(CompliationData& cd)
		{
			if (data.type == Token::NULLPTR)
				cd.code.push_back(OpCode::PUSHNULL);
			else if (data.type == Token::TRUE)
				cd.code.push_back(OpCode::PUSHTRUE);
			else if (data.type == Token::FALSE)
				cd.code.push_back(OpCode::PUSHFALSE);
			else if (data.type == Token::LITERAL_NUMBER)
			{
				double value = stod(data.str);
				if (value <= std::numeric_limits<float>::max())
				{
					short* fvalue = (short*)new float(value);
					cd.code.push_back(OpCode::PUSHDWORD);
					cd.code.push_back(fvalue[0]);
					cd.code.push_back(fvalue[1]);
					delete fvalue;
				}
				else
				{
					short* dvalue = (short*)new double(value);
					cd.code.push_back(OpCode::PUSHQWORD);
					cd.code.push_back(dvalue[0]);
					cd.code.push_back(dvalue[1]);
					cd.code.push_back(dvalue[2]);
					cd.code.push_back(dvalue[3]);
					delete dvalue;
				}
			}
			else if (data.type == Token::LITERAL_STRING)
			{
				string& str = data.str;
				size_t str_size = str.size();
				size_t index = 0;
				cd.code.push_back(OpCode::PUSHSTR);
				cd.code.push_back(str_size);
				while (index + 2 <= str_size)
					cd.code.push_back(str[index + 1] << 8 | str[index]), index += 2;
				if (index == str_size)
					cd.code.push_back(0);
				else
					cd.code.push_back(str[index]);
			}
			return true;
		}
		
	};
	struct SyntaxIdentifier : SyntaxNode
	{
		string id;
		
		SyntaxIdentifier(const Token& _token) : id(_token.str) { line = _token.line; }
		bool CreateRCode(CompliationData& cd)
		{
			uint16_t value = cd.Identify(id);
			if (value == 0xFFFF)
			{
				size_t findex = -1;
				size_t ids_size = cd.global.size();
				for (size_t index = 0; index < ids_size; ++index)
					if (cd.global[index].name == id)
					{
						findex = index;
						break;
					}
				if (findex == -1)
				{
					cd.errors.push_back({"Undefined identifier " + id, line});
					return false;
				}
				cd.code.push_back(OpCode::PUSH);
				cd.code.push_back(findex);
				return true;
			}
			cd.code.push_back(OpCode::READ);
			cd.code.push_back(value);
			return true;
		}
		bool CreateLCode(CompliationData& cd)
		{
			uint16_t value = cd.Identify(id);
			if (value == 0xFFFF)
			{
				size_t findex = -1;
				size_t ids_size = cd.global.size();
				for (size_t index = 0; index < ids_size; ++index)
					if (cd.global[index].name == id)
					{
						findex = index;
						break;
					}
				if (findex == -1)
				{
					cd.errors.push_back({"Undefined identifier " + id, line});
					return false;
				}
				if (cd.global[findex].option & VarDesc::CONST)
				{
					cd.errors.push_back({"'" + id + "' is constants value", line});
					return false;
				}
				cd.code.push_back(OpCode::STORE);
				cd.code.push_back(findex);
				return true;
			}
			if(cd.GetScopeVariable(value)->option & VarDesc::CONST)
			{
				cd.errors.push_back({"'" + id + "' is constants value", line});
				return false;
			}
			cd.code.push_back(OpCode::WRITE);
			cd.code.push_back(value);
			return true;
		}
	};
	struct SyntaxSelf : SyntaxNode
	{
		SyntaxSelf(size_t _line) { line = _line; }
		string GetType() const { return "self"; }
	};
	struct SyntaxComma : SyntaxNode // ,
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxComma(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) { line = lexpr->line; }
		~SyntaxComma() { delete lexpr;	delete rexpr; }
		
		bool CreateCode(CompliationData& cd)
		{
			return CreateRCode(cd);
		}
		bool CreateRCode(CompliationData& cd)
		{
			if (!rexpr->CreateRCode(cd))
				return false;
			if (!lexpr->CreateRCode(cd))
				return false;
			return true;
		}
	};
	struct SyntaxNot : SyntaxNode // !
	{
		SyntaxNode* expr;
		SyntaxNot(SyntaxNode* _expr) : expr(_expr) { line = _expr->line; }
		~SyntaxNot() { delete expr; }
		
		bool CreateRCode(CompliationData& cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::NOT);
			return true;
		}
	};
	struct SyntaxOr : SyntaxNode // ||
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxOr(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) { line = lexpr->line; }
		~SyntaxOr() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::OR);
			return true;
		}
	};
	struct SyntaxAnd : SyntaxNode // &&
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxAnd(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxAnd() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::AND);
			return true;
		}
	};
	struct SyntaxXor : SyntaxNode // ^
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxXor(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxXor() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::XOR);
			return true;
		}
	};
	struct SyntaxEqual : SyntaxNode // ==
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxEqual(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxEqual() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::EQ);
			return true;
		}
	};
	struct SyntaxNotEqual : SyntaxNode // !=
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxNotEqual(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxNotEqual() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::NEQ);
			return true;
		}
	};
	struct SyntaxGreatThan : SyntaxNode // >
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxGreatThan(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxGreatThan() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::GT);
			return true;
		}
	};
	struct SyntaxGreatEqual : SyntaxNode // >=
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxGreatEqual(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxGreatEqual() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::GE);
			return true;
		}
	};
	struct SyntaxLessThan : SyntaxNode // <
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxLessThan(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxLessThan() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::LT);
			return true;
		}
	};
	struct SyntaxLessEqual : SyntaxNode // <=
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxLessEqual(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxLessEqual() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::LE);
			return true;
		}
	};
	struct SyntaxAdd : SyntaxNode // +
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxAdd(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxAdd() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::ADD);
			return true;
		}
	};
	struct SyntaxSub : SyntaxNode // -
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxSub(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxSub() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::SUB);
			return true;
		}
	};
	struct SyntaxMul : SyntaxNode // *
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxMul(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxMul() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::MUL);
			return true;
		}
	};
	struct SyntaxDiv : SyntaxNode // /
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxDiv(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxDiv() { delete lexpr, rexpr; }
		
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::DIV);
			return true;
		}
	};
	struct SyntaxMod : SyntaxNode // %
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxMod(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxMod() { delete lexpr, rexpr; }
		
		string GetType() const { return "mod"; }
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::MOD);
			return true;
		}
	};
	struct SyntaxPow : SyntaxNode // **
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxPow(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxPow() { delete lexpr, rexpr; }
		
		string GetType() const { return "pow"; }
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::POW);
			return true;
		}
	};
	struct SyntaxAssign : SyntaxNode
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxAssign(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxAssign() { delete lexpr, rexpr; }
		
		string GetType() const { return "assign"; }
		bool CreateCode(CompliationData& cd)
		{
			if (!rexpr->CreateRCode(cd))
				return false;
			if (!lexpr->CreateLCode(cd))
				return false;
			cd.code.push_back(OpCode::POP);
			return true;
		}
		bool CreateRCode(CompliationData& cd)
		{
			if (!rexpr->CreateRCode(cd))
				return false;
			if (!lexpr->CreateLCode(cd))
				return false;
			return true;
		}
	};
	struct SyntaxPrefixPlus : SyntaxNode // +
	{
		SyntaxNode* expr;
		SyntaxPrefixPlus(SyntaxNode* _expr) : expr(_expr) {}
		~SyntaxPrefixPlus() { delete expr; }
		
		string GetType() const { return "minus"; }
		bool CreateRCode(CompliationData& cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			return true;
		}
	};
	struct SyntaxPrefixMinus : SyntaxNode // -
	{
		SyntaxNode* expr;
		SyntaxPrefixMinus(SyntaxNode* _expr) : expr(_expr) {}
		~SyntaxPrefixMinus() { delete expr; }
		
		string GetType() const { return "minus"; }
		bool CreateRCode(CompliationData& cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::SIGN);
			return true;
		}
	};
	struct SyntaxPrefixPlusx2 : SyntaxNode // ++
	{
		SyntaxNode* expr;
		SyntaxPrefixPlusx2(SyntaxNode* _expr) : expr(_expr) {}
		~SyntaxPrefixPlusx2() { delete expr; }
		
		string GetType() const { return "prefix plusx2"; }
		bool CreateCode(CompliationData& cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::PUSHDWORD);
			cd.code.push_back(float1[0]);
			cd.code.push_back(float1[1]);
			cd.code.push_back(OpCode::ADD);
			if (!expr->CreateLCode(cd))
			{
				cd.errors.push_back({"Incorrect left-expression", expr->line});
				return false;
			}
			cd.code.push_back(OpCode::POP);
			return true;
		}
		bool CreateRCode(CompliationData& cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::PUSHDWORD);
			cd.code.push_back(float1[0]);
			cd.code.push_back(float1[1]);
			cd.code.push_back(OpCode::ADD);
			if (!expr->CreateLCode(cd))
			{
				cd.errors.push_back({"Incorrect left-expression", expr->line});
				return false;
			}
			return true;
		}
	};
	struct SyntaxPostfixPlusx2 : SyntaxNode // ++
	{
		SyntaxNode* expr;
		SyntaxPostfixPlusx2(SyntaxNode* _expr) : expr(_expr) {}
		~SyntaxPostfixPlusx2() { delete expr; }
		
		string GetType() const { return "postfix plusx2"; }
		bool CreateCode(CompliationData& cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::PUSHDWORD);
			cd.code.push_back(float1[0]);
			cd.code.push_back(float1[1]);
			cd.code.push_back(OpCode::ADD);
			if (!expr->CreateLCode(cd))
			{
				cd.errors.push_back({"Incorrect left-expression", expr->line});
				return false;
			}
			cd.code.push_back(OpCode::POP);
			return true;
		}
		bool CreateRCode(CompliationData& cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::PUSHDWORD);
			cd.code.push_back(float1[0]);
			cd.code.push_back(float1[1]);
			cd.code.push_back(OpCode::ADD);
			if (!expr->CreateLCode(cd))
			{
				cd.errors.push_back({"Incorrect left-expression", expr->line});
				return false;
			}
			cd.code.push_back(OpCode::PUSHDWORD);
			cd.code.push_back(float1[0]);
			cd.code.push_back(float1[1]);
			cd.code.push_back(OpCode::SUB);
			return true;
		}
	};
	struct SyntaxPrefixMinusx2 : SyntaxNode // --
	{
		SyntaxNode* expr;
		SyntaxPrefixMinusx2(SyntaxNode* _expr) : expr(_expr) {}
		~SyntaxPrefixMinusx2() { delete expr; }
		
		string GetType() const { return "minusx2"; }
		bool CreateCode(CompliationData& cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::PUSHDWORD);
			cd.code.push_back(float1[0]);
			cd.code.push_back(float1[1]);
			cd.code.push_back(OpCode::SUB);
			if (!expr->CreateLCode(cd))
			{
				cd.errors.push_back({"Incorrect left-expression", expr->line});
				return false;
			}
			cd.code.push_back(OpCode::POP);
			return true;
		}
		bool CreateRCode(CompliationData& cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::PUSHDWORD);
			cd.code.push_back(float1[0]);
			cd.code.push_back(float1[1]);
			cd.code.push_back(OpCode::SUB);
			if (!expr->CreateLCode(cd))
			{
				cd.errors.push_back({"Incorrect left-expression", expr->line});
				return false;
			}
			return true;
		}
	};
	struct SyntaxPostfixMinusx2 : SyntaxNode // --
	{
		SyntaxNode* expr;
		SyntaxPostfixMinusx2(SyntaxNode* _expr) : expr(_expr) {}
		~SyntaxPostfixMinusx2() { delete expr; }
		
		string GetType() const { return "minusx2"; }
		bool CreateCode(CompliationData& cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::PUSHDWORD);
			cd.code.push_back(float1[0]);
			cd.code.push_back(float1[1]);
			cd.code.push_back(OpCode::SUB);
			if (!expr->CreateLCode(cd))
			{
				cd.errors.push_back({"Incorrect left-expression", expr->line});
				return false;
			}
			cd.code.push_back(OpCode::POP);
			return true;
		}
		bool CreateRCode(CompliationData& cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::PUSHDWORD);
			cd.code.push_back(float1[0]);
			cd.code.push_back(float1[1]);
			cd.code.push_back(OpCode::SUB);
			if (!expr->CreateLCode(cd))
			{
				cd.errors.push_back({"Incorrect left-expression", expr->line});
				return false;
			}
			cd.code.push_back(OpCode::PUSHDWORD);
			cd.code.push_back(float1[0]);
			cd.code.push_back(float1[1]);
			cd.code.push_back(OpCode::ADD);
			return true;
		}
	};
	struct SyntaxNew : SyntaxNode // new
	{
		SyntaxNode* expr;
		SyntaxNew(SyntaxNode* _expr) : expr(_expr) {}
		~SyntaxNew() { delete expr; }
		
		string GetType() const { return "new"; }
		bool CreateRCode(CompliationData& cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			cd.code.back() = OpCode::INST;
			return true;
		}
	};
	struct SyntaxAs : SyntaxNode // as
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxAs(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxAs() { delete lexpr, rexpr; }
		
		string GetType() const { return "as"; }
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::AS);
			return true;
		}
	};
	struct SyntaxIs : SyntaxNode // is
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxIs(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxIs() { delete lexpr, rexpr; }
		
		string GetType() const { return "is"; }
		bool CreateRCode(CompliationData& cd)
		{
			if (!lexpr->CreateRCode(cd))
				return false;
			if (!rexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::IS);
			return true;
		}
	};
	struct SyntaxDot : SyntaxNode // .
	{
		SyntaxNode* expr;
		string str;
		SyntaxDot(SyntaxNode* _expr, string _str) : expr(_expr), str(_str) {}
		~SyntaxDot() { delete expr; }
		
		string GetType() const { return "dot"; }
		bool CreateLCode(CompliationData& cd)
		{
			size_t str_size = str.size();
			size_t index = 0;
			cd.code.push_back(OpCode::PUSHSTR);
			cd.code.push_back(str_size);
			while (index + 2 <= str_size)
				cd.code.push_back(str[index++] << 8 | str[index++]);
			if (index == str_size)
				cd.code.push_back(0);
			else
				cd.code.push_back(str[index] << 8);
			if (!expr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::REFSET);
			return true;
		}
		bool CreateRCode(CompliationData& cd)
		{
			size_t str_size = str.size();
			size_t index = 0;
			cd.code.push_back(OpCode::PUSHSTR);
			cd.code.push_back(str_size);
			while (index + 2 <= str_size)
				cd.code.push_back(str[index++] << 8 | str[index++]);
			if (index == str_size)
				cd.code.push_back(0);
			else
				cd.code.push_back(str[index] << 8);
			if (!expr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::REFGET);
			return true;
		}
	};
	struct SyntaxRef : SyntaxNode // arr[0]
	{
		SyntaxNode* lexpr;
		SyntaxNode* rexpr;
		SyntaxRef(SyntaxNode* _lexpr, SyntaxNode* _rexpr) : lexpr(_lexpr), rexpr(_rexpr) {}
		~SyntaxRef() { delete lexpr, rexpr; }
		
		string GetType() const { return "ref"; }
		bool CreateLCode(CompliationData& cd)
		{
			if (!rexpr->CreateRCode(cd))
				return false;
			if (!lexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::ARRSET);
			return true;
		}
		bool CreateRCode(CompliationData& cd)
		{
			if (!rexpr->CreateRCode(cd))
				return false;
			if (!lexpr->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::ARRGET);
			return true;
		}
	};
	struct SyntaxCall : SyntaxNode
	{
		SyntaxNode* expr;
		vector<SyntaxNode*> params;
		SyntaxCall(SyntaxNode* _func) : expr(_func) {}
		~SyntaxCall() { delete expr; for (auto param : params) delete param; }
		
		string GetType() const { return "call"; }
		bool CreateCode(CompliationData& cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			size_t param_size = params.size();
			for (size_t index = 0; index < param_size; ++index)
				if (!params[index]->CreateRCode(cd))
					return false;
			cd.code.push_back(OpCode::CALL);
			cd.code.push_back(param_size);
			cd.code.push_back(OpCode::POP);
			return true;
		}
		bool CreateRCode(CompliationData& cd)
		{
			if (!expr->CreateRCode(cd))
				return false;
			size_t param_size = params.size();
			for (size_t index = 0; index < param_size; ++index)
				if (!params[index]->CreateRCode(cd))
					return false;
			cd.code.push_back(OpCode::CALL);
			cd.code.push_back(param_size);
			return true;
		}
	};
	struct SyntaxArray : SyntaxNode
	{
		vector<SyntaxNode*> elements;
		SyntaxArray() {}
		~SyntaxArray() { for (auto element : elements) delete element; }
		
		string GetType() const { return "array"; }
		bool CreateRCode(CompliationData& cd)
		{
			size_t elements_size = elements.size();
			for (size_t index = 0; index < elements_size; ++index)
				if (!elements[elements_size - index - 1]->CreateRCode(cd))
					return false;
			cd.code.push_back(OpCode::INSTARR);
			cd.code.push_back(elements_size);
			return true;
		}
	};
	struct SyntaxDictionary : SyntaxNode
	{
		vector<pair<SyntaxNode*, SyntaxNode*>> elements;
		
		string GetType() const { return "dictionary"; }

		bool CreateRCode(CompliationData& cd)
		{
			size_t elements_size = elements.size();
			for (size_t index = 0; index < elements_size; ++index)
			{
				if (!elements[elements_size - index - 1].second->CreateRCode(cd))
					return false;
				if (!elements[elements_size - index - 1].first->CreateRCode(cd))
					return false;
			}
			cd.code.push_back(OpCode::INSTDIC);
			cd.code.push_back(elements_size);
			return true;
		}
	};
	struct SyntaxDeclare : SyntaxNode
	{
		vector<pair<VarDesc, SyntaxNode*>> elements;

		~SyntaxDeclare() { for (auto element : elements) delete element.second; }
		string GetType() const { return "declare"; }
		bool CreateCode(CompliationData& cd)
		{
			for (auto element : elements)
			{
				if(cd.scope.empty())
				{
					size_t global_size = cd.global.size();
					for(size_t index = 0; index < global_size; ++index)
						if(cd.global[index].name == element.first.name)
						{
							cd.errors.push_back({"Already declared " + element.first.name, line});
							return false;
						}
					cd.global.push_back({element.first});
					if(element.second != nullptr)
					{
						if (!element.second->CreateRCode(cd))
							return false;
						cd.code.push_back(OpCode::STORE);
						cd.code.push_back(global_size);
						cd.code.push_back(OpCode::POP);
					}
				}
				else
				{
					uint16_t index = cd.Identify(element.first.name);
					if (index == 0xFFFF)
						cd.scope.back().idlist.push_back({element.first});
					if (element.second)
					{
						if (!element.second->CreateRCode(cd))
							return false;
					}
					else
						cd.code.push_back(OpCode::PUSHNULL);
					if (index != 0xFFFF)
						cd.code.push_back(OpCode::WRITE), cd.code.push_back(index);
				}
			}
			return true;
		}
		bool CreateRCode(CompliationData& cd)
		{
			for (auto element : elements)
			{
				if(cd.scope.empty())
				{
					cd.errors.push_back({"Incorrect right expr declare must in scope range", line});
					return false;
				}
				uint16_t index = cd.Identify(element.first.name);
				if (index == 0xFFFF)
					cd.scope.back().idlist.push_back({element.first});
				if (element.second)
				{
					if (!element.second->CreateRCode(cd))
						return false;
				}
				else
					cd.code.push_back(OpCode::PUSHNULL);
				if (index != 0xFFFF)
				{
					cd.code.push_back(OpCode::WRITE);
					cd.code.push_back(index);
				}
				cd.code.push_back(OpCode::READ);
				cd.code.push_back(cd.scope.back().idlist.size() - 1);
			}
			return true;
		}
	};
	struct SyntaxBlock : SyntaxNode
	{
		vector<SyntaxNode*> sents;
		virtual ~SyntaxBlock() { for (auto sent : sents) delete sent; }
		
		bool CreateCode(CompliationData& cd)
		{
			cd.scope.push_back(LocalScope());
			size_t sents_size = sents.size();
			for (size_t index = 0; index < sents_size; ++index)
				if (sents[index]->CreateCode(cd) == false)
					return false;
			size_t list_size = cd.scope.back().idlist.size();
			if (list_size > 0)
			{
				cd.code.push_back(OpCode::POPTO);
				cd.code.push_back(list_size);
			}
			cd.scope.pop_back();
			return true;
		}
	};
	struct SyntaxTree : SyntaxBlock
	{
		vector<Error> errors;
		static bool ParseText(SyntaxTree& code, const string& str);
		bool CreateCode(CompliationData& cd)
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
			if (cd.code.empty())
				cd.code.push_back(OpCode::RETURN);
			else if (cd.code.back() != OpCode::RETURN)
				cd.code.push_back(OpCode::RETURN);
			return true;
		}
	};
	struct SyntaxFunction : SyntaxNode
	{
		vector<string> params;
		SyntaxBlock* sents;
		~SyntaxFunction() { if (sents) delete sents; }
		
		string GetType() const { return "function"; }
		bool CreateRCode(CompliationData& cd)
		{
			cd.scope.push_back(LocalScope());
			size_t param_size = params.size();
			for (size_t index = 0; index < param_size; ++index)
				cd.scope.back().idlist.push_back({params[index]});
			cd.code.push_back(OpCode::PUSHFUNC);
			cd.code.push_back(OpCode::NONE);
			size_t delta = cd.code.size();
			if(param_size > 0)
			{
				cd.code.push_back(OpCode::PARAMSET);
				cd.code.push_back(param_size);
			}
			size_t sents_size = sents->sents.size();
			for (size_t index = 0; index < sents_size; ++index)
				if (!sents->sents[index]->CreateCode(cd))
					return false;
			if (cd.code.back() != OpCode::RETURN)
				cd.code.push_back(OpCode::RETURNNULL);
			cd.code[delta - 1] = cd.code.size() - delta;
			cd.scope.pop_back();
			return true;
		}
	};
	struct SyntaxSingleSentence : SyntaxNode
	{
		SyntaxNode* expr;
		SyntaxSingleSentence(SyntaxNode* _expr = nullptr) : expr(_expr) {}
		~SyntaxSingleSentence() { delete expr; }
		
		bool CreateCode(CompliationData& cd)
		{
			if (expr != nullptr)
				return expr->CreateCode(cd);
			return true;
		}
	};
	struct SyntaxIf : SyntaxNode
	{
		SyntaxNode* cond;
		SyntaxBlock* truesents;
		SyntaxBlock* falsesents;
		~SyntaxIf() { delete cond; delete truesents; if (falsesents) delete falsesents; }
		
		string GetType() const { return "if"; }
		bool CreateCode(CompliationData& cd)
		{
			if (!cond->CreateRCode(cd))
				return false;
			cd.code.push_back(OpCode::CFJMP);
			cd.code.push_back(OpCode::NONE);
			size_t tdelta = cd.code.size();
			if (!truesents->CreateCode(cd))
				return false;
			if (falsesents != nullptr)
			{
				cd.code.push_back(OpCode::FJMP);
				cd.code.push_back(OpCode::NONE);
				cd.code[tdelta - 1] = cd.code.size() - tdelta;
				size_t fdelta = cd.code.size();
				if (!falsesents->CreateCode(cd))
					return false;
				cd.code[fdelta - 1] = cd.code.size() - fdelta;
			}
			else
				cd.code[tdelta - 1] = cd.code.size() - tdelta;
			return true;
		}
	};
	struct SyntaxLoop : SyntaxNode
	{
		SyntaxNode* init;
		SyntaxNode* prefix_condition;
		SyntaxNode* postfix_condition;
		SyntaxNode* loop;
		SyntaxBlock* sents;
		~SyntaxLoop() { if (init) delete init; if (prefix_condition) delete prefix_condition; if (postfix_condition) delete postfix_condition; if (loop) delete loop; }
		
		string GetType() const { return "loop"; }
		bool CreateCode(CompliationData& cd)
		{
			cd.scope.push_back(LocalScope());
			if (init != nullptr)
			{
				if (!init->CreateCode(cd))
				{
					cd.errors.push_back({"Incorrect expression", init->line});
					return false;
				}
			}
			cd.code.push_back(OpCode::FJMP);
			cd.code.push_back(OpCode::NONE);
			size_t delta = cd.code.size();
			if (loop != nullptr)
			{
				if (!loop->CreateCode(cd))
				{
					cd.errors.push_back({"Incorrect expression", loop->line});
					return false;
				}
				cd.code[delta - 1] = cd.code.size() - delta;
			}
			if (prefix_condition != nullptr)
			{
				if (!prefix_condition->CreateRCode(cd))
				{
					cd.errors.push_back({"Incorrect expression", prefix_condition->line});
					return false;
				}
				cd.code.push_back(OpCode::CFJMP);
				cd.scope.back().endpoint.push_back(cd.code.size());
				cd.code.push_back(OpCode::NONE);
			}
			if (postfix_condition != nullptr)
			{
				if (!postfix_condition->CreateRCode(cd))
				{
					cd.errors.push_back({"Incorrect expression", prefix_condition->line});
					return false;
				}
				cd.code.push_back(OpCode::CFJMP);
				cd.scope.back().endpoint.push_back(cd.code.size());
				cd.code.push_back(OpCode::NONE);
				cd.code[delta - 1] = cd.code.size() - delta;
			}
			if (!sents->CreateCode(cd))
				return false;
			cd.code.push_back(OpCode::BJMP);
			cd.scope.back().startpoint.push_back(cd.code.size());
			cd.code.push_back(OpCode::NONE);
			for (size_t index : cd.scope.back().startpoint)
			{
				cd.code[index] = index - delta + 1;
			}
			for (size_t index : cd.scope.back().endpoint)
			{
				cd.code[index] = cd.code.size() - index - 1;
			}
			size_t list_size = cd.scope.back().idlist.size();
			if (list_size > 0)
			{
				cd.code.push_back(OpCode::POPTO);
				cd.code.push_back(list_size);
			}
			cd.scope.pop_back();
			return true;
		}
	};
	struct SyntaxReturn : SyntaxNode
	{
		SyntaxNode* value;
		SyntaxReturn(SyntaxNode* _value) : value(_value) { line = value->line; }
		~SyntaxReturn() { delete value; }
		string GetType() const { return "return"; }
		bool CreateCode(CompliationData& cd)
		{
			if (cd.scope.empty())
			{
				cd.errors.push_back({"Incorrect scope", line});
				return false;
			}
			if (value)
			{
				if (!value->CreateRCode(cd))
				{
					cd.errors.push_back({"Incorrect expression", value->line});
					return false;
				}
				cd.code.push_back(OpCode::RETURN);
			}
			else
				cd.code.push_back(OpCode::RETURNNULL);
			return true;
		}
	};
	struct SyntaxContinue : SyntaxNode
	{
		SyntaxContinue(size_t _line) { line = _line; }
		string GetType() const { return "continue"; }
		bool CreateCode(CompliationData& cd)
		{
			if (cd.scope.empty())
			{
				cd.errors.push_back({"Incorrect scope", line});
				return false;
			}
			cd.code.push_back(OpCode::BJMP);
			cd.scope.back().startpoint.push_back(cd.code.size());
			cd.code.push_back(OpCode::NONE);
			return true;
		}
	};
	struct SyntaxBreak : SyntaxNode
	{
		SyntaxBreak(size_t _line) { line = _line; }
		string GetType() const { return "break"; }
		bool CreateCode(CompliationData& cd)
		{
			if (cd.scope.empty())
			{
				cd.errors.push_back({"Incorrect scope", line});
				return false;
			}
			cd.code.push_back(OpCode::FJMP);
			cd.scope.back().endpoint.push_back(cd.code.size());
			cd.code.push_back(OpCode::NONE);
			return true;
		}
	};
	struct SyntaxClass : SyntaxNode
	{
		vector<SyntaxNode*> member;
		
		string GetType() const { return "class"; }
		bool CreateRCode(CompliationData& cd)
		{
			auto delta = cd.code.size();
			cd.scope.push_back(LocalScope());
			for(auto iter : member)
				iter->CreateCode(cd);
			cd.code.push_back(OpCode::DEF);
			cd.code.push_back(cd.code.size() - delta);
			return false;
		}
	};
	SyntaxNode* ParseExpr(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseComma(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseAssign(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseOr(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseAnd(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseCmp(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseAdd(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseMul(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParsePow(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParsePrefix(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParsePostfix(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseElement(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseLiteral(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseSimpleLiteral(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseArray(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	pair<SyntaxNode*, SyntaxNode*>* ParseKeyVal(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseDictionary(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseFunction(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseSentence(vector<Token>& tokens, size_t& index, vector<Error>& errors, size_t args_count ...);
	SyntaxNode* ParseIf(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseLoop(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseFlowControl(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxNode* ParseDeclare(vector<Token>& tokens, size_t& index, vector<Error>& errors);
	SyntaxBlock* ParseBlock(vector<Token>& tokens, size_t& index, vector<Error>& errors);
};
#endif