#pragma once

#include <memory>
#include <string>
#include <vector>
#include <cassert>
#include <iostream>

#include "koopa_util.hpp"

class BaseASt;
class CompUnitAST;
class FuncDefAST;
class FuncTypeAST;
class BlockAST;
class StmtAST;
class ExpAST;
class NumberAST;
class PrimaryExpAST;
class UnaryExpAST;


// 所有 AST 的基类
class BaseAST
{
public:
    virtual ~BaseAST() = default;
    virtual void pass_property(void *pro) = 0;
    virtual std::string to_string() const
    {
        return "(Not Implement)";
    }
    virtual void *to_koopa_item(koopa_raw_slice_t parent) const 
    {
        std::cerr<<"Not Implement to_koopa_item"<<std::endl;
        assert(false);
        return nullptr;
    };
    virtual void *build_koopa_values(std::vector<const void*> &buf, koopa_raw_slice_t parent) const 
    {
        std::cerr<<"Not Implement build_koopa_values"<<std::endl;
        assert(false);
        return nullptr;
    }
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

    void pass_property(void *pro) override
    {
        func_def->pass_property(nullptr);
    }

    std::string to_string() const override
    {
        return "CompUnitAST { " + func_def->to_string() + " }";
    }

    void *to_koopa_item(koopa_raw_slice_t parent) const override
    {
        return nullptr;
    }

    koopa_raw_program_t to_koopa_raw_program() const
    {
        std::vector<const void *> funcs;
        funcs.push_back(func_def->to_koopa_item(empty_koopa_rs()));

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

    void pass_property(void * pro) override
    {
        func_type->pass_property(nullptr);
        block->pass_property(nullptr);
    }

    std::string to_string() const override
    {
        return "FuncDefAST { " + func_type->to_string() + ", " + ident + ", " + block->to_string() + " }";
    }

    void *to_koopa_item(koopa_raw_slice_t parent) const override
    {
        koopa_raw_function_data_t *res = new koopa_raw_function_data_t();

        koopa_raw_type_kind_t *ty = new koopa_raw_type_kind_t();
        ty->tag = KOOPA_RTT_FUNCTION;
        ty->data.function.params = empty_koopa_rs(KOOPA_RSIK_TYPE);
        ty->data.function.ret = (const struct koopa_raw_type_kind *)func_type->to_koopa_item(empty_koopa_rs());
        res->ty = ty;

        char *tname = new char(ident.length() + 1);
        ("@" + ident).copy(tname, sizeof(tname));
        res->name = tname;

        res->params = empty_koopa_rs(KOOPA_RSIK_VALUE);

        std::vector<const void *> blocks;
        blocks.push_back(block->to_koopa_item(empty_koopa_rs()));
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

    void pass_property(void *pro) override {}

    void *to_koopa_item(koopa_raw_slice_t parent) const override
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

    void pass_property(void *pro) override
    {
        stmt->pass_property(nullptr);
    }

    std::string to_string() const override
    {
        return "BlockAST { " + stmt->to_string() + " }";
    }

    void *to_koopa_item(koopa_raw_slice_t parent) const override
    {
        koopa_raw_basic_block_data_t *res = new koopa_raw_basic_block_data_t();
        res->name = "%entry";
        res->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);

        std::vector<const void *> stmts;
        stmt->build_koopa_values(stmts, empty_koopa_rs());
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
    void pass_property(void *pro) override {}
    std::string to_string() const override
    {
        return "StmtAST { return, " + ret_num->to_string() + " }";
    }
    void *build_koopa_values(std::vector<const void*> &buf, koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
        res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
        res->name = nullptr;
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_RETURN;
        res->kind.data.ret.value = (const koopa_raw_value_data *)ret_num->build_koopa_values(buf, child_used_by);
        buf.push_back(res);
        return res;
    }
};

class ExpAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> unaryExp;

    ExpAST(std::unique_ptr<BaseAST> &_unaryExp)
    {
        unaryExp = std::move(_unaryExp);
    }

    void pass_property(void *pro) override
    {
        std::shared_ptr<BaseAST> lastExp = nullptr;
        unaryExp->pass_property(&lastExp);
    }

    void *build_koopa_values(std::vector<const void*> &buf, koopa_raw_slice_t parent) const override
    {
        return unaryExp->build_koopa_values(buf, parent);
    }
};

class NumberAST : public BaseAST
{
public:
    int val;
    NumberAST(int _val)  {val = _val;}
    void pass_property(void *pro) override {}
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
    void *build_koopa_values(std::vector<const void*> &buf, koopa_raw_slice_t parent) const override
    {
        auto res = to_koopa_item(parent);
        //buf.push_back(res);
        return res;
    }
};

class PrimaryExpAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> nextExp; // Exp or Number

