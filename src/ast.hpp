#pragma once

#include <memory>
#include <string>
#include <vector>

#include "koopa_util.hpp"

// 所有 AST 的基类
class BaseAST
{
public:
    virtual ~BaseAST() = default;
    virtual std::string to_string() const = 0;
    virtual void *to_koopa_item() const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST
{
public:
    // 用智能指针管理对象
    std::unique_ptr<BaseAST> func_def;

    CompUnitAST(std::unique_ptr<BaseAST> &_func_def)
    {
        func_def = std::move(_func_def);
    }

    std::string to_string() const override
    {
        return "CompUnitAST { " + func_def->to_string() + " }";
    }

    void *to_koopa_item() const override
    {
        return nullptr;
    }

    koopa_raw_program_t to_koopa_raw_program() const
    {
        std::vector<const void *> funcs;
        funcs.push_back(func_def->to_koopa_item());

        koopa_raw_program_t res;
        res.values = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res.funcs = make_koopa_rs_from_vector(funcs, KOOPA_RSIK_FUNCTION);

        return res;
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    FuncDefAST(std::unique_ptr<BaseAST> &_func_type, const char *_ident, std::unique_ptr<BaseAST> &_block)
        : ident(_ident)
    {
        func_type = std::move(_func_type);
        block = std::move(_block);
    }

    std::string to_string() const override
    {
        return "FuncDefAST { " + func_type->to_string() + ", " + ident + ", " + block->to_string() + " }";
    }

    void *to_koopa_item() const override
    {
        koopa_raw_function_data_t *res = new koopa_raw_function_data_t();

        koopa_raw_type_kind_t *ty = new koopa_raw_type_kind_t();
        ty->tag = KOOPA_RTT_FUNCTION;
        ty->data.function.params = empty_koopa_rs(KOOPA_RSIK_TYPE);
        ty->data.function.ret = (const struct koopa_raw_type_kind *)func_type->to_koopa_item();
        res->ty = ty;

        char *tname = new char(ident.length() + 1);
        ("@" + ident).copy(tname, sizeof(tname));
        res->name = tname;

        res->params = empty_koopa_rs(KOOPA_RSIK_VALUE);

        std::vector<const void *> blocks;
        blocks.push_back(block->to_koopa_item());
        res->bbs = make_koopa_rs_from_vector(blocks, KOOPA_RSIK_BASIC_BLOCK);

        return res;
    }
};

class FuncTypeAST : public BaseAST
{
public:
    std::string name;
    FuncTypeAST(const char *_name) : name(_name) {}
    std::string to_string() const override
    {
        return "FuncTypeAST { " + name + " }";
    }

    void *to_koopa_item() const override
    {
        if (name == "int")
            return simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
        return nullptr; // not implement
    }
};

class BlockAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> stmt;

    BlockAST(std::unique_ptr<BaseAST> &_stmt)
    {
        stmt = std::move(_stmt);
    }

    std::string to_string() const override
    {
        return "BlockAST { " + stmt->to_string() + " }";
    }

    void *to_koopa_item() const override
    {
        koopa_raw_basic_block_data_t *res = new koopa_raw_basic_block_data_t();
        res->name = "%entry";
        res->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        
        std::vector<const void *> stmts;
        stmts.push_back(stmt->to_koopa_item());
        res->insts = make_koopa_rs_from_vector(stmts, KOOPA_RSIK_VALUE);

        return res;
    }
};

class StmtAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> ret_num;
    StmtAST(std::unique_ptr<BaseAST> &_ret_num)
    {
        ret_num = std::move(_ret_num);
    }
    std::string to_string() const override
    {
        return "StmtAST { return, " + ret_num->to_string() + " }";
    }
    void *to_koopa_item() const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
        res->name = nullptr;
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_RETURN;
        res->kind.data.ret.value = (const koopa_raw_value_data*)ret_num->to_koopa_item();
        return res;
    }
};

class NumberAST : public BaseAST
{
public:
    int val;
    NumberAST(int _val) : val(_val) {}
    std::string to_string() const override
    {
        return "NumberAST { int " + std::to_string(val) + " }";
    }
    void *to_koopa_item() const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
        res->name = nullptr;
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_INTEGER;
        res->kind.data.integer.value = val;
        return res;
    }
};
