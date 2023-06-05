#pragma once

#include <map>

#include "ast/base_ast.hpp"
#include "koopa_util.hpp"

enum InstType
{
    ConstDecl,
    Decl,
    Stmt,
    Branch
};
typedef std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> InstSet;

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
        block_maintainer.SetBasicBlockBuf(&blocks);

        koopa_raw_basic_block_data_t *entry_block = new koopa_raw_basic_block_data_t();
        entry_block->name = new_char_arr("%entry_" + ident);
        entry_block->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
        entry_block->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);

        block_maintainer.AddNewBasicBlock(entry_block);
        block->build_koopa_values(empty_koopa_rs());
        block_maintainer.FinishCurrentBlock();

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
    InstSet insts;

    BlockAST() {}
    BlockAST(InstSet &_insts)
    {
        for (auto &inst : _insts)
            insts.push_back(std::make_pair(inst.first, std::move(inst.second)));
    }

    static void add_InstSet(const InstSet &insts)
    {
        symbol_list.NewEnv();
        for (const auto &inst : insts)
        {
            switch (inst.first)
            {
            case ConstDecl:
                // TODO: used_by
                inst.second->to_koopa_item(empty_koopa_rs());
                break;
            case Decl:
                // TODO: used_by
                inst.second->build_koopa_values(empty_koopa_rs());
                break;
            case Stmt:
                inst.second->build_koopa_values(empty_koopa_rs());
                break;
            case Branch:
                inst.second->build_koopa_values(empty_koopa_rs());
                break;
            }
        }
        symbol_list.DeleteEnv();
    }

    void *build_koopa_values(koopa_raw_slice_t parent) const override
    {
        add_InstSet(insts);
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
    void *build_koopa_values(koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
        res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
        res->name = nullptr;
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_RETURN;
        res->kind.data.ret.value = (const koopa_raw_value_data *)ret_num->build_koopa_values(child_used_by);
        block_maintainer.AddInst(res);
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
    void *build_koopa_values(koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
        res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
        res->name = nullptr;
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_STORE;
        res->kind.data.store.value = (koopa_raw_value_t)exp->build_koopa_values(child_used_by);
        res->kind.data.store.dest = (koopa_raw_value_t)lval->to_koopa_item(child_used_by);
        block_maintainer.AddInst(res);
        return nullptr;
    }
};

class BranchAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> exp;
    InstSet true_instset;
    InstSet false_instset;
    BranchAST(std::unique_ptr<BaseAST> &_exp, InstSet &_true_insts)
    {
        for (auto &inst : _true_insts)
            true_instset.push_back(std::make_pair(inst.first, std::move(inst.second)));
        exp = std::move(_exp);
    }
    BranchAST(std::unique_ptr<BaseAST> &_exp, InstSet &_true_insts, InstSet &_false_insts)
    {
        for (auto &inst : _true_insts)
            true_instset.push_back(std::make_pair(inst.first, std::move(inst.second)));
        for (auto &inst : _false_insts)
            false_instset.push_back(std::make_pair(inst.first, std::move(inst.second)));
        exp = std::move(_exp);
    }
    void *build_koopa_values(koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
        res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
        res->name = nullptr;
        res->used_by = parent;
        res->kind.tag = KOOPA_RVT_BRANCH;
        res->kind.data.branch.cond = (koopa_raw_value_t)exp->build_koopa_values(child_used_by);
        koopa_raw_basic_block_data_t *true_block = new koopa_raw_basic_block_data_t();
        koopa_raw_basic_block_data_t *false_block = new koopa_raw_basic_block_data_t();
        koopa_raw_basic_block_data_t *end_block = new koopa_raw_basic_block_data_t();
        res->kind.data.branch.true_bb = true_block;
        res->kind.data.branch.false_bb = false_block;
        res->kind.data.branch.true_args = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.data.branch.false_args = empty_koopa_rs(KOOPA_RSIK_VALUE);
        block_maintainer.AddInst(res);

        true_block->name = new_char_arr("%true");
        true_block->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
        true_block->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        block_maintainer.AddNewBasicBlock(true_block);
        BlockAST::add_InstSet(this->true_instset);
        block_maintainer.AddInst(JumpInst(end_block));
        
        false_block->name = new_char_arr("%false");
        false_block->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
        false_block->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        block_maintainer.AddNewBasicBlock(false_block);
        std::vector<const void *> false_insts;
        BlockAST::add_InstSet(this->false_instset);
        block_maintainer.AddInst(JumpInst(end_block));

        end_block->name = new_char_arr("%end");
        end_block->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
        end_block->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        block_maintainer.AddNewBasicBlock(end_block);
        
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

    void *build_koopa_values(koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
        res->ty = make_int_pointer_type();
        res->name = new_char_arr("@" + name);
        res->used_by = parent;
        res->kind.tag = KOOPA_RVT_ALLOC;
        block_maintainer.AddInst(res);
        symbol_list.AddSymbol(name, LValSymbol(LValSymbol::Var, res));

        if (exp)
        {
            koopa_raw_value_data *store = new koopa_raw_value_data();
            store->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
            store->name = nullptr;
            store->used_by = empty_koopa_rs();
            store->kind.tag = KOOPA_RVT_STORE;
            store->kind.data.store.dest = res;
            store->kind.data.store.value = (koopa_raw_value_t)exp->build_koopa_values(child_used_by);
            block_maintainer.AddInst(store);
        }

        return res;
    }
};