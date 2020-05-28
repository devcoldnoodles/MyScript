#ifndef __BYTECODE_H__
#define __BYTECODE_H__

#include"pch.h"
#include"data.h"

namespace myscript
{
	constexpr auto FORMATSIZE = 64;
	struct Object
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
	const std::string ToString(Object::Type);
	const std::string ToString(Object*);
	const std::string ToString(Error);

	class VirtualMachine
	{
	private:
		char* memory;
		size_t capacity;
		std::multiset<Object*> allocs;
		std::vector<Object*> global;
		std::map<std::string, Object*> glob;
		std::vector<std::string> names;
		std::vector<uint16_t> codes;
		Object* p_null;
		Object* p_true;
		Object* p_false;
		std::vector<VirtualThread*> threads;
	public:
		friend class VirtualThread;
		VirtualMachine(CompliationDesc* data);
		~VirtualMachine();
		void Execute();
		inline void Lock(Object* index)
		{
			allocs.insert(index);
		}
		inline void UnLock(Object* index)
		{
			auto iter = allocs.find(index);
			if (iter != allocs.end())
			{
				if (allocs.count(*iter) <= 1 && (*iter)->type == Object::ARRAY)
				{
					Object** content = (Object**)(*iter)->content;
					size_t size = (*iter)->size / sizeof(Object*);
					for (size_t index = 0; index < size; ++index)
						UnLock(content[index]);
				}
				allocs.erase(iter);
			}
		}
		inline Object* Allocate(const size_t alloc_size)
		{
			if (capacity <= alloc_size)
				return nullptr;
			if (allocs.size() == 0)
				return (Object*)memory;
			Object* rear_index = (Object*)memory;
			auto allocs_end = allocs.end();
			for (auto iter = allocs.begin(); iter != allocs_end; iter = allocs.upper_bound(*iter))
			{
				if (*iter - rear_index >= alloc_size + sizeof(Object))
					return rear_index;
				rear_index = *iter + (*iter)->size + sizeof(Object);
			}
			return rear_index;
		}
		inline size_t GetGlobalIndex(const std::string& id) { return distance(names.begin(), find(names.begin(), names.end(), id)); }
		inline Object* GetGlobalValue(const std::string& id) { return global[GetGlobalIndex(id)]; }
		inline void SetGlobalValue(const size_t id, Object* ref)
		{
			if (global[id] != ref)
			{
				UnLock(global[id]);
				Lock(global[id] = ref);
			}
		}
		inline Object* CreateHeader(const uint16_t _type, const uint32_t _size, const uint16_t _adinf = 0)
		{
			Object* addr = Allocate(_size);
			addr->type = _type;
			addr->size = _size;
			addr->adinf = _adinf;
			return addr;
		}
		inline Object* CreateHeader(const uint16_t _type, const uint32_t _size, const uint16_t _adinf, const void* _content)
		{
			Object* addr = Allocate(_size);
			addr->type = _type;
			addr->size = _size;
			addr->adinf = _adinf;
			memcpy(addr->content, _content, _size);
			return addr;
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
		std::vector<Object*> stack;
		std::vector<uint16_t*> callstack;
		std::vector<size_t> basestack;
		std::vector<Error> errors;
		bool state;
		Object* OperateEQ(Object* l, Object* r);
		Object* OperateNEQ(Object* l, Object* r);
		Object* OperateGT(Object* l, Object* r);
		Object* OperateGE(Object* l, Object* r);
		Object* OperateLT(Object* l, Object* r);
		Object* OperateLE(Object* l, Object* r);
		Object* OperateNOT(Object* l);
		Object* OperateAND(Object* l, Object* r);
		Object* OperateOR(Object* l, Object* r);
		Object* OperateXOR(Object* l, Object* r);
		Object* OperateSIGN(Object* l);
		Object* OperateADD(Object* l, Object* r);
		Object* OperateSUB(Object* l, Object* r);
		Object* OperateMUL(Object* l, Object* r);
		Object* OperateDIV(Object* l, Object* r);
		Object* OperateMOD(Object* l, Object* r);
		Object* OperatePOW(Object* l, Object* r);
		
		inline void StackPush(Object* operand)
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
		inline Object* stack_pop()
		{
			Object* temp = stack.back();
			machine->UnLock(temp);
			stack.pop_back();
			return temp;
		}
		inline Object* StackBack(size_t index)
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
		inline Object* CreateHeader(const uint16_t _type, const uint32_t _size, const uint16_t _adinf = 0)
		{
			return machine->CreateHeader(_type, _size, _adinf);
		}
		inline Object* CreateNull(void)
		{
			return machine->p_null;
		}
		inline Object* CreateBool(const bool boolean)
		{
			return boolean ? machine->p_true : machine->p_false;
		}
		inline Object* CreateNumber(const double number)
		{
			return machine->CreateHeader(Object::NUMBER, sizeof(double), 0, &number);
		}
		inline Object* CreateString(const char* str, const size_t str_size)
		{
			Object* alloc = machine->CreateHeader(Object::STRING, str_size + 1, 0, str);
			alloc->content[str_size] = '\0';
			return alloc;
		}
		inline Object* CreateString(const char* str, const size_t str_size, const char* addit, const size_t addit_size)
		{
			Object* alloc = machine->CreateHeader(Object::STRING, str_size + addit_size + 1);
			if (alloc != nullptr)
			{
				char *content = (char *)alloc->content;
				memmove(content, str, str_size);
				memmove(content + str_size, addit, addit_size);
				content[str_size + addit_size] = '\0';
			}
			return alloc;
		}
		inline Object* CreateFunction(const OpCode *opcode, const size_t size)
		{
			return machine->CreateHeader(Object::FUNCTION, sizeof(OpCode) * size, 0, opcode);
		}
		inline Object* CreateObject(const void *addr, const size_t size)
		{
			return machine->CreateHeader(Object::COBJECT, size, 0, addr);
		}
		inline std::vector<Object*> GetParameters()
		{
			return std::vector<Object*>(stack.begin() + basestack.back(), stack.begin() + stack.size());
		}
	};
}
#endif