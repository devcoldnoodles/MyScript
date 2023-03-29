#include "virtualmachine.h"
#include "virtualthread.h"
#include "bytecode.h"
#include <string.h>

using namespace myscript;

VirtualMachine::VirtualMachine(CompliationDesc *data) : codes(data->code)
{
    memory = (char *)malloc(capacity = 1024 * 1024);
    Lock(p_null = CreateHeader(MetaObject::NULLPTR, 0));
    Lock(p_true = CreateHeader(MetaObject::BOOLEAN, 0, 1));
    Lock(p_false = CreateHeader(MetaObject::BOOLEAN, 0));
    for (auto iter : data->global)
    {
        names.push_back(iter.name);
        global.push_back(p_null);
        Lock(p_null);
    }
    for (auto iter : data->globals)
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

void VirtualMachine::Execute()
{
    auto thread = new VirtualThread(this, &codes[0]);
    threads.push_back(thread);
    thread->Execute();
}

void VirtualMachine::Lock(MetaObject *index)
{
    allocs.insert(index);
}

void VirtualMachine::UnLock(MetaObject *index)
{
    auto iter = allocs.find(index);
    if (iter != allocs.end())
    {
        if (allocs.count(*iter) <= 1 && (*iter)->type == MetaObject::ARRAY)
        {
            MetaObject **content = (MetaObject **)(*iter)->content;
            size_t size = (*iter)->size / sizeof(MetaObject *);
            for (size_t index = 0; index < size; ++index)
                UnLock(content[index]);
        }
        allocs.erase(iter);
    }
}

MetaObject *VirtualMachine::Allocate(const size_t alloc_size)
{
    if (capacity <= alloc_size)
        return nullptr;
    if (allocs.size() == 0)
        return (MetaObject *)memory;
    MetaObject *rear_index = (MetaObject *)memory;
    auto allocs_end = allocs.end();
    for (auto iter = allocs.begin(); iter != allocs_end; iter = allocs.upper_bound(*iter))
    {
        if (*iter - rear_index >= alloc_size + sizeof(MetaObject))
            return rear_index;
        rear_index = *iter + (*iter)->size + sizeof(MetaObject);
    }
    return rear_index;
}

size_t VirtualMachine::GetGlobalIndex(const std::string &id)
{
    return distance(names.begin(), find(names.begin(), names.end(), id));
}

void VirtualMachine::SetGlobalValue(const size_t id, MetaObject *ref)
{
    if (global[id] != ref)
    {
        UnLock(global[id]);
        Lock(global[id] = ref);
    }
}

MetaObject *VirtualMachine::CreateHeader(const uint16_t _type, const uint32_t _size, const uint16_t _adinf)
{
    MetaObject *addr = Allocate(_size);
    addr->type = _type;
    addr->size = _size;
    addr->adinf = _adinf;
    return addr;
}

MetaObject *VirtualMachine::CreateHeader(const uint16_t _type, const uint32_t _size, const uint16_t _adinf, const void *_content)
{
    MetaObject *addr = Allocate(_size);
    addr->type = _type;
    addr->size = _size;
    addr->adinf = _adinf;
    memcpy(addr->content, _content, _size);
    return addr;
}

MetaObject *VirtualMachine::CopyObject(MetaObject *src)
{
    return CreateHeader(src->type, src->size, src->adinf, src->content);
}

std::string VirtualMachine::Report()
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