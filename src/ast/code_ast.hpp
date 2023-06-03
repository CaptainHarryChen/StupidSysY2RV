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
        res->name = new_char_arr("@" + ident);
        res->params = empty_koopa_rs(KOOPA_RSIK_VALUE);

        std::vector<const void *> blocks;
        symbol_list.SetBasicBlockBuf(&blocks);

        koopa_raw_basic_block_data_t *entry_block = new koopa_raw_basic_block_data_t();
        entry_block->name = new_char_arr("%entry_" + ident);
        entry_block->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
        entry_block->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);

        symbol_list.AddNewBasicBlock(entry_block);
        std::vector<const void *> entry_insts;
        block->build_koopa_values(entry_insts, empty_koopa_rs());
        entry_block->insts = make_koopa_rs_from_vector(entry_insts, KOOPA_RSIK_VALUE);

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

// AST code block, enclosed in '{}'. Different from koopa basic block
class BlockAST : public BaseAST
{
public:
    std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> insts;

    BlockAST() {}
    BlockAST(std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> &_insts)
    {
        for (auto &inst : _insts)
            insts.push_back(make_pair(inst.first, std::move(inst.second)));
    }

    void *build_koopa_values(std::vector<const void *> &buf, koopa_raw_slice_t parent) const override
    {
        symbol_list.NewEnv();
        for (auto &inst : insts)
        {
            switch (inst.first)
            {
            case ConstDecl:
                // TODO: used_by
                inst.second->to_koopa_item(empty_koopa_rs());
                break;
            case Decl:
                // TODO: used_by
                inst.second->build_koopa_values(buf, empty_koopa_rs());
                break;
            case Stmt:
                inst.second->build_koopa_values(buf, empty_koopa_rs());
                break;
            }
        }
        symbol_list.DeleteEnv();
        return nullptr;
    }
};

class ReturnAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> ret_num;
    ReturnAST(std::unique_ptr<BaseAST> &_ret_num)
    {
        ret_num = std::move(_ret_num);
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

class AssignmentAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> lval;
    std::unique_ptr<BaseAST> exp;
    AssignmentAST(std::unique_ptr<BaseAST> &_lval, std::unique_ptr<BaseAST> &_exp)
    {
        lval = std::move(_lval);
        exp = std::move(_exp);
    }
    void *build_koopa_values(std::vector<const void *> &buf, koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
        res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
        res->name = nullptr;
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_STORE;
        res->kind.data.store.value = (koopa_raw_value_t)exp->build_koopa_values(buf, child_used_by);
        res->kind.data.store.dest = (koopa_raw_value_t)lval->to_koopa_item(child_used_by);
        buf.push_back(res);
        return nullptr;
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
        res->name = new_char_arr("@" + name);
        res->used_by = parent;
        res->kind.tag = KOOPA_RVT_ALLOC;
        buf.push_back(res);
        symbol_list.AddSymbol(name, LValSymbol(LValSymbol::Var, res));

        if (exp)
        {
            koopa_raw_value_data *store = new koopa_raw_value_data();
            store->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
            store->name = nullptr;
            store->used_by = empty_koopa_rs();
            store->kind.tag = KOOPA_RVT_STORE;
            store->kind.data.store.dest = res;
            store->kind.data.store.value = (koopa_raw_value_t)exp->build_koopa_values(buf, child_used_by);
            buf.push_back(store);
        }

        return res;
    }
};