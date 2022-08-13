#ifndef __SCANNER_H__
#define __SCANNER_H__
#include <string>
#include <vector>
#include "token.h"

namespace myscript
{
    class Scanner
    {
    private:
        std::string src;
        size_t pos;
        size_t lines;

    public:
        static bool Tokenize(const char *src, std::vector<TokenDesc*> &result);
        static TokenDesc *Scan(std::string src);
        void Scan(TokenDesc *desc);
        short Advance(bool inc_lines = false);
        bool Advance(const char *dest);
        short Advance(char32_t cond, short true_sign, short false_sign);
        short GetSingleToken(TokenDesc *desc);
        TokenDesc *ScanString(TokenDesc *desc);
        TokenDesc *ScanNumber(TokenDesc *desc);
        TokenDesc *ScanChar(TokenDesc *desc);
    };
} // namespace script
#endif