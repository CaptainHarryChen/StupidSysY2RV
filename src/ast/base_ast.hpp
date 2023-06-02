#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <cassert>

#include <koopa.h>

#include "symbol_list.hpp"

// 所有 AST 的基类
class BaseAST
{
public:
    static SymbolList symbol_list;

    virtual ~BaseAST() = default;
    virtual std::string to_string() const
    {
        return "(Not Implement)";
    }
    virtual void *to_koopa_item(koopa_raw_slice_t parent) const
    {
        std::cerr << "Not Implement to_koopa_item" << std::endl;
        assert(false);
        return nullptr;
    };
    virtual void *build_koopa_values(std::vector<const void *> &buf, koopa_raw_slice_t parent) const
    {
        std::cerr << "Not Implement build_koopa_values" << std::endl;
        assert(false);
        return nullptr;
    }
    virtual int CalcValue() const
    {
        std::cerr << "Not Implement build_koopa_values" << std::endl;
        assert(false);
        return 0;
    }
};