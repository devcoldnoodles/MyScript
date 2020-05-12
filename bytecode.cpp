#include"pch.h"
#include"bytecode.h"

namespace myscript 
{
	const string ToString(Object::Type type)
	{
		return type == Object::NULLPTR ? "null" :
			type == Object::BOOLEAN ? "boolean" :
			type == Object::NUMBER ? "number" :
			type == Object::STRING ? "string" :
			type == Object::FUNCTION ? "function" :
			type == Object::ARRAY ? "array" :
			type == Object::OBJECT ? "object" :
			type == Object::COBJECT ? "cobject" :
			type == Object::CFUNCTION ? "cfunction" : "error";
	}
	const string ToString(Object* object)
	{
		switch (object->type)
		{
		case Object::NULLPTR:
			return "null";
		case Object::BOOLEAN:
			return object->adinf ? "true" : "false";
		case Object::NUMBER:
			char nbuf[FORMATSIZE];
			sprintf_s(nbuf, FORMATSIZE - 1, "%g", *(double*)object->content);
			return nbuf;
		case Object::STRING:
			return object->content;
		case Object::FUNCTION:
			return "function";
		case Object::CFUNCTION:
			return "cfunction";
		case Object::ARRAY:
		{
			string temp = "array [ ";
			Object** element_content = (Object**)object->content;
			size_t count = object->size / sizeof(Object*);
			for (size_t index = 0; index < count; ++index)
				temp += "(" + ToString(element_content[index]) + ") ";
			temp += "]";
			return temp;
		}
		case Object::OBJECT:
		{
			string temp = "object { \n";
			Object** element_content = (Object**)object->content;
			size_t count = object->size / sizeof(Object*) / 2;
			for (size_t index = 0; index < count; ++index)
				temp += ToString(element_content[index * 2]) + " : " + ToString(element_content[index * 2 + 1]) + "\n";
			temp += "}";
			return temp;
		}
		case Object::COBJECT:
			return "wrapping cbject";
		default:
			return "error";
		}
	}
	const string ToString(Error err)
	{
		constexpr unsigned int buf_size = 64;
		char temp[buf_size];
		sprintf_s(temp, buf_size, "[%llu lines] %s", err.line, err.inf.c_str());
		return temp;
	}

