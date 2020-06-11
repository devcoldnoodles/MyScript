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
	std::ifstream in("script.txt");
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
	FILE* fp = fopen("script.txt", "r"); // C:\\Users\\Administrator\\Desktop\\script.txt
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
	cdesc.Insert({"clock", VarDesc::CONST}, CreateMetaCFunction([](std::vector<MetaObject*> args){
		return CreateMetaNumber(clock());
	}));
	cdesc.Insert({"typeof", VarDesc::CONST}, CreateMetaCFunction([](std::vector<MetaObject*> args){
		return args.size() == 1 ? CreateMetaString(ToString(MetaObject::Type(args[0]->type))) : CreateMetaNull();
	}));

	cdesc.Insert({"print", VarDesc::CONST}, CreateMetaCFunction([](std::vector<MetaObject*> args) {
		for (auto arg : args)
			printf("%s", ToString(arg).c_str());
		return CreateMetaNull();
	}));
	cdesc.Insert({"scan", VarDesc::CONST}, CreateMetaCFunction([](std::vector<MetaObject*> args) {
		char buf[2048];
		if(buf != fgets(buf, 2047, stdin))
			return CreateMetaNull();
		size_t size = strlen(buf);
		buf[size - 1] = '\0';
		return CreateMetaString(buf, size - 1);
	}));
	cdesc.Insert({"copy", VarDesc::CONST}, CreateMetaCFunction([](std::vector<MetaObject*> args) {
		if(args.size() != 1)
			return CreateMetaNull();
		return CreateMetaObject((MetaObject::Type)args.front()->type, args.front()->adinf, args.front()->size, (void*)args.front()->content);
	}));
	cdesc.Insert({"size", VarDesc::CONST}, CreateMetaCFunction([](std::vector<MetaObject*> args) {
		if(args.size() != 1)
			return CreateMetaNull();
		return CreateMetaNumber(args[0]->size);
	}));
	if (!SyntaxTree::ParseText(code, buffer))
	{
		printf("[parsing error]\n");
		std::string temp;
		for (auto error : code.errors)
			temp += ToString(error) + '\n';
		printf("%s", temp.c_str());
		goto ErrorHandle;
	}
	if (!code.CreateCode(&cdesc))
	{
		printf("[code generate error]\n");
		std::string temp;
		for (auto error : cdesc.errors)
			temp += ToString(error) + '\n';
		printf("%s", temp.c_str());
		goto ErrorHandle;
	}
	vm = new VirtualMachine(&cdesc);
	vm->Execute();
	
	printf("\n");
	delete buffer;
	delete vm;
	return EXIT_SUCCESS;
ErrorHandle:
	delete buffer;
	return EXIT_FAILURE;
}