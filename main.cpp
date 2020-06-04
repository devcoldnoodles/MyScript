#include "pch.h"
#include "bytecode.h"
#include "parser.h"
#include <ctime>
#include <fstream>

using namespace myscript;

int main(int argc, char** argv) {
	CompliationDesc cdesc;
	VirtualMachine* vm;
	#ifdef __cplusplus
	std::ifstream in("D:\\script.txt");
	if(!in.is_open())
	{
		printf("Failed to find script");
		return EXIT_FAILURE;
	}
	in.seekg(0, std::ios::end);
    size_t size = in.tellg();
	char* buffer = new char[size + 1];
	if (!buffer)
	{
		printf("Failed to allocate buffer");
		return EXIT_FAILURE;
	}
	memset(buffer, 0, size + 1);
	in.seekg(0, std::ios::beg);
	in.read(buffer, size);
	buffer[size] = 0;
	in.close();
	//std::cout << buffer << std::endl;
	#else
	FILE* fp = fopen("D:\\script.txt", "r"); // C:\\Users\\Administrator\\Desktop\\script.txt
	if (!fp)
	{
		printf("Failed to find script");
		return EXIT_FAILURE;
	}
	char* buffer;
	long size;	
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	buffer = (char*)malloc(size + 1);
	if (!buffer)
	{
		printf("Failed to allocate buffer");
		return EXIT_FAILURE;
	}
	memset(buffer, 0, size + 1);
	fseek(fp, 0, SEEK_SET);
	fread(buffer, size, 1, fp);
	buffer[size] = 0;
	fclose(fp);
	//printf("%s\n", buffer);
	#endif
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
		size_t size = strlen(buf);
		buf[size - 1] = '\0';
		return thread->CreateString(buf, size - 1);
	});
	cdesc.RegistCFunc("copy", [](VirtualThread *thread) {
		Object* source = thread->GetParameters()[0];
		Object* dest = thread->CreateHeader(source->type, source->size, source->adinf);
		memcpy(dest->content, source->content, source->size);
		return dest;
	});
	cdesc.RegistCFunc("size", [](VirtualThread *thread) {
		return thread->CreateNumber(thread->GetParameters()[0]->size);
	});
	if (!SyntaxTree::ParseText(code, buffer))
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