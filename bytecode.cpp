#include "pch.h"
#include "bytecode.h"

namespace myscript 
{
	MetaObject* CreateMetaObject(MetaObject::Type _type, uint16_t _adinf, uint32_t _size)
	{
		MetaObject* alloc = (MetaObject*)malloc(sizeof(MetaObject) + _size);
		alloc->type = _type;
		alloc->adinf = _adinf;
		alloc->size = _size;
		return alloc;
	}
	MetaObject* CreateMetaObject(MetaObject::Type _type, uint16_t _adinf, uint32_t _size, const void* _content)
	{
		MetaObject* alloc = (MetaObject*)malloc(sizeof(MetaObject) + _size);
		alloc->type = _type;
		alloc->adinf = _adinf;
		alloc->size = _size;
		memcpy(alloc->content, _content, _size);
		return alloc;
	}
	MetaObject* CreateMetaNull(void)
	{
		return CreateMetaObject(MetaObject::NULLPTR, 0, 0);
	}
	MetaObject* CreateMetaBool(const bool boolean)
	{
		return CreateMetaObject(MetaObject::NULLPTR, boolean, 0);
	}
	MetaObject* CreateMetaNumber(const double number)
	{
		return CreateMetaObject(MetaObject::NUMBER, 0, sizeof(double), &number);
	}
	MetaObject* CreateMetaString(const char* str, const size_t size)
	{
		MetaObject* alloc = (MetaObject*)malloc(sizeof(MetaObject) + size + 1);
		alloc->type = MetaObject::STRING;
		alloc->adinf = 0;
		alloc->size = size;
		memcpy(alloc->content, str, size);
		alloc->content[size] = '\0';
		return alloc;
	}
	MetaObject* CreateMetaString(const char* str, const size_t str_size, const char* addit, const size_t addit_size)
	{
		MetaObject* alloc = (MetaObject*)malloc(sizeof(MetaObject) + str_size + addit_size + 1);
		alloc->type = MetaObject::STRING;
		alloc->adinf = 0;
		alloc->size = str_size + addit_size;
		memcpy(alloc->content, str, str_size);
		memcpy(alloc->content + str_size, addit, addit_size);
		alloc->content[str_size + addit_size] = '\0';
		return alloc;
	}
	MetaObject* CreateMetaFunction(const OpCode* src, const size_t size)
	{
		return CreateMetaObject(MetaObject::FUNCTION, 0, sizeof(OpCode) * size, src);
	}
	MetaObject* CreateMetaCFunction(CFunc func)
	{
		return CreateMetaObject(MetaObject::CFUNCTION, 0, sizeof(CFunc), &func);
	}
	MetaObject* CreateMetaCObject(const void* addr, const size_t size)
	{
		return CreateMetaObject(MetaObject::COBJECT, 0, size, addr);
	}
	MetaObject* CreateMetaData(const void* data, const size_t size)
	{
		MetaObject* alloc = CreateMetaObject(MetaObject::METADATA, 0, size);
		//memcpy(alloc->content)
		return alloc;
	}
	const std::string ToString(MetaObject::Type type)
	{
		return type == MetaObject::NULLPTR ? "null" :
			type == MetaObject::BOOLEAN ? "boolean" :
			type == MetaObject::NUMBER ? "number" :
			type == MetaObject::STRING ? "string" :
			type == MetaObject::FUNCTION ? "function" :
			type == MetaObject::ARRAY ? "array" :
			type == MetaObject::OBJECT ? "object" :
			type == MetaObject::COBJECT ? "cobject" :
			type == MetaObject::CFUNCTION ? "cfunction" : "error";
	}
	const std::string ToString(MetaObject* object)
	{
		switch (object->type)
		{
		case MetaObject::NULLPTR:
			return "null";
		case MetaObject::BOOLEAN:
			return object->adinf ? "true" : "false";
		case MetaObject::NUMBER:
			char nbuf[FORMATSIZE];
			sprintf(nbuf, "%g", *(double*)object->content);
			return nbuf;
		case MetaObject::STRING:
			return object->content;
		case MetaObject::FUNCTION:
			return "function";
		case MetaObject::CFUNCTION:
			return "cfunction";
		case MetaObject::ARRAY:
		{
			std::string temp = "array [ ";
			MetaObject** element_content = (MetaObject**)object->content;
			size_t count = object->size / sizeof(MetaObject*);
			for (size_t index = 0; index < count; ++index)
				temp += "(" + ToString(element_content[index]) + ") ";
			temp += "]";
			return temp;
		}
		case MetaObject::OBJECT:
		{
			std::string temp = "object { \n";
			MetaObject** element_content = (MetaObject**)object->content;
			size_t count = object->size / sizeof(MetaObject*) / 2;
			for (size_t index = 0; index < count; ++index)
				temp += ToString(element_content[index * 2]) + " : " + ToString(element_content[index * 2 + 1]) + "\n";
			temp += "}";
			return temp;
		}
		case MetaObject::COBJECT:
			return "wrapping cbject";
		default:
			return "error";
		}
	}
	const std::string ToString(Error err)
	{
		constexpr auto buf_size = 64;
		char temp[buf_size];
		sprintf(temp, "[%lu lines] %s", err.line, err.inf.c_str());
		return temp;
	}
	void VirtualMachine::Execute()
	{
		auto thread = new VirtualThread(this, &codes[0]);
		threads.push_back(thread);
		thread->Execute();
	}
	VirtualMachine::VirtualMachine(CompliationDesc* data) : codes(data->code)
	{
		memory = (char*)malloc(capacity = 1024 * 1024);
		Lock(p_null = CreateHeader(MetaObject::NULLPTR, 0));
		Lock(p_true = CreateHeader(MetaObject::BOOLEAN, 0, 1));
		Lock(p_false = CreateHeader(MetaObject::BOOLEAN, 0));
		for(auto iter : data->global)
		{
			names.push_back(iter.name);
			global.push_back(p_null);
			Lock(p_null);
		}
		for(auto iter : data->globals)
		{
			// glob.insert({iter.first, CopyObject(iter.second)});
			SetGlobalValue(GetGlobalIndex(iter.first.name), CopyObject(iter.second));
			free(iter.second);
		}
	}
	VirtualMachine::~VirtualMachine()
	{
		free(memory);
	}
	MetaObject* VirtualThread::OperateEQ(MetaObject* l, MetaObject* r)
	{
		if(l->type == MetaObject::NUMBER && r->type == MetaObject::NUMBER)
			return *((double*)l->content) == *((double*)r->content) ? machine->p_true : machine->p_false;
		if(l->type == MetaObject::STRING && r->type == MetaObject::STRING)
			return strcmp(l->content, r->content) == 0 ? machine->p_true : machine->p_false;
		return l == r ? machine->p_true : machine->p_false;
	}
	MetaObject* VirtualThread::OperateNEQ(MetaObject* l, MetaObject* r)
	{
		if(l->type == MetaObject::NUMBER && r->type == MetaObject::NUMBER)
			return *((double*)l->content) != *((double*)r->content) ? machine->p_true : machine->p_false;
		if(l->type == MetaObject::STRING && r->type == MetaObject::STRING)
			return strcmp(l->content, r->content) != 0 ? machine->p_true : machine->p_false;
		return l != r ? machine->p_true : machine->p_false;
	}
	MetaObject* VirtualThread::OperateGT(MetaObject* l, MetaObject* r)
	{
		if(l->type == MetaObject::NUMBER && r->type == MetaObject::NUMBER)
			return *((double*)l->content) >= *((double*)r->content) ? machine->p_true : machine->p_false;
		if(l->type == MetaObject::STRING && r->type == MetaObject::STRING)
			return strcmp(l->content, r->content) >= 0 ? machine->p_true : machine->p_false;
		return machine->p_null;
	}
	MetaObject* VirtualThread::OperateGE(MetaObject* l, MetaObject* r)
	{
		if(l->type == MetaObject::NUMBER && r->type == MetaObject::NUMBER)
			return *((double*)l->content) >= *((double*)r->content) ? machine->p_true : machine->p_false;
		if(l->type == MetaObject::STRING && r->type == MetaObject::STRING)
			return strcmp(l->content, r->content) >= 0 ? machine->p_true : machine->p_false;
		return machine->p_null;
	}
	MetaObject* VirtualThread::OperateLT(MetaObject* l, MetaObject* r)
	{
		if(l->type == MetaObject::NUMBER && r->type == MetaObject::NUMBER)
			return *((double*)l->content) < *((double*)r->content) ? machine->p_true : machine->p_false;
		if(l->type == MetaObject::STRING && r->type == MetaObject::STRING)
			return strcmp(l->content, r->content) < 0 ? machine->p_true : machine->p_false;
		return machine->p_null;
	}
	MetaObject* VirtualThread::OperateLE(MetaObject* l, MetaObject* r)
	{
		if(l->type == MetaObject::NUMBER && r->type == MetaObject::NUMBER)
			return *((double*)l->content) <= *((double*)r->content) ? machine->p_true : machine->p_false;
		if(l->type == MetaObject::STRING && r->type == MetaObject::STRING)
			return strcmp(l->content, r->content) <= 0 ? machine->p_true : machine->p_false;
		return machine->p_null;
	}
	MetaObject* VirtualThread::OperateNOT(MetaObject* l)
	{
		if(l->type == MetaObject::BOOLEAN)
			return l->adinf ? machine->p_false : machine->p_true;
		return machine->p_null;
	}
	MetaObject* VirtualThread::OperateAND(MetaObject* l, MetaObject* r)
	{
		if(l->type == MetaObject::BOOLEAN && r->type && MetaObject::BOOLEAN)
			return l->adinf && r->adinf ? machine->p_true : machine->p_false;
		return machine->p_null;
	}
	MetaObject* VirtualThread::OperateOR(MetaObject* l, MetaObject* r)
	{
		if(l->type == MetaObject::BOOLEAN && r->type && MetaObject::BOOLEAN)
			return l->adinf || r->adinf ? machine->p_true : machine->p_false;
		return machine->p_null;
	}
	MetaObject* VirtualThread::OperateXOR(MetaObject* l, MetaObject* r)
	{
		if(l->type == MetaObject::BOOLEAN && r->type && MetaObject::BOOLEAN)
			return l->adinf != r->adinf ? machine->p_true : machine->p_false;
		return machine->p_null;
	}
	MetaObject* VirtualThread::OperateSIGN(MetaObject* l)
	{
		if(l->type == MetaObject::NUMBER)
			return CreateNumber(*((double*)l->content));
		return machine->p_null;
	}
	MetaObject* VirtualThread::OperateADD(MetaObject* l, MetaObject* r)
	{
		MetaObject* addr = machine->p_null;
		switch (l->type)
		{
		case MetaObject::NUMBER:
			if (r->type == MetaObject::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = CreateNumber(*lvalue + *rvalue);
			}
			else if (r->type == MetaObject::STRING)
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
		case MetaObject::STRING:
			if (r->type == MetaObject::NUMBER)
			{
				char* lvalue = (char*)l->content;
				double* rvalue = (double*)r->content;
				char fmt[FORMATSIZE];
				if (l->size == 0)
					addr = CreateString(fmt, sprintf(fmt, "%g", *rvalue));
				else
					addr = CreateString(lvalue, l->size - 1, fmt, sprintf(fmt, "%g", *rvalue));
			}
			else if (r->type == MetaObject::STRING)
			{
				char* lvalue = (char*)l->content;
				char* rvalue = (char*)r->content;
				addr = CreateString(lvalue, l->size - 1, rvalue, r->size - 1);
			}
			break;
		}
		return addr;
	}
	MetaObject* VirtualThread::OperateSUB(MetaObject* l, MetaObject* r)
	{
		MetaObject* addr = machine->p_null;
		switch (l->type)
		{
		case MetaObject::NUMBER:
			switch (r->type)
			{
			case MetaObject::NUMBER:
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = CreateNumber(*lvalue - *rvalue);
			}
			break;
			case MetaObject::STRING:
			{
				double* lvalue = (double*)l->content;
				addr = CreateNumber(*lvalue - atof(r->content));
			}
			break;
			default:
				break;
			}
			break;
		case MetaObject::STRING:
			switch (r->type)
			{
			case MetaObject::NUMBER:
			{
				double* rvalue = (double*)r->content;
				addr = CreateNumber(atof(l->content) - *rvalue);
			}
			break;
			case MetaObject::STRING:
			{
				addr = CreateNumber(atof(l->content) - atof(r->content));
			}
			break;
			default:
				break;
			}
			break;
		}
		return addr;
	}
	MetaObject* VirtualThread::OperateMUL(MetaObject* l, MetaObject* r)
	{
		MetaObject* addr = machine->p_null;
		switch (l->type)
		{
		case MetaObject::NUMBER:
			if (r->type == MetaObject::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = CreateNumber(*lvalue * *rvalue);
			}
			break;
		}
		return addr;
	}
	MetaObject* VirtualThread::OperateDIV(MetaObject* l, MetaObject* r)
	{
		MetaObject* addr = machine->p_null;
		switch (l->type)
		{
		case MetaObject::NUMBER:
			if (r->type == MetaObject::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = CreateNumber(*lvalue / *rvalue);
			}
			break;
		}
		return addr;
	}
	MetaObject* VirtualThread::OperateMOD(MetaObject* l, MetaObject* r)
	{
		MetaObject* addr = machine->p_null;
		switch (l->type)
		{
		case MetaObject::NUMBER:
			if (r->type == MetaObject::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = CreateNumber(fmod(*lvalue, *rvalue));
			}
			break;
		}
		return addr;
	}
	MetaObject* VirtualThread::OperatePOW(MetaObject* l, MetaObject* r)
	{
		MetaObject* addr = machine->p_null;
		switch (l->type)
		{
		case MetaObject::NUMBER:
			if (r->type == MetaObject::NUMBER)
			{
				double* lvalue = (double*)l->content;
				double* rvalue = (double*)r->content;
				addr = CreateNumber(pow(*lvalue, *rvalue));
			}
			break;
		}
		return addr;
	}
	void VirtualThread::Execute()
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
					callback = 0;
					break;
				}
				machine->UnLock(machine->global[operand]);
				machine->Lock(machine->global[operand] = stack.back());
			}
			break;
			case OpCode::PUSHDWORD:
			{
				StackPush(CreateNumber(*(float*)cursor));
				cursor += 2;
			}
			break;
			case OpCode::PUSHQWORD:
			{
				StackPush(CreateNumber(*(double*)cursor));
				cursor += 4;
			}
			break;
			case OpCode::PUSHSTR:
			{
				uint16_t operand = (uint16_t)*cursor++;
				StackPush(CreateString(cursor, operand));
				cursor += operand / 2 + 1;
			}
			break;
			case OpCode::PUSHFUNC:
			{
				uint16_t operand = (uint16_t)*cursor++;
				StackPush(CreateFunction(cursor, operand));
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
				MetaObject* cond = stack.back();
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
				MetaObject* cond = stack.back();
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
				MetaObject* inf = stack[stack.size() - operand - 1];
				basestack.push_back(stack.size() - operand);
				switch (inf->type)
				{
				case MetaObject::FUNCTION:
					callstack.push_back(cursor);
					cursor = (uint16_t*)inf->content;
					break;
				case MetaObject::CFUNCTION:
					auto value = (*(CFunc*)inf->content)(std::vector<MetaObject*>(stack.begin() + basestack.back(), stack.begin() + stack.size()));
					StackPush(CopyObject(value));
					free(value);
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
					callback = 0;
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
					callback = 0;
					break;
				}
				cursor = callstack.back();
				callstack.pop_back();
				StackErase(basestack.back() - 1, stack.size());
				StackPush(machine->p_null);
				basestack.pop_back();
			}
			break;
			case OpCode::NEW:
			{
				uint16_t operand = (uint16_t)*cursor++;
				MetaObject* target = machine->global[operand];
				if(target->type != MetaObject::METADATA)
				{
					errors.push_back({"invalid operation"});
					callback = 0;
					break;
				}
				MetaObject* dest = machine->CreateHeader(MetaObject::OBJECT, sizeof(MetaObject*) * (stack.size() - basestack.back() -1), operand);
				MetaObject** content = (MetaObject**)dest->content;
				StackErase(basestack.back() - 1, stack.size());
				StackPush(dest);
			}
			break;
			case OpCode::INSTARR:
			{
				uint16_t operand = (uint16_t)*cursor++;
				MetaObject* addr = machine->CreateHeader(MetaObject::ARRAY, operand * sizeof(MetaObject* ));
				MetaObject**content = (MetaObject**)addr->content;
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
				MetaObject* addr = machine->CreateHeader(MetaObject::OBJECT, operand * sizeof(MetaObject* ) * 2);
				MetaObject**content = (MetaObject**)addr->content;
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
				StackPush(machine->CreateHeader(MetaObject::METADATA, operand * sizeof(OpCode), 0, cursor));
				cursor += operand;
			}
			break;
			case OpCode::ARRSET:
			{
				if (stack.size() < 3)
				{
					errors.push_back({"잘못된 호출", 0});
					callback = 0;
					break;
				}
				MetaObject* arr = StackBack(0);
				MetaObject* key = StackBack(1);
				MetaObject* value = StackBack(2);
				if (arr->type == MetaObject::ARRAY)
				{
					if (key->type != MetaObject::NUMBER)
					{
						errors.push_back({"인덱스 참조는 숫자만 가능합니다.", 0});
						callback = 0;
						break;
					}
					size_t index = static_cast<size_t>(*(double *)key->content);
					size_t size = arr->size / sizeof(MetaObject* );
					if (index >= size)
					{
						errors.push_back({"참조하려는 인덱스가 범위를 넘어섰습니다.", 0});
						callback = 0;
						break;
					}
					MetaObject** elements = (MetaObject**)arr->content;
					machine->UnLock(elements[index]);
					machine->Lock(value);
					elements[index] = value;
					StackPop(3);
					StackPush(elements[index]);
				}
				else if (arr->type == MetaObject::STRING)
				{
					if (key->type != MetaObject::NUMBER)
					{
						errors.push_back({"인덱스 참조는 숫자만 가능합니다.", 0});
						callback = 0;
						break;
					}
					size_t index = static_cast<size_t>(*(double *)key->content);
					if (index >= arr->size)
					{
						errors.push_back({"참조하려는 인덱스가 범위를 넘어섰습니다.", 0});
						callback = 0;
						break;
					}
					if (value->type != MetaObject::STRING)
					{
						errors.push_back({"문자만 대입가능합니다.", 0});
						callback = 0;
						break;
					}
					arr->content[index] = value->content[0];
					StackPop(3);
					StackPush(arr);
				}
				else
				{
					errors.push_back({"왼쪽 표현식은 인덱스 참조가 불가능합니다.", 0});
					callback = 0;
					break;
				}
			}
			break;
			case OpCode::ARRGET:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 호출.", 0});
					callback = 0;
					break;
				}
				MetaObject* arr = StackBack(0);
				MetaObject* key = StackBack(1);
				if (arr->type == MetaObject::ARRAY)
				{
					if (key->type != MetaObject::NUMBER)
					{
						errors.push_back({"인덱스 참조는 숫자만 가능합니다.", 0});
						callback = 0;
						break;
					}
					size_t index = static_cast<size_t>(*(double *)key->content);
					size_t size = arr->size / sizeof(MetaObject* );
					if (index >= size)
					{
						errors.push_back({"참조하려는 인덱스가 범위를 넘어섰습니다.", 0});
						callback = 0;
						break;
					}
					MetaObject** elements = (MetaObject**)arr->content;
					StackPop(2);
					StackPush(elements[index]);
				}
				else if (arr->type == MetaObject::STRING)
				{
					if (key->type != MetaObject::NUMBER)
					{
						errors.push_back({"인덱스 참조는 숫자만 가능합니다.", 0});
						callback = 0;
						break;
					}
					size_t index = static_cast<size_t>(*(double *)key->content);
					if (index >= arr->size)
					{
						errors.push_back({"참조하려는 인덱스가 범위를 넘어섰습니다.", 0});
						callback = 0;
						break;
					}
					StackPop(2);
					StackPush(CreateString(&arr->content[index], 1));
				}
				else
				{
					errors.push_back({"왼쪽 표현식은 인덱스 참조가 불가능합니다.", 0});
					callback = 0;
					break;
				}
			}
			break;
			case OpCode::REFSET:
			{
				if (stack.size() < 3)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = 0;
					break;
				}
				MetaObject* arr = StackBack(0);
				MetaObject* key = StackBack(1);
				MetaObject* value = StackBack(2);
				if (arr->type != MetaObject::OBJECT)
				{
					errors.push_back({"참조할 수 없는 객체입니다.", 0});
					callback = 0;
					break;
				}
				MetaObject** elements = (MetaObject**)arr->content;
				size_t size = arr->size / sizeof(MetaObject* ) / 2;
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
					callback = 0;
					break;
				}
				MetaObject* arr = StackBack(0);
				MetaObject* key = StackBack(1);
				if (arr->type != MetaObject::OBJECT)
				{
					errors.push_back({"참조할 수 없는 객체입니다.", 0});
					callback = 0;
					break;
				}
				MetaObject** elements = (MetaObject**)arr->content;
				size_t size = arr->size / sizeof(MetaObject* ) / 2;
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
					callback = 0;
					break;
				}
				MetaObject* operand = OperateNOT(stack.back());
				StackPop();
				StackPush(operand);
			}
			break;
			case OpCode::EQ:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = 0;
					break;
				}
				MetaObject* operand = OperateEQ(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::NEQ:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = 0;
					break;
				}
				MetaObject* operand = machine->p_null;
				MetaObject* r = stack_pop();
				MetaObject* l = stack_pop();
				switch (l->type)
				{
				case MetaObject::NUMBER:
					if (r->type == MetaObject::NUMBER)
					{
						double *lvalue = (double *)l->content;
						double *rvalue = (double *)r->content;
						operand = *lvalue != *rvalue ? machine->p_true : machine->p_false;
					}
					break;
				case MetaObject::STRING:
					if (r->type == MetaObject::STRING)
					{
						char *lvalue = (char *)l->content;
						char *rvalue = (char *)r->content;
						operand = strcmp(lvalue, rvalue) != 0 ? machine->p_true : machine->p_false;
					}
					break;
				case MetaObject::BOOLEAN:
					if (r->type == MetaObject::BOOLEAN)
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
					callback = 0;
					break;
				}
				MetaObject* operand = machine->p_null;
				MetaObject* r = stack_pop();
				MetaObject* l = stack_pop();
				switch (l->type)
				{
				case MetaObject::NUMBER:
					if (r->type == MetaObject::NUMBER)
					{
						double *lvalue = (double *)l->content;
						double *rvalue = (double *)r->content;
						operand = *lvalue > *rvalue ? machine->p_true : machine->p_false;
					}
					break;
				case MetaObject::STRING:
					if (r->type == MetaObject::STRING)
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
					callback = 0;
					break;
				}
				MetaObject* operand = OperateGE(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::LT:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = 0;
					break;
				}
				MetaObject* operand = OperateLT(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::LE:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = 0;
					break;
				}
				MetaObject* operand = OperateLE(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::OR:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = 0;
					break;
				}
				MetaObject* operand = OperateOR(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::AND:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = 0;
					break;
				}
				MetaObject* operand = OperateAND(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::SIGN:
			{
				if (stack.size() < 1)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = 0;
					break;
				}
				MetaObject* operand = OperateSIGN(stack.back());
				StackPop();
				StackPush(operand);
			}
			break;
			case OpCode::ADD:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = 0;
					break;
				}
				MetaObject* operand = OperateADD(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::SUB:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = 0;
					break;
				}
				MetaObject* operand = OperateSUB(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::MUL:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = 0;
					break;
				}
				MetaObject* operand = OperateMUL(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::DIV:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = 0;
					break;
				}
				MetaObject* operand = OperateDIV(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::MOD:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = 0;
					break;
				}
				MetaObject* operand = OperateMOD(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			case OpCode::POW:
			{
				if (stack.size() < 2)
				{
					errors.push_back({"잘못된 바이트코드입니다.", 0});
					callback = 0;
					break;
				}
				MetaObject* operand = OperatePOW(StackBack(1), StackBack(0));
				StackPop(2);
				StackPush(operand);
			}
			break;
			default:
				callback = 0;
				break;
			}
		}
		for (auto error : errors)
			std::cout << ToString(error).c_str() << std::endl;
	}
}