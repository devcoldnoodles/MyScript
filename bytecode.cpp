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
		return CreateMetaObject(MetaObject::BOOLEAN, boolean, 0);
	}

	MetaObject* CreateMetaNumber(const double number)
	{
		return CreateMetaObject(MetaObject::NUMBER, 0, sizeof(double), &number);
	}

	MetaObject* CreateMetaString(std::string str)
	{
		MetaObject* alloc = (MetaObject*)malloc(sizeof(MetaObject) + str.size() + 1);
		alloc->type = MetaObject::STRING;
		alloc->adinf = 0;
		alloc->size = str.size();
		memcpy(alloc->content, str.c_str(), str.size());
		alloc->content[str.size()] = '\0';
		return alloc;
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
			std::string temp = "[";
			MetaObject** element_content = (MetaObject**)object->content;
			size_t count = object->size / sizeof(MetaObject*);
			for (size_t index = 0; index < count - 1; ++index)
				temp += ToString(element_content[index]) + ", ";
			temp += ToString(element_content[count - 1]) + "]";
			return temp;
		}
		case MetaObject::OBJECT:
		{
			std::string temp = "{ ";
			MetaObject** element_content = (MetaObject**)object->content;
			size_t count = object->size / sizeof(MetaObject*) / 2;
			for (size_t index = 0; index < count - 1; ++index)
				temp += ToString(element_content[index * 2]) + " : " + ToString(element_content[index * 2 + 1]) + ",";
			temp += ToString(element_content[(count - 1) * 2]) + " : " + ToString(element_content[(count - 1) * 2 + 1]) + "}";
			return temp;
		}
		case MetaObject::COBJECT:
			return "wrapping cbject";
		default:
			return "error";
		}
	}
}