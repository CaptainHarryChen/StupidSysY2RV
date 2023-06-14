#include "ast/ast.hpp"

BlockMaintainer BaseAST::block_maintainer;
SymbolList BaseAST::symbol_list;
LoopMaintainer BaseAST::loop_maintainer;

char *new_char_arr(std::string str)
{
    size_t n = str.length();
    char *res = new char[n + 1];
    str.copy(res, n + 1);
    res[n] = 0;
    return res;
}
