#ifndef __DATA_H__
#define __DATA_H__
#include"pch.h"

namespace myscript 
{
	struct MetaObject;
	typedef MetaObject* (*CTestFunc)(int argc, MetaObject* args);
	typedef MetaObject* (*CFunc)(std::vector<MetaObject*> args);

	struct Token
	{
		enum Type : uint16_t
		{
			NONE, // 
			IDENTIFIER, // 
			COMMENTLINE, // 
			COMMENTBLOCK, // 
			EOT, // 
			ASSIGN, // =
			EQ, // ==
			NEQ, // !=
			QUESTION, // ?
			NOT, // !
			BOR, // |
			ASSIGN_BOR, // |=
			OR, // ||
			BAND, // &
			ASSIGN_BAND, // &=
			AND, // &&
			ADD, // +
			ASSIGN_ADD, // +=
			INC, // ++
			SUB, // -
			ASSIGN_SUB, // -=
			DEC, // --	
			MUL, // *
			ASSIGN_MUL, // *=
			POW, // **
			DIV, // /
			ASSIGN_DIV, // /=
			MOD, // %
			ASSIGN_MOD, // %=
			XOR, // ^
			ASSIGNXOR, // ^=
			GT, // >
			GE, // >=
			LT, // <
			LE, // <=
			LPARAM, // (
			RPARAM, // )
			LBRACKET, // {
			RBRACKET, // }
			LSUBRACKET, // [
			RSUBRACKET, // ]
			COMMA, // ,
			DOT, // .
			BACKSLASH, //
			COLON, // :
			SEMICOLON, // ;	
			APOSTROPHE, // '
			QUOTATION, // "
			NUMBER, // literal of number
			STRING, // literal of std::string
			VAR, // var
			CONST, // const
			STATIC, // static
			FUNCTION, // function
			CLASS, // struct
			PUBLIC, // public
			PRIVATE, // private
			PROTECT, // protect
			IF, // if
			ELSE, // else
			MATCh, // match
			DEFAULT, // default
			LOOP, // loop
			CONTINUE, // continue
			BREAK, // break
			RETURN, // return
			TRUE, // true
			FALSE, // false
			NULLPTR, // null
			NEW, // new
			SELF, // this
			IS, // is
			AS, // as
		} type;
		std::string str;
		size_t line;
	};
	struct Error
	{
		std::string inf;
		size_t line;
	};
	enum OpCode : uint16_t
	{
		NONE = 0, // unused value causes error
		PUSH, // push global value at stack
		PUSHBINARY, // push binary at stack
		PUSHDWORD, // push 4byte nubmer at stack 
		PUSHQWORD, // push 8byte nubmer at stack
		PUSHSTR, //  push std::string at stack
		PUSHFUNC, // push function at stack
		PUSHNULL, // push null at stack
		PUSHTRUE, // push true at stack
		PUSHFALSE, // push false at stack
		PUSHZERO, // push 0(nubmer) at stack
		PUSHONE, // push 1(number) at stack
		POP, // remove stack top value
		POPTO, // remove stack top value by operand
		STORE, // top value of stack is store to global variable
		FJMP, // cursor moves forward
		CFJMP, // If the top value of the stack is false, the cursor moves forward
		BJMP, // cursor moves backward
		CBJMP, // If the top value of the stack is false, the cursor moves backward
		RETURN, // exit the function and returns value
		RETURNNULL,// exit the function and returns null
		READ, // push local vlaue at stack
		WRITE, // top value of stack is store to local variable
		READUPPER, // push another scope`s local value at stack
		WRITEUPPER, //top value of stack is store to another scope`s local value	
		CALL, // call the function
		PARAMSET, // 
		INSTARR, //
		INSTDIC, //
		REFSET, //
		REFGET, //
		ARRSET, //
		ARRGET, //
		NEW, //
		DEF, // define class
		AS, // operator as
		IS, // operator is
		EQ, // operator ==
		NEQ, // operator !=
		GT, // operator >
		GE, // operator >=
		LT, // operator <
		LE, // operator <=
		AND, // operator &&
		OR, // operator ||
		XOR, // operator ^
		NOT, // operator !
		SIGN, // operator -
		ADD, // operator +
		SUB, // operator -
		MUL, // operator *
		DIV, // operator /
		MOD, // operator %
		POW, // operator **
	};
	struct VarDesc
	{
		std::string name;
		enum : uint16_t {
			VAR = 0,
			CONST = 1,
			STATIC = 2,
			PUBLIC = 4,
			PRIVATE = 8,
			INTERNAL = 16,
		} option;
	};

	struct VarDescCompare
	{
		bool operator()(const VarDesc& a, const VarDesc& b) const
		{
			return a.name < b.name;
		}
	};

	struct LocalScope
	{
		std::vector<VarDesc> variables;
		std::vector<size_t> startpoint;
		std::vector<size_t> endpoint;
		std::map<std::string, std::vector<size_t>> labels;
	};
	struct ScriptState
	{
		std::vector<LocalScope> scope;
		std::map<VarDesc, MetaObject*> variables;
	};
	struct CompliationDesc
	{
		std::vector<uint16_t> code;
		std::vector<LocalScope> scope;
		std::vector<VarDesc> global;
		std::vector<Error> errors;
		std::map<VarDesc, MetaObject*, VarDescCompare> globals;

		const uint16_t Identify(const std::string& id) const
		{
			size_t scope_size = scope.size();
			for (size_t depth = 0; depth < scope_size; ++depth)
			{
				size_t scope_depth = scope_size - depth - 1;
				size_t id_size = scope[scope_depth].variables.size();
				for (size_t index = 0; index < id_size; ++index)
				{
					if (scope[scope_depth].variables[index].name == id)
					{
						size_t total = 0;
						for (size_t scope_index = 0; scope_index < scope_depth; ++scope_index)
							total += scope[scope_index].variables.size();
						return total + index;
					}
				}
			}
			return -1;
		}
		const VarDesc* GetScopeVariable(const uint16_t id)
		{
			int32_t unique = id;
			for (size_t index = 0; index < scope.size(); ++index)
				if((unique -= scope[index].variables.size()) < 0)
					return &scope[index].variables[unique + scope[index].variables.size()];
			return nullptr;
		}

		void Insert(VarDesc desc, MetaObject* object)
		{
			globals[desc] = object;
			global.push_back(desc);
		}
	};
}
#endif