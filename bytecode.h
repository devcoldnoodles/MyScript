#ifndef __BYTECODE_H__
#define __BYTECODE_H__

#include "pch.h"
#include "data.h"
#include <iostream>

#define FORMATSIZE 64

namespace myscript
{
	struct MetaObject
	{
		enum Type : uint16_t
		{
			NULLPTR = 0,
			REF = 1,
			BOOLEAN = 2,
			NUMBER = 4,
			STRING = 8,
			FUNCTION = 16,
			CFUNCTION = 32,
			ARRAY = 64,
			OBJECT = 128,
			COBJECT = 256,
			METADATA = 512,
		};
		uint16_t type;
		uint16_t adinf;
		uint32_t size;
		char content[0];
	};
	MetaObject* CreateMetaObject(MetaObject::Type _type, uint16_t _adinf, uint32_t _size);
	MetaObject* CreateMetaObject(MetaObject::Type _type, uint16_t _adinf, uint32_t _size, const void* _content);
	MetaObject* CreateMetaNull(void);
	MetaObject* CreateMetaBool(const bool boolean);
	MetaObject* CreateMetaNumber(const double number);
	MetaObject* CreateMetaString(std::string str);
	MetaObject* CreateMetaString(const char* str, const size_t size);
	MetaObject* CreateMetaString(const char* str, const size_t str_size, const char* addit, const size_t addit_size);
	MetaObject* CreateMetaFunction(const OpCode* src, const size_t size);
	MetaObject* CreateMetaCFunction(CFunc func);
	MetaObject* CreateMetaCObject(const void* addr, const size_t size);
	MetaObject* CreateMetaData(const void* data, const size_t size);
	MetaObject* CreateInstance();
	const std::string ToString(MetaObject::Type);
	const std::string ToString(MetaObject*);
	const std::string ToString(Error);

	class VirtualThread;
	class VirtualMachine
	{
	private:
		char* memory;
		size_t capacity;
		std::multiset<MetaObject*> allocs;
		std::vector<MetaObject*> global;
		std::map<VarDesc, MetaObject*, VarDescCompare> glob;
		std::vector<std::string> names;
		std::vector<uint16_t> codes;
		MetaObject* p_null;
		MetaObject* p_true;
		MetaObject* p_false;
		std::vector<VirtualThread*> threads;
	public:
		friend class VirtualThread;
		VirtualMachine(CompliationDesc* data);
		~VirtualMachine();
		void Execute();
		inline void Lock(MetaObject* index)
		{
			allocs.insert(index);
		}
		inline void UnLock(MetaObject* index)
		{
			auto iter = allocs.find(index);
			if (iter != allocs.end())
			{
				if (allocs.count(*iter) <= 1 && (*iter)->type == MetaObject::ARRAY)
				{
					MetaObject** content = (MetaObject**)(*iter)->content;
					size_t size = (*iter)->size / sizeof(MetaObject*);
					for (size_t index = 0; index < size; ++index)
						UnLock(content[index]);
				}
				allocs.erase(iter);
			}
		}
		inline MetaObject* Allocate(const size_t alloc_size)
		{
			if (capacity <= alloc_size)
				return nullptr;
			if (allocs.size() == 0)
				return (MetaObject*)memory;
			MetaObject* rear_index = (MetaObject*)memory;
			auto allocs_end = allocs.end();
			for (auto iter = allocs.begin(); iter != allocs_end; iter = allocs.upper_bound(*iter))
			{
				if (*iter - rear_index >= alloc_size + sizeof(MetaObject))
					return rear_index;
				rear_index = *iter + (*iter)->size + sizeof(MetaObject);
			}
			return rear_index;
		}
		inline size_t GetGlobalIndex(const std::string& id) { return distance(names.begin(), find(names.begin(), names.end(), id)); }
		inline MetaObject* GetGlobalValue(const std::string& id) { return global[GetGlobalIndex(id)]; }
		inline void SetGlobalValue(const size_t id, MetaObject* ref)
		{
			if (global[id] != ref)
			{
				UnLock(global[id]);
				Lock(global[id] = ref);
			}
		}
		inline MetaObject* CreateHeader(const uint16_t _type, const uint32_t _size, const uint16_t _adinf = 0)
		{
			MetaObject* addr = Allocate(_size);
			addr->type = _type;
			addr->size = _size;
			addr->adinf = _adinf;
			return addr;
		}
		inline MetaObject* CreateHeader(const uint16_t _type, const uint32_t _size, const uint16_t _adinf, const void* _content)
		{
			MetaObject* addr = Allocate(_size);
			addr->type = _type;
			addr->size = _size;
			addr->adinf = _adinf;
			memcpy(addr->content, _content, _size);
			return addr;
		}
		inline MetaObject* CopyObject(MetaObject* src)
		{
			return CreateHeader(src->type, src->size, src->adinf, src->content);
		}
		std::string Report()
		{
			std::string temp = "[report]\n";
			temp += "[Capacity] = " + std::to_string(capacity) + "\n";
			size_t usingMemory = 0;
			for (auto iter = allocs.begin(); iter != allocs.end(); iter = allocs.upper_bound(*iter))
				usingMemory += (*iter)->size;
			temp += "[Using Space] = " + std::to_string(usingMemory) + "\n";
			temp += "[Available Space] = " + std::to_string(capacity - usingMemory);
			return temp;
		}
	};
	class VirtualThread
	{
	private:
		VirtualMachine* machine;
		uint16_t* cursor;
		std::vector<MetaObject*> stack;
		std::vector<uint16_t*> callstack;
		std::vector<size_t> basestack;
		std::vector<Error> errors;
		bool state;
		MetaObject* OperateEQ(MetaObject* l, MetaObject* r);
		MetaObject* OperateNEQ(MetaObject* l, MetaObject* r);
		MetaObject* OperateGT(MetaObject* l, MetaObject* r);
		MetaObject* OperateGE(MetaObject* l, MetaObject* r);
		MetaObject* OperateLT(MetaObject* l, MetaObject* r);
		MetaObject* OperateLE(MetaObject* l, MetaObject* r);
		MetaObject* OperateNOT(MetaObject* l);
		MetaObject* OperateAND(MetaObject* l, MetaObject* r);
		MetaObject* OperateOR(MetaObject* l, MetaObject* r);
		MetaObject* OperateXOR(MetaObject* l, MetaObject* r);
		MetaObject* OperateSIGN(MetaObject* l);
		MetaObject* OperateADD(MetaObject* l, MetaObject* r);
		MetaObject* OperateSUB(MetaObject* l, MetaObject* r);
		MetaObject* OperateMUL(MetaObject* l, MetaObject* r);
		MetaObject* OperateDIV(MetaObject* l, MetaObject* r);
		MetaObject* OperateMOD(MetaObject* l, MetaObject* r);
		MetaObject* OperatePOW(MetaObject* l, MetaObject* r);
		void (*OnError)(Error) = [](Error error)
		{
			std::cout << ToString(error).c_str() << std::endl;
		};
		