    void pass_property(void *pro) override
    {
        nextExp->pass_property(nullptr);
    }

    PrimaryExpAST(std::unique_ptr<BaseAST> &_nextExp)
    {
        nextExp = std::move(_nextExp);
    }
    void *build_koopa_values(std::vector<const void*> &buf, koopa_raw_slice_t parent) const override
    {
        return nextExp->build_koopa_values(buf, parent);
    }
};

class UnaryExpAST : public BaseAST
{
public:
    enum
    {
        Primary,
        Op
    } type;
    std::string op;
    std::shared_ptr<BaseAST> lastExp;
    std::unique_ptr<BaseAST> nextExp; // PrimaryExp or UnaryExp

    UnaryExpAST(std::unique_ptr<BaseAST> &_primary_exp)
    {
        type = Primary;
        nextExp = std::move(_primary_exp);
    }
    UnaryExpAST(const char *_op, std::unique_ptr<BaseAST> &_unary_exp)
    {
        type = Op;
        op = std::string(_op);
        nextExp = std::move(_unary_exp);
    }

    void pass_property(void *pro) override
    {
        lastExp = *(std::shared_ptr<BaseAST>*)pro;
        nextExp->pass_property(nullptr);
    }
    
    void *build_koopa_values(std::vector<const void*> &buf, koopa_raw_slice_t parent) const override
    {
        NumberAST zero(0);
        koopa_raw_value_data *res = nullptr;
        switch(type)
        {
        case Primary:
            res = (koopa_raw_value_data*)nextExp->build_koopa_values(buf, parent);
            break;
        case Op:
            res = new koopa_raw_value_data();
            koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
            res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
            res->name = nullptr;
            res->used_by = parent;
            res->kind.tag = KOOPA_RVT_BINARY;
            auto &binary = res->kind.data.binary;
            if(op == "+")
            {
                binary.op = KOOPA_RBO_ADD;
                binary.lhs = lastExp ? (koopa_raw_value_t)lastExp->build_koopa_values(buf, child_used_by) 
                        : (koopa_raw_value_t)zero.build_koopa_values(buf, child_used_by);
                binary.rhs = (koopa_raw_value_t)nextExp->build_koopa_values(buf, child_used_by);
            }
            else if(op == "-")
            {
                binary.op = KOOPA_RBO_SUB;
                binary.lhs = lastExp ? (koopa_raw_value_t)lastExp->build_koopa_values(buf, child_used_by) 
                        : (koopa_raw_value_t)zero.build_koopa_values(buf, child_used_by);
                binary.rhs = (koopa_raw_value_t)nextExp->build_koopa_values(buf, child_used_by);
            }
            else if(op == "!")
            {
                binary.op = KOOPA_RBO_EQ;
                binary.lhs = lastExp ? (koopa_raw_value_t)lastExp->build_koopa_values(buf, child_used_by) 
                        : (koopa_raw_value_t)zero.build_koopa_values(buf, child_used_by);
                binary.rhs = (koopa_raw_value_t)nextExp->build_koopa_values(buf, child_used_by);
            }
            buf.push_back(res);
            break;
        }
        return res;
    }
};