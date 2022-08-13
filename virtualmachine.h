#ifndef __VIRTUALMACHINE_H__
#define __VIRTUALMACHINE_H__

#include <map>
#include <set>
#include <vector>
#include <string>
#include <cstdlib>

namespace myscript
{
    struct MetaObject;
    class VirtualThread;
    struct CompliationDesc;

    class VirtualMachine
    {
    private:
        char *memory;
        size_t capacity;
        std::multiset<MetaObject *> allocs;
        std::vector<MetaObject *> global;
        // std::map<VarDesc, MetaObject *, VarDescCompare> glob;
        std::vector<std::string> names;
        std::vector<uint16_t> codes;
        MetaObject *p_null;
        MetaObject *p_true;
        MetaObject *p_false;
        std::vector<VirtualThread *> threads;

    public:
        friend class VirtualThread;
        VirtualMachine(CompliationDesc *data);
        ~VirtualMachine();
        void Execute();
        void Lock(MetaObject *index);
        void UnLock(MetaObject *index);
        MetaObject *Allocate(const size_t alloc_size);
        size_t GetGlobalIndex(const std::string &id);
        MetaObject *GetGlobalValue(const std::string &id) { return global[GetGlobalIndex(id)]; }
        void SetGlobalValue(const size_t id, MetaObject *ref);
        MetaObject *CreateHeader(const uint16_t _type, const uint32_t _size, const uint16_t _adinf = 0);
        MetaObject *CreateHeader(const uint16_t _type, const uint32_t _size, const uint16_t _adinf, const void *_content);
        MetaObject *CopyObject(MetaObject *src);
        std::string Report();
    };
}

#endif