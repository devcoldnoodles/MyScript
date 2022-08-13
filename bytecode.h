#ifndef __BYTECODE_H__
#define __BYTECODE_H__

#include "pch.h"
#include "data.h"
#include <iostream>

namespace myscript
{
	constexpr auto FORMATSIZE = 64;
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
}
#endif