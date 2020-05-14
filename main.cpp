#include "pch.h"
#include "bytecode.h"
#include "parser.h"
#include <ctime>
#include <fstream>

using namespace myscript;

int main(int argc, char** argv) {
	CompliationData cdata;
	ADRMemory* vm;
	static bool script_loop = true;
	FILE* fp = fopen("D:\\script.txt", "a+");
	if (!fp)
	{
		printf("Failed to find script");
		return EXIT_FAILURE;
	}
	char* buffer;
	long buf_size;
	fseek(fp, 0, SEEK_END);
	buf_size = ftell(fp);
	buffer = (char*)malloc(buf_size + 1);
	if (!buffer)
	{
		printf("Failed to allocate buffer");
		return EXIT_FAILURE;
	}
	fseek(fp, 0, SEEK_SET);
	fread(buffer, buf_size, 1, fp);
	buffer[buf_size] = '\0';
	fclose(fp);
	SyntaxTree code;
	cdata.RegistCFunc("clock", [](ADRThread* thread) {
		return thread->CreateNumber(clock());
	});
	cdata.RegistCFunc("typeof", [](ADRThread *thread) {
		auto params = thread->GetParameters();
		if (params.size() == 1)
		{
			string tpyeinfo = ToString(Object::Type(params[0]->type));
			return thread->CreateString(tpyeinfo.c_str(), tpyeinfo.size());
		}
		return thread->CreateNull();
	});
	cdata.RegistCFunc("print", [](ADRThread *thread) {
		auto params = thread->GetParameters();
		for (auto iter : params)
		{
			printf("%s", ToString(iter).c_str());
		}
		return thread->CreateNumber(params.size());
	});
	cdata.RegistCFunc("scan", [](ADRThread *thread) {
		char buf[2048];
		fgets(buf, 2047, stdin);
		return thread->CreateString(buf, strlen(buf));
	});
	cdata.RegistCFunc("copy", [](ADRThread *thread) {
		Object* source = thread->GetParameters()[0];
		Object* dest = thread->CreateHeader(source->type, source->size, source->adinf);
		memcpy_s(dest->content, source->size, source->content, source->size);
		return dest;
	});
	cdata.RegistCFunc("size", [](ADRThread *thread) {
		return thread->CreateNumber(double(thread->GetParameters()[0]->size - sizeof(Object*)));
	});
	cdata.RegistCFunc("exit", [](ADRThread* thread) {
		script_loop = false;
		return thread->CreateNull();
	});
	if (!SyntaxTree::ParseText(code, string(buffer)))
	{
		printf("[parsing error]\n");
		string temp;
		size_t error_size = code.errors.size();
		for (size_t index = 0; index < error_size; ++index)
			temp += ToString(code.errors[index]) + "\n";
		printf("%s", temp.c_str());
		goto ErrorHandle;
	}
	if (!code.CreateCode(&cdata))
	{
		printf("[code generate error]\n");
		string temp;
		size_t error_size = cdata.errors.size();
		for (size_t index = 0; index < error_size; ++index)
			temp += ToString(cdata.errors[index]) + "\n";
		printf("%s", temp.c_str());
		goto ErrorHandle;
	}
	vm = new ADRMemory(cdata);
	vm->Execute();
	// while (script_loop)
	// {
	// 	fgets(buffer, buf_size - 1, stdin);
	// 	code.sents.clear();
	// 	cdata.Reset();
	// 	if (!SyntaxTree::ParseText(code, string(buffer)))
	// 	{
	// 		printf("%s\n", code.ErrorLog().c_str());
	// 		continue;
	// 	}
	// 	if (!code.CreateCode(cdata))
	// 	{
	// 		for (auto& error : cdata.errors)
	// 			printf("%s\n", error.ToString().c_str());
	// 		continue;
	// 	}
	// 	delete vm;
	// 	delete tr;
	// 	vm = new ADRMemory(cdata);
	// 	tr = vm->GetMainThread();
	// 	tr->Run();
	// }
	delete buffer;
	delete vm;
	return EXIT_SUCCESS;
ErrorHandle:
	delete buffer;
	return EXIT_FAILURE;
}