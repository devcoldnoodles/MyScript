#include "pch.h"
#include "bytecode.h"
#include "parser.h"
#include <ctime>
#include <fstream>

using namespace myscript;

int main(int argc, char** argv) {
	CompliationDesc cdesc;
	VirtualMachine* vm;
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
	cdesc.RegistCFunc("clock", [](VirtualThread* thread) {
		return thread->CreateNumber(clock());
	});
	cdesc.RegistCFunc("typeof", [](VirtualThread *thread) {
		auto params = thread->GetParameters();
		if (params.size() == 1)
		{
			std::string tpyeinfo = ToString(Object::Type(params[0]->type));
			return thread->CreateString(tpyeinfo.c_str(), tpyeinfo.size());
		}
		return thread->CreateNull();
	});
	cdesc.RegistCFunc("print", [](VirtualThread *thread) {
		auto params = thread->GetParameters();
		for (auto iter : params)
		{
			printf("%s", ToString(iter).c_str());
		}
		return thread->CreateNumber(params.size());
	});
	cdesc.RegistCFunc("scan", [](VirtualThread *thread) {
		char buf[2048];
		fgets(buf, 2047, stdin);
		return thread->CreateString(buf, strlen(buf));
	});
	cdesc.RegistCFunc("copy", [](VirtualThread *thread) {
		Object* source = thread->GetParameters()[0];
		Object* dest = thread->CreateHeader(source->type, source->size, source->adinf);
		memcpy_s(dest->content, dest->size, source->content, source->size);
		return dest;
	});
	cdesc.RegistCFunc("size", [](VirtualThread *thread) {
		return thread->CreateNumber(double(thread->GetParameters()[0]->size - sizeof(Object*)));
	});
	if (!SyntaxTree::ParseText(code, std::string(buffer)))
	{
		printf("[parsing error]\n");
		std::string temp;
		for (size_t index = 0; index < code.errors.size(); ++index)
			temp += ToString(code.errors[index]) + "\n";
		printf("%s", temp.c_str());
		goto ErrorHandle;
	}
	if (!code.CreateCode(&cdesc))
	{
		printf("[code generate error]\n");
		std::string temp;
		for (size_t index = 0; index < cdesc.errors.size(); ++index)
			temp += ToString(cdesc.errors[index]) + "\n";
		printf("%s", temp.c_str());
		goto ErrorHandle;
	}
	vm = new VirtualMachine(&cdesc);
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