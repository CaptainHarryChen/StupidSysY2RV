#pragma once

#include <map>

#include "ast/base_ast.hpp"
#include "koopa_util.hpp"

enum InstType
{
    ConstDecl,
    Decl,
    Stmt
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
    std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> insts;

    BlockAST() {}
    BlockAST(std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> &_insts)
    {
        for(auto &inst : _insts)
            insts.push_back(make_pair(inst.first, std::move(inst.second)));
    }

    void *to_koopa_item(koopa_raw_slice_t parent) const override
    {
        symbol_list.NewEnv();
        koopa_raw_basic_block_data_t *res = new koopa_raw_basic_block_data_t();
        res->name = "%entry";
        res->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);

        std::vector<const void *> stmts;
        for(auto &inst : insts)
        {
            switch(inst.first)
            {
            case ConstDecl:
                // TODO: used_by
                inst.second->to_koopa_item(empty_koopa_rs());
                break;
            case Decl:
                // TODO: used_by
                inst.second->build_koopa_values(stmts, empty_koopa_rs());
                break;
            case Stmt:
                inst.second->build_koopa_values(stmts, empty_koopa_rs());
                break;
            }
        }
        res->insts = make_koopa_rs_from_vector(stmts, KOOPA_RSIK_VALUE);

        symbol_list.DeleteEnv();
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
    void *build_koopa_values(std::vector<const void *> &buf, koopa_raw_slice_t parent) const override
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

class ConstDefAST : public BaseAST
{
public:
    std::string name;
    std::unique_ptr<BaseAST> exp;

    ConstDefAST(const char *_name, std::unique_ptr<BaseAST> &_exp)
        : name(_name)
    {
        exp = std::move(_exp);
    }

    void *to_koopa_item(koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
        res->name = nullptr;
        res->used_by = parent;
        res->kind.tag = KOOPA_RVT_INTEGER;
        res->kind.data.integer.value = exp->CalcValue();
        symbol_list.AddSymbol(name, LValSymbol(LValSymbol::Const, res));
        return res;
    }
};

class VarDefAST : public BaseAST
{
public:
    std::string name;
    std::unique_ptr<BaseAST> exp;

    VarDefAST(const char *_name)
        : name(_name)
    {
        exp = nullptr;
    }

    VarDefAST(const char *_name, std::unique_ptr<BaseAST> &_exp)
        : name(_name)
    {
        exp = std::move(_exp);
    }

    void *build_koopa_values(std::vector<const void *> &buf, koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
        res->ty = make_int_pointer_type();
        char *tname = new char(name.length() + 1);
        ("@" + name).copy(tname, sizeof(tname));
        res->name = tname;
        res->used_by = parent;
        res->kind.tag = KOOPA_RVT_ALLOC;
        buf.push_back(res);
        symbol_list.AddSymbol(name, LValSymbol(LValSymbol::Var, res));

        koopa_raw_value_data *store = new koopa_raw_value_data();
        store->name = nullptr;
        store->used_by = empty_koopa_rs();
        store->kind.tag = KOOPA_RVT_STORE;
        store->kind.data.store.dest = res;
        if(exp)
            store->kind.data.store.value = (koopa_raw_value_t)exp->build_koopa_values(buf, child_used_by);
        else
        {
            NumberAST zero(0);
            store->kind.data.store.value = (koopa_raw_value_t)zero.to_koopa_item(child_used_by);
        }
        buf.push_back(store);
        
        return res;
    }
};