	ADRMemory::ADRMemory(CompliationData& data) : codes(data.code)
	{
		memory = (char*)malloc(capacity = 1024 * 1024);
		Lock(p_null = CreateHeader(Object::NULLPTR, 0));
		Lock(p_true = CreateHeader(Object::BOOLEAN, 0, 1));
		Lock(p_false = CreateHeader(Object::BOOLEAN, 0));
		for(auto iter : data.global)
		{
			names.push_back(iter.name);
			global.push_back(p_null);
			Lock(p_null);
		}
		for (auto iter : data.regist_func)
			SetGlobalValue(GetGlobalIndex(iter.first), CreateHeader(Object::CFUNCTION, sizeof(CFunction), 0, &iter.second));
	}
	ADRMemory::~ADRMemory()
	{
		free(memory);
	}
	Object* ADRThread::OperateEQ(Object* l, Object* r)
	{
		Object* addr = machine->p_null;
		switch (l->type)
		{
		case Object::NUMBER:
			if (r->type == Object::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = *lvalue == *rvalue ? machine->p_true : machine->p_false;
			}
			break;
		case Object::STRING:
			if (r->type == Object::STRING)
			{
				char* lvalue = (char*)l->content;
				char* rvalue = (char*)r->content;
				addr = strcmp(lvalue, rvalue) == 0 ? machine->p_true : machine->p_false;
			}
			break;
		case Object::BOOLEAN:
			if (r->type == Object::BOOLEAN)
			{
				addr = l == r ? machine->p_true : machine->p_false;
			}
			break;
		default:
			addr = l == r ? machine->p_true : machine->p_false;
			break;
		}
		return addr;
	}
	Object* ADRThread::OperateNEQ(Object* l, Object* r)
	{
		Object* addr = machine->p_null;
		switch (l->type)
		{
		case Object::NUMBER:
			if (r->type == Object::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = *lvalue != *rvalue ? machine->p_true : machine->p_false;
			}
			break;
		case Object::STRING:
			if (r->type == Object::STRING)
			{
				char* lvalue = (char*)l->content;
				char* rvalue = (char*)r->content;
				addr = strcmp(lvalue, rvalue) != 0 ? machine->p_true : machine->p_false;
			}
			break;
		case Object::BOOLEAN:
			if (r->type == Object::BOOLEAN)
			{
				addr = l != r ? machine->p_true : machine->p_false;
			}
			break;
		default:
			addr = l != r ? machine->p_true : machine->p_false;
			break;
		}
		return addr;
	}
	Object* ADRThread::OperateGT(Object* l, Object* r)
	{
		Object* addr = machine->p_null;
		switch (l->type)
		{
		case Object::NUMBER:
			if (r->type == Object::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = *lvalue > * rvalue ? machine->p_true : machine->p_false;
			}
			break;
		case Object::STRING:
			if (r->type == Object::STRING)
			{
				char* lvalue = (char*)l->content;
				char* rvalue = (char*)r->content;
				addr = strcmp(lvalue, rvalue) > 0 ? machine->p_true : machine->p_false;
			}
			break;
		}
		return addr;
	}
	Object* ADRThread::OperateGE(Object* l, Object* r)
	{
		Object* addr = machine->p_null;
		switch (l->type)
		{
		case Object::NUMBER:
			if (r->type == Object::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = *lvalue >= *rvalue ? machine->p_true : machine->p_false;
			}
			break;
		case Object::STRING:
			if (r->type == Object::STRING)
			{
				char* lvalue = (char*)l->content;
				char* rvalue = (char*)r->content;
				addr = strcmp(lvalue, rvalue) >= 0 ? machine->p_true : machine->p_false;
			}
			break;
		}
		return addr;
	}
	Object* ADRThread::OperateLT(Object* l, Object* r)
	{
		Object* addr = machine->p_null;
		switch (l->type)
		{
		case Object::NUMBER:
			if (r->type == Object::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = *lvalue < *rvalue ? machine->p_true : machine->p_false;
			}
			break;
		case Object::STRING:
			if (r->type == Object::STRING)
			{
				char* lvalue = (char*)l->content;
				char* rvalue = (char*)r->content;
				addr = strcmp(lvalue, rvalue) < 0 ? machine->p_true : machine->p_false;
			}
			break;
		}
		return addr;
	}
	Object* ADRThread::OperateLE(Object* l, Object* r)
	{
		Object* addr = machine->p_null;
		switch (l->type)
		{
		case Object::NUMBER:
			if (r->type == Object::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = *lvalue <= *rvalue ? machine->p_true : machine->p_false;
			}
			break;
		case Object::STRING:
			if (r->type == Object::STRING)
			{
				char* lvalue = (char*)l->content;
				char* rvalue = (char*)r->content;
				addr = strcmp(lvalue, rvalue) <= 0 ? machine->p_true : machine->p_false;
			}
			break;
		}
		return addr;
	}
	Object* ADRThread::OperateNOT(Object* l)
	{
		Object* addr = machine->p_null;
		switch (l->type)
		{
		case Object::BOOLEAN:
			addr = l->adinf ? machine->p_true : machine->p_false;
			break;
		}
		return addr;
	}
	Object* ADRThread::OperateAND(Object* l, Object* r)
	{
		Object* addr = machine->p_null;
		switch (l->type)
		{
		case Object::BOOLEAN:
			if (r->type == Object::BOOLEAN)
			{
				addr = l->adinf && r->adinf ? machine->p_true : machine->p_false;
			}
			break;
		}
		return addr;
	}
	Object* ADRThread::OperateOR(Object* l, Object* r)
	{
		Object* addr = machine->p_null;
		switch (l->type)
		{
		case Object::BOOLEAN:
			if (r->type == Object::BOOLEAN)
			{
				addr = l->adinf || r->adinf ? machine->p_true : machine->p_false;
			}
			break;
		}
		return addr;
	}
	Object* ADRThread::OperateXOR(Object* l, Object* r)
	{
		Object* addr = machine->p_null;
		/*
		switch (l->type)
		{
		case Object::boolean:
			if (r->type == Object::boolean)
			{
				addr = l->adinf || r->adinf ? machine->p_true : machine->p_false;
			}
			break;
		}
		*/
		return addr;
	}
	Object* ADRThread::OperateSIGN(Object* l)
	{
		Object* addr = machine->p_null;
		switch (l->type)
		{
		case Object::NUMBER:
			double* lvalue = (double*)l->content;
			addr = CreateNumber(-*lvalue);
			break;
		}
		return addr;
	}
	Object* ADRThread::OperateADD(Object* l, Object* r)
	{
		Object* addr = machine->p_null;
		switch (l->type)
		{
		case Object::NUMBER:
			if (r->type == Object::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = CreateNumber(*lvalue + *rvalue);
			}
			else if (r->type == Object::STRING)
			{
				double* lvalue = (double*)l->content;
				char* rvalue = (char*)r->content;
				char fmt[FORMATSIZE];
				if (r->size == 0)
					addr = CreateString(fmt, sprintf(fmt, "%g", *lvalue));
				else
					addr = CreateString(fmt, sprintf(fmt, "%g", *lvalue), rvalue, r->size - 1);
			}
			break;
		case Object::STRING:
			if (r->type == Object::NUMBER)
			{
				char* lvalue = (char*)l->content;
				double* rvalue = (double*)r->content;
				char fmt[FORMATSIZE];
				if (l->size == 0)
					addr = CreateString(fmt, sprintf(fmt, "%g", *rvalue));
				else
					addr = CreateString(lvalue, l->size - 1, fmt, sprintf(fmt, "%g", *rvalue));
			}
			else if (r->type == Object::STRING)
			{
				char* lvalue = (char*)l->content;
				char* rvalue = (char*)r->content;
				addr = CreateString(lvalue, l->size - 1, rvalue, r->size - 1);
			}
			break;
		}
		return addr;
	}
	Object* ADRThread::OperateSUB(Object* l, Object* r)
	{
		Object* addr = machine->p_null;
		switch (l->type)
		{
		case Object::NUMBER:
			if (r->type == Object::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = CreateNumber(*lvalue - *rvalue);
			}
			break;
		}
		return addr;
	}
	Object* ADRThread::OperateMUL(Object* l, Object* r)
	{
		Object* addr = machine->p_null;
		switch (l->type)
		{
		case Object::NUMBER:
			if (r->type == Object::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = CreateNumber(*lvalue * *rvalue);
			}
			break;
		}
		return addr;
	}
	Object* ADRThread::OperateDIV(Object* l, Object* r)
	{
		Object* addr = machine->p_null;
		switch (l->type)
		{
		case Object::NUMBER:
			if (r->type == Object::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = CreateNumber(*lvalue / *rvalue);
			}
			break;
		}
		return addr;
	}
	Object* ADRThread::OperateMOD(Object* l, Object* r)
	{
		Object* addr = machine->p_null;
		switch (l->type)
		{
		case Object::NUMBER:
			if (r->type == Object::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = CreateNumber(fmod(*lvalue, *rvalue));
			}
			break;
		}
		return addr;
	}
	Object* ADRThread::OperatePOW(Object* l, Object* r)
	{
		Object* addr = machine->p_null;
		switch (l->type)
		{
		case Object::NUMBER:
			if (r->type == Object::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = CreateNumber(pow(*lvalue, *rvalue));
			}
			break;
		}
		return addr;
	}
	void ADRThread::Execute()
	{
		size_t callback = 1;
		while (callback)
		{
			switch (*cursor++)
			{
			case OpCode::PUSH:
			{
				uint16_t operand = (uint16_t)*cursor++;
				StackPush(machine->global[operand]);
			}
			break;
			case OpCode::STORE:
			{
				uint16_t operand = (uint16_t)*cursor++;
				if (stack.empty())
				{
					callback = -1;
					break;
				}
				machine->UnLock(machine->global[operand]);
				machine->Lock(machine->global[operand] = stack.back());
			}
			break;
			case OpCode::PUSHDWORD:
			{
				StackPush(CreateNumber(*(float *)cursor));
				cursor += 2;
			}
			break;
			case OpCode::PUSHQWORD:
			{
				StackPush(CreateNumber(*(double *)cursor));
				cursor += 4;
			}
			break;
			case OpCode::PUSHSTR:
			{
				uint16_t operand = (uint16_t)*cursor++;
				StackPush(CreateString((char *)cursor, operand));
				cursor += operand / 2 + 1;
			}
			break;
			case OpCode::PUSHFUNC:
			{
				uint16_t operand = (uint16_t)*cursor++;
				StackPush(CreateFunction((OpCode *)cursor, operand));
				cursor += operand;
			}
			break;
			case OpCode::POP:
			{
				StackPop();
			}
			break;
			case OpCode::POPTO:
			{
				uint16_t operand = (uint16_t)*cursor++;
				size_t stack_size = stack.size();
				StackErase(stack_size - operand, stack_size);
			}
			break;
			case OpCode::PUSHNULL:
			{
				StackPush(machine->p_null);
			}
			break;
			case OpCode::PUSHTRUE:
			{
				StackPush(machine->p_true);
			}
			break;
			case OpCode::PUSHFALSE:
			{
				StackPush(machine->p_false);
			}
			break;
			case OpCode::FJMP:
			{
				uint16_t operand = (uint16_t)*cursor++;
				cursor += operand;
			}
			break;
			case OpCode::CFJMP:
			{
				uint16_t operand = (uint16_t)*cursor++;
				Object* cond = stack.back();
				if (cond == machine->p_false || cond == machine->p_null)
					cursor += operand;
				StackPop();
			}
			break;
			case OpCode::BJMP:
			{
				uint16_t operand = (uint16_t)*cursor++;
				cursor -= operand;
			}
			break;
			case OpCode::CBJMP:
			{
				uint16_t operand = (uint16_t)*cursor++;
				Object* cond = stack.back();
				if (cond == machine->p_false || cond == machine->p_null)
					cursor -= operand;
				StackPop();
			}
			break;
			case OpCode::READ:
			{
				uint16_t operand = (uint16_t)*cursor++;
				StackPush(stack[basestack.back() + operand]);
			}
			break;
			case OpCode::WRITE:
			{
				uint16_t operand = (uint16_t)*cursor++;
				machine->UnLock(stack[basestack.back() + operand]);
				machine->Lock(stack.back());
				stack[basestack.back() + operand] = stack.back();
			}
			break;
			case OpCode::READUPPER:
			{
				uint16_t scope = (uint16_t)*cursor++;
				uint16_t index = (uint16_t)*cursor++;
				StackPush(stack[basestack[basestack.size() - scope - 1] + index]);
			}
			break;
			case OpCode::WRITEUPPER:
			{
				uint16_t scope = (uint16_t)*cursor++;
				uint16_t index = (uint16_t)*cursor++;
				uint16_t operand = basestack[basestack.size() - scope - 1] + index;
				machine->UnLock(stack[operand]);
				machine->Lock(stack.back());
				stack[operand] = stack.back();
			}
			break;
			case OpCode::CALL:
			{
				uint16_t operand = (uint16_t)*cursor++;
				Object* inf = stack[stack.size() - operand - 1];
				basestack.push_back(stack.size() - operand);
				switch (inf->type)
				{
				case Object::FUNCTION:
					callstack.push_back(cursor);
					cursor = (uint16_t*)inf->content;
					break;
				case Object::CFUNCTION:
					StackPush((*(CFunction *)inf->content)(this));
					StackErase(basestack.back() - 1, stack.size() - 1);
					basestack.pop_back();
				}
			}
			break;
			case OpCode::PARAMSET:
			{
				uint16_t operand = (uint16_t)*cursor++;
				short delta = stack.size() - basestack.back() - operand;
				if (delta > 0)
					for (short index = 0; index < delta; ++index)
						StackPop();
				else
					for (short index = 0; index > delta; --index)
						StackPush(machine->p_null);
			}
			break;
			case OpCode::RETURN:
			{
				if (callstack.empty())
				{
					callback = -1;
					break;
				}
				cursor = callstack.back();
				callstack.pop_back();
				StackErase(basestack.back() - 1, stack.size() - 1);
				basestack.pop_back();
			}
			break;
			case OpCode::RETURNNULL:
			{
				if (callstack.empty())
				{
					callback = -1;
					break;
				}
				cursor = callstack.back();
				callstack.pop_back();
				StackErase(basestack.back() - 1, stack.size());
				StackPush(machine->p_null);
				basestack.pop_back();
			}
			break;
			case OpCode::INSTARR:
			{
				uint16_t operand = (uint16_t)*cursor++;
				Object* addr = machine->CreateHeader(Object::ARRAY, operand * sizeof(Object* ));
				Object* *content = (Object* *)addr->content;
				for (size_t index = 0; index < operand; ++index)
				{
					content[index] = stack.back();
					stack.pop_back();
				}
				StackPush(addr);
			}
			break;
			case OpCode::INSTDIC:
			{
				uint16_t operand = (uint16_t)*cursor++;
				Object* addr = machine->CreateHeader(Object::OBJECT, operand * sizeof(Object* ) * 2);
				Object* *content = (Object* *)addr->content;
				for (size_t index = 0; index < operand; ++index)
				{
					content[index * 2] = stack.back();
					stack.pop_back();
					content[index * 2 + 1] = stack.back();
					stack.pop_back();
				}
				StackPush(addr);
			}
			break;
			case OpCode::DEF:
			{
				uint16_t operand = (uint16_t)*cursor++;
				StackPush(machine->CreateHeader(Object::METADATA, operand * sizeof(OpCode), 0, cursor));
				cursor += operand;
			}
			break;
			case OpCode::ARRSET:
			{
				if (stack.size() < 3)
				{
					errors.push_back({"잘못된 호출", 0});
					callback = -1;
					break;
				}
				Object* arr = StackBack(0);
				Object* key = StackBack(1);
				Object* value = StackBack(2);
				if (arr->type == Object::ARRAY)
				{
					if (key->type != Object::NUMBER)
					{
						errors.push_back({"인덱스 참조는 숫자만 가능합니다.", 0});
						callback = -1;
						break;
					}
					size_t index = static_cast<size_t>(*(double *)key->content);
					size_t size = arr->size / sizeof(Object* );
					if (index >= size)
					{
						errors.push_back({"참조하려는 인덱스가 범위를 넘어섰습니다.", 0});
						callback = -1;
						break;
					}
					Object* *elements = (Object* *)arr->content;
					machine->UnLock(elements[index]);
					machine->Lock(value);
					elements[index] = value;
					StackPop(3);
					StackPush(elements[index]);
				}
				else if (arr->type == Object::STRING)
				{
					if (key->type != Object::NUMBER)
					{
						errors.push_back({"인덱스 참조는 숫자만 가능합니다.", 0});
						callback = -1;
						break;
					}
					size_t index = static_cast<size_t>(*(double *)key->content);
					if (index >= arr->size)
					{
						errors.push_back({"참조하려는 인덱스가 범위를 넘어섰습니다.", 0});
						callback = -1;
						break;
					}
					if (value->type != Object::STRING)
					{
						errors.push_back({"문자만 대입가능합니다.", 0});
						callback = -1;
						break;
					}
					arr->content[index] = value->content[0];
					StackPop(3);
					StackPush(arr);
				}
				else
				{
					errors.push_back({"왼쪽 표현식은 인덱스 참조가 불가능합니다.", 0});
					callback = -1;
					break;
				}
			}
			break;
			case OpCode::ARRGET:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 호출.", 0});
					callback = -1;
					break;
				}
				Object* arr = StackBack(0);
				Object* key = StackBack(1);
				if (arr->type == Object::ARRAY)
				{
					if (key->type != Object::NUMBER)
					{
						errors.push_back({"인덱스 참조는 숫자만 가능합니다.", 0});
						callback = -1;
						break;
					}
					size_t index = static_cast<size_t>(*(double *)key->content);
					size_t size = arr->size / sizeof(Object* );
					if (index >= size)
					{
						errors.push_back({"참조하려는 인덱스가 범위를 넘어섰습니다.", 0});
						callback = -1;
						break;
					}
					Object* *elements = (Object* *)arr->content;
					StackPop(2);
					StackPush(elements[index]);
				}
				else if (arr->type == Object::STRING)
				{
					if (key->type != Object::NUMBER)
					{
						errors.push_back({"인덱스 참조는 숫자만 가능합니다.", 0});
						callback = -1;
						break;
					}
					size_t index = static_cast<size_t>(*(double *)key->content);
					if (index >= arr->size)
					{
						errors.push_back({"참조하려는 인덱스가 범위를 넘어섰습니다.", 0});
						callback = -1;
						break;
					}
					StackPop(2);
					StackPush(CreateString(&arr->content[index], 1));
				}
				else
				{
					errors.push_back({"왼쪽 표현식은 인덱스 참조가 불가능합니다.", 0});
					callback = -1;
					break;
				}
			}
			break;
			case OpCode::REFSET:
			{
				if (stack.size() < 3)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* arr = StackBack(0);
				Object* key = StackBack(1);
				Object* value = StackBack(2);
				if (arr->type != Object::OBJECT)
				{
					errors.push_back({"참조할 수 없는 객체입니다.", 0});
					callback = -1;
					break;
				}
				Object* *elements = (Object* *)arr->content;
				size_t size = arr->size / sizeof(Object* ) / 2;
				size_t index = 0;
				while (index < size)
				{
					if (OperateEQ(elements[index * 2], key) == machine->p_true)
						break;
					++index;
				}
				if (index >= size)
				{
					errors.push_back({"잘못된 멤버를 참조하였습니다.", 0});
					StackPop(3);
					StackPush(machine->p_null);

					break;
				}
				machine->UnLock(elements[index * 2 + 1]);
				machine->Lock(value);
				elements[index * 2 + 1] = value;
				StackPop(3);
				StackPush(elements[index * 2 + 1]);
			}
			break;
			case OpCode::REFGET:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* arr = StackBack(0);
				Object* key = StackBack(1);
				if (arr->type != Object::OBJECT)
				{
					errors.push_back({"참조할 수 없는 객체입니다.", 0});
					callback = -1;
					break;
				}
				Object* *elements = (Object* *)arr->content;
				size_t size = arr->size / sizeof(Object* ) / 2;
				size_t index = 0;
				while (index < size)
				{
					if (OperateEQ(elements[index * 2], key) == machine->p_true)
						break;
					++index;
				}
				if (index >= size)
				{
					StackPop(2);
					StackPush(machine->p_null);

					break;
				}
				StackPop(2);
				StackPush(elements[index * 2 + 1]);
			}
			break;
			case OpCode::NOT:
			{
				if (stack.size() < 1)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* operand = OperateNOT(stack.back());
				StackPop();
				StackPush(operand);
			}
			break;
			case OpCode::EQ:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* operand = OperateEQ(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::NEQ:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* operand = machine->p_null;
				Object* r = stack_pop();
				Object* l = stack_pop();
				switch (l->type)
				{
				case Object::NUMBER:
					if (r->type == Object::NUMBER)
					{
						double *lvalue = (double *)l->content;
						double *rvalue = (double *)r->content;
						operand = *lvalue != *rvalue ? machine->p_true : machine->p_false;
					}
					break;
				case Object::STRING:
					if (r->type == Object::STRING)
					{
						char *lvalue = (char *)l->content;
						char *rvalue = (char *)r->content;
						operand = strcmp(lvalue, rvalue) != 0 ? machine->p_true : machine->p_false;
					}
					break;
				case Object::BOOLEAN:
					if (r->type == Object::BOOLEAN)
					{
						operand = l != r ? machine->p_true : machine->p_false;
					}
					break;
				default:
					operand = l != r ? machine->p_true : machine->p_false;
					break;
				}
				StackPush(operand);
			}
			break;
			case OpCode::GT:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* operand = machine->p_null;
				Object* r = stack_pop();
				Object* l = stack_pop();
				switch (l->type)
				{
				case Object::NUMBER:
					if (r->type == Object::NUMBER)
					{
						double *lvalue = (double *)l->content;
						double *rvalue = (double *)r->content;
						operand = *lvalue > *rvalue ? machine->p_true : machine->p_false;
					}
					break;
				case Object::STRING:
					if (r->type == Object::STRING)
					{
						char *lvalue = (char *)l->content;
						char *rvalue = (char *)r->content;
						operand = strcmp(lvalue, rvalue) > 0 ? machine->p_true : machine->p_false;
					}
					break;
				}
				StackPush(operand);
			}
			break;
			case OpCode::GE:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* operand = OperateGE(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::LT:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* operand = OperateLT(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::LE:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* operand = OperateLE(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::OR:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* operand = OperateOR(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::AND:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* operand = OperateAND(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::SIGN:
			{
				if (stack.size() < 1)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* operand = OperateSIGN(stack.back());
				StackPop();
				StackPush(operand);
			}
			break;
			case OpCode::ADD:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* operand = OperateADD(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::SUB:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* operand = OperateSUB(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::MUL:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* operand = OperateMUL(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::DIV:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* operand = OperateDIV(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::MOD:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* operand = OperateMOD(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::POW:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = -1;
					break;
				}
				Object* operand = OperatePOW(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			default:
				callback = -1;
				break;
			}
		}
		if (errors.size() > 0)
		{
			for (auto error : errors)
				std::cout << ToString(error).c_str() << std::endl;
		}
	}
}