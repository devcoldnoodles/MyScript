#ifndef __VIRTUALTHREAD_H__
#define __VIRTUALTHREAD_H__

#include <vector>
#include "data.h"

namespace myscript
{
    struct MetaObject;
    class VirtualMachine;

    class VirtualThread
    {
    private:
        VirtualMachine *machine;
        uint16_t *cursor;
        std::vector<MetaObject *> stack;
        std::vector<uint16_t *> callstack;
        std::vector<size_t> basestack;
        std::vector<Error> errors;
        bool state;

    public:
        VirtualThread(VirtualMachine *_machine, uint16_t *_cursor);
        MetaObject *CreateHeader(const uint16_t _type, const uint32_t _size, const uint16_t _adinf = 0);
        MetaObject *CreateNull(void);
        MetaObject *CreateBool(const bool boolean);
        MetaObject *CreateNumber(const double number);
        MetaObject *CreateString(const void *str, const size_t str_size);
        MetaObject *CreateString(const void *str, const size_t str_size, const char *addit, const size_t addit_size);
        MetaObject *CreateFunction(const void *opcode, const size_t size);
        MetaObject *CreateObject(const void *addr, const size_t size);
        MetaObject *CopyObject(MetaObject *src);
        std::vector<MetaObject *> GetParameters();
        void Execute();

    private:
        MetaObject *OperateEQ(MetaObject *l, MetaObject *r);
        MetaObject *OperateNEQ(MetaObject *l, MetaObject *r);
        MetaObject *OperateGT(MetaObject *l, MetaObject *r);
        MetaObject *OperateGE(MetaObject *l, MetaObject *r);
        MetaObject *OperateLT(MetaObject *l, MetaObject *r);
        MetaObject *OperateLE(MetaObject *l, MetaObject *r);
        MetaObject *OperateNOT(MetaObject *l);
        MetaObject *OperateAND(MetaObject *l, MetaObject *r);
        MetaObject *OperateOR(MetaObject *l, MetaObject *r);
        MetaObject *OperateXOR(MetaObject *l, MetaObject *r);
        MetaObject *OperateSIGN(MetaObject *l);
        MetaObject *OperateADD(MetaObject *l, MetaObject *r);
        MetaObject *OperateSUB(MetaObject *l, MetaObject *r);
        MetaObject *OperateMUL(MetaObject *l, MetaObject *r);
        MetaObject *OperateDIV(MetaObject *l, MetaObject *r);
        MetaObject *OperateMOD(MetaObject *l, MetaObject *r);
        MetaObject *OperatePOW(MetaObject *l, MetaObject *r);

        void PrintError(Error error);
        void StackPush(MetaObject *operand);
        void StackPop(int loop = 1);
        MetaObject *stack_pop();
        MetaObject *StackBack(size_t index);
        void StackErase(size_t start, size_t end);
    };
}

#endif