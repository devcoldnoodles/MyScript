#ifndef __SCANNER_H__
#define __SCANNER_H__
#include <string>
#include "token.h"

namespace script
{
class Scanner
{
private:
    std::string src;
    size_t pos;
    size_t lines;
public:
    void Scan(TokenDesc* desc);
    ushort Advance(bool inc_lines = false);
    ushort GetSingleToken(TokenDesc* desc);
    TokenDesc* ScanString(TokenDesc* desc);
    TokenDesc* ScanNumber(TokenDesc* desc);
    TokenDesc* ScanChar(TokenDesc* desc);
};
} // namespace script
#endif