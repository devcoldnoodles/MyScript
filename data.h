#ifndef __DATA_H__
#define __DATA_H__

#include <vector>
#include <string>
#include <map>

namespace myscript
{
	struct MetaObject;
	typedef MetaObject *(*CFunc)(std::vector<MetaObject *> args);

	struct Error
	{
		std::string inf;
		size_t line;

		Error(const std::string &_inf, const size_t &_line) : inf(_inf), line(_line) {}
		const std::string ToString()
		{
			constexpr auto buf_size = 64;
			char temp[buf_size];
			if (line)
				sprintf(temp, "[%lu lines] %s", line, inf.c_str());
			else
				sprintf(temp, "%s", inf.c_str());
			return temp;
		}
	};

	enum OpCode : short
	{
		NONE = 0,	// unused value causes error
		PUSH,		// push global value at stack
		PUSHBINARY, // push binary at stack
		PUSHDWORD,	// push 4byte nubmer at stack
		PUSHQWORD,	// push 8byte nubmer at stack
		PUSHSTR,	// push std::string at stack
		PUSHFUNC,	// push function at stack
		PUSHNULL,	// push null at stack
		PUSHTRUE,	// push true at stack
		PUSHFALSE,	// push false at stack
		PUSHZERO,	// push 0(nubmer) at stack
		PUSHONE,	// push 1(number) at stack
		POP,		// remove stack top value
		POPTO,		// remove stack top value by operand
		STORE,		// top value of stack is store to global variable
		FJMP,		// cursor moves forward
		CFJMP,		// If the top value of the stack is false, the cursor moves forward
		BJMP,		// cursor moves backward
		CBJMP,		// If the top value of the stack is false, the cursor moves backward
		RETURN,		// exit the function and returns value
		RETURNNULL, // exit the function and returns null
		READ,		// push local vlaue at stack
		WRITE,		// top value of stack is store to local variable
		READUPPER,	// push another scope`s local value at stack
		WRITEUPPER, // top value of stack is store to another scope`s local value
		CALL,		// call the function
		PARAMSET,	//
		INSTARR,	//
		INSTOBJ,	//
		REFSET,		//
		REFGET,		//
		ARRSET,		//
		ARRGET,		//
		NEW,		//
		DEF,		// define class
		AS,			// operator as
		IS,			// operator is
		EQ,			// operator ==
		NEQ,		// operator !=
		LT,			// operator <
		LE,			// operator <=
		GT,			// operator >
		GE,			// operator >=
		AND,		// operator &&
		OR,			// operator ||
		XOR,		// operator ^
		NOT,		// operator !
		SIGN,		// operator -
		ADD,		// operator +
		SUB,		// operator -
		MUL,		// operator *
		DIV,		// operator /
		MOD,		// operator %
		POW,		// operator **
	};

	struct VarDesc
	{
		std::string name;
		enum : uint16_t
		{
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
		bool operator()(const VarDesc &a, const VarDesc &b) const
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
		std::map<VarDesc, MetaObject *> variables;
	};

	struct CompliationDesc
	{
		std::vector<uint16_t> code;
		std::vector<LocalScope> scope;
		std::vector<VarDesc> global;
		std::vector<Error> errors;
		std::map<VarDesc, MetaObject *, VarDescCompare> globals;

		const uint16_t Identify(const std::string &id) const
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

		const VarDesc *GetScopeVariable(const uint16_t id)
		{
			int32_t unique = id;
			for (size_t index = 0; index < scope.size(); ++index)
				if ((unique -= scope[index].variables.size()) < 0)
					return &scope[index].variables[unique + scope[index].variables.size()];
			return nullptr;
		}

		void Insert(VarDesc desc, MetaObject *object)
		{
			globals[desc] = object;
			global.push_back(desc);
		}
	};
}
#endif