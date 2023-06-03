#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <cassert>

#include "koopa_util.hpp"
#include "symbol_list.hpp"

char *new_char_arr(std::string str);

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

class NumberAST : public BaseAST
{
public:
    int val;
    NumberAST(int _val) { val = _val; }
    std::string to_string() const override
    {
        return "NumberAST { int " + std::to_string(val) + " }";
    }

    void *to_koopa_item(koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
        res->name = nullptr;
        res->used_by = parent;
        res->kind.tag = KOOPA_RVT_INTEGER;
        res->kind.data.integer.value = val;
        return res;
    }
    void *build_koopa_values(std::vector<const void *> &buf, koopa_raw_slice_t parent) const override
    {
        auto res = to_koopa_item(parent);
        // buf.push_back(res);
        return res;
    }
    int CalcValue() const override
    {
        return val;
    }
};