		inline void StackPush(MetaObject* operand)
		{
			machine->Lock(operand);
			stack.push_back(operand);
		}
		inline void StackPop(int loop = 1)
		{
			for (int count = 0; count < loop; ++count)
			{
				machine->UnLock(stack.back());
				stack.pop_back();
			}
		}
		inline MetaObject* stack_pop()
		{
			MetaObject* temp = stack.back();
			machine->UnLock(temp);
			stack.pop_back();
			return temp;
		}
		inline MetaObject* StackBack(size_t index)
		{
			return stack[stack.size() - index - 1];
		}
		inline void StackErase(size_t start, size_t end)
		{
			size_t index = start;
			while (index < end)
				machine->UnLock(stack[index++]);
			stack.erase(stack.begin() + start, stack.begin() + end);
		}
	public:
		void Execute();
		VirtualThread(VirtualMachine* _machine, uint16_t* _cursor) : machine(_machine), cursor(_cursor)
		{
			stack.reserve(1024);
			callstack.reserve(256);
			basestack.reserve(256);
			basestack.push_back(0);
		}
		inline MetaObject* CreateHeader(const uint16_t _type, const uint32_t _size, const uint16_t _adinf = 0)
		{
			return machine->CreateHeader(_type, _size, _adinf);
		}
		inline MetaObject* CreateNull(void)
		{
			return machine->p_null;
		}
		inline MetaObject* CreateBool(const bool boolean)
		{
			return boolean ? machine->p_true : machine->p_false;
		}
		inline MetaObject* CreateNumber(const double number)
		{
			return machine->CreateHeader(MetaObject::NUMBER, sizeof(double), 0, &number);
		}
		inline MetaObject* CreateString(const void* str, const size_t str_size)
		{
			MetaObject* alloc = machine->CreateHeader(MetaObject::STRING, str_size + 1, 0, str);
			alloc->content[str_size] = '\0';
			return alloc;
		}
		inline MetaObject* CreateString(const void* str, const size_t str_size, const char* addit, const size_t addit_size)
		{
			MetaObject* alloc = machine->CreateHeader(MetaObject::STRING, str_size + addit_size + 1);
			if (alloc != nullptr)
			{
				char *content = (char *)alloc->content;
				memmove(content, str, str_size);
				memmove(content + str_size, addit, addit_size);
				content[str_size + addit_size] = '\0';
			}
			return alloc;
		}
		inline MetaObject* CreateFunction(const void* opcode, const size_t size)
		{
			return machine->CreateHeader(MetaObject::FUNCTION, sizeof(OpCode) * size, 0, opcode);
		}
		inline MetaObject* CreateObject(const void* addr, const size_t size)
		{
			return machine->CreateHeader(MetaObject::COBJECT, size, 0, addr);
		}
		inline MetaObject* CopyObject(MetaObject* src)
		{
			return machine->CreateHeader(src->type, src->size, src->adinf, src->content);
		}
		inline std::vector<MetaObject*> GetParameters()
		{
			return std::vector<MetaObject*>(stack.begin() + basestack.back(), stack.begin() + stack.size());
		}
	};
}
#endif