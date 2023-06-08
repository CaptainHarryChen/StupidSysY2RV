#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <cassert>

#include "koopa_util.hpp"
#include "symbol_list.hpp"
#include "block_maintainer.hpp"
#include "loop_maintainer.hpp"

char *new_char_arr(std::string str);

// 所有 AST 的基类
class BaseAST
{
public:
    static SymbolList symbol_list;
    static BlockMaintainer block_maintainer;
    static LoopMaintainer loop_maintainer;

    virtual ~BaseAST() = default;
    // 调试用字符串
    virtual std::string to_string() const
    {
        return "(Not Implement)";
    }
    // 输出koopa对象，并在全局环境添加各种信息
    virtual void *build_koopa_values() const
    {
        std::cerr << "Not Implement build_koopa_values" << std::endl;
        assert(false);
        return nullptr;
    }
    // 用于表达式AST求值
    virtual int CalcValue() const
    {
        std::cerr << "Not Implement build_koopa_values" << std::endl;
        assert(false);
        return 0;
    }
    // 返回该AST的左值（用于变量）
    virtual void *koopa_leftvalue() const
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

    void *build_koopa_values() const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
        res->name = nullptr;
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_INTEGER;
        res->kind.data.integer.value = val;
        return res;
    }
    int CalcValue() const override
    {
        return val;
    }
};
