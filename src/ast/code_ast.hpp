#pragma once

#include <map>

#include "ast/base_ast.hpp"
#include "koopa_util.hpp"

enum InstType
{
    ConstDecl,
    Decl,
    ArrayDecl,
    Stmt,
    Branch,
    While,
    Break,
    Continue
};
typedef std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>> InstSet;

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST
{
    void add_lib_funcs(std::vector<const void*> &funcs) const;

public:
    // 用智能指针管理对象
    std::vector<std::unique_ptr<BaseAST>> func_list;
    InstSet value_list;

    CompUnitAST(std::vector<BaseAST*> &_func_list, InstSet &_value_list);

    koopa_raw_program_t to_koopa_raw_program() const
    {
        symbol_list.NewEnv();
        std::vector<const void *> values;
        std::vector<const void *> funcs;
        add_lib_funcs(funcs);
        for(auto &pa : value_list)
        {
            assert(pa.first == ConstDecl || pa.first == Decl || pa.first == ArrayDecl);
            if(pa.first == ConstDecl)
                pa.second->build_koopa_values();
            else
                values.push_back(pa.second->build_koopa_values());
        }
        for(auto &func_ast : func_list)
            funcs.push_back(func_ast->build_koopa_values());
        symbol_list.DeleteEnv();

        koopa_raw_program_t res;
        res.values = make_koopa_rs_from_vector(values, KOOPA_RSIK_VALUE);
        res.funcs = make_koopa_rs_from_vector(funcs, KOOPA_RSIK_FUNCTION);

        return res;
    }
};

class BTypeAST : public BaseAST
{
public:
    std::string name;
    BTypeAST(const char *_name) : name(_name) {}

    void *build_koopa_values() const override
    {
        if (name == "int")
            return simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
        else if(name == "void")
            return simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
        return nullptr; // not implement
    }
};

class FuncFParamAST : public BaseAST
{
public:
    enum ParamType
    {
        Int,
        Array
    } type;
    std::string name;
    int index;
    std::vector<std::unique_ptr<BaseAST>> sz_exp;

    FuncFParamAST(ParamType _type, const char *_name, int _index) : type(_type), name(_name), index(_index) {}
    FuncFParamAST(ParamType _type, const char *_name, int _index, std::vector<BaseAST*> &_sz_Exp) : type(_type), name(_name), index(_index)
    {
        for(auto e : _sz_Exp)
            sz_exp.emplace_back(e);
    }

    void *get_koopa_type() const
    {
        if(type == Array)
        {
            if(sz_exp.empty())
                return make_int_pointer_type();
            else
            {
                std::vector<int> sz;
                for(auto &e : sz_exp)
                    sz.push_back(e->CalcValue());
                koopa_raw_type_kind *ty = make_array_type(sz);
                koopa_raw_type_kind *tty = new koopa_raw_type_kind();
                tty->tag = KOOPA_RTT_POINTER;
                tty->data.pointer.base = ty;
                return tty;
            }
        }
        else if (type == Int)
            return simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
        return nullptr;
    }

    void *build_koopa_values() const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        res->ty = (koopa_raw_type_kind*)get_koopa_type();
        res->name = new_char_arr("@" + name);
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_FUNC_ARG_REF;
        res->kind.data.func_arg_ref.index = index;
        return res;
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
            inst.second->build_koopa_values();
        symbol_list.DeleteEnv();
    }

    void build_koopa_values_no_env() const
    {
        for (const auto &inst : insts)
            inst.second->build_koopa_values();
    }

    void *build_koopa_values() const override
    {
        add_InstSet(insts);
        return nullptr;
    }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::vector<std::unique_ptr<FuncFParamAST>> fparams;
    std::unique_ptr<BlockAST> block;

    FuncDefAST(std::unique_ptr<BaseAST> &_func_type, const char *_ident, std::vector<BaseAST*> &_fparams, std::unique_ptr<BaseAST> &_block)
        : ident(_ident)
    {
        func_type = std::move(_func_type);
        for(BaseAST* fp : _fparams)
            fparams.emplace_back(dynamic_cast<FuncFParamAST*>(fp));
        block = std::unique_ptr<BlockAST>(dynamic_cast<BlockAST*>(_block.release()));
    }

    std::string to_string() const override
    {
        return "FuncDefAST { " + func_type->to_string() + ", " + ident + ", " + block->to_string() + " }";
    }

    void *build_koopa_values() const override
    {
        koopa_raw_function_data_t *res = new koopa_raw_function_data_t();
        symbol_list.AddSymbol(ident, LValSymbol(LValSymbol::Function, res));

        koopa_raw_type_kind_t *ty = new koopa_raw_type_kind_t();
        ty->tag = KOOPA_RTT_FUNCTION;
        std::vector<const void*> par;
        for(auto &fp : fparams)
            par.push_back(fp->get_koopa_type());
        ty->data.function.params = make_koopa_rs_from_vector(par, KOOPA_RSIK_TYPE);
        ty->data.function.ret = (const struct koopa_raw_type_kind *)func_type->build_koopa_values();
        res->ty = ty;
        res->name = new_char_arr("@" + ident);
        par.clear();
        for(auto &fp : fparams)
            par.push_back(fp->build_koopa_values());
        res->params = make_koopa_rs_from_vector(par, KOOPA_RSIK_VALUE);

        std::vector<const void *> blocks;
        block_maintainer.SetBasicBlockBuf(&blocks);

        koopa_raw_basic_block_data_t *entry_block = new koopa_raw_basic_block_data_t();
        entry_block->name = new_char_arr("%entry_" + ident);
        entry_block->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
        entry_block->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);

        symbol_list.NewEnv();
        block_maintainer.AddNewBasicBlock(entry_block);
        block_maintainer.SetCurrentFunction(res);
        for(size_t i = 0; i < fparams.size(); i++)
        {
            auto &fp = fparams[i];
            koopa_raw_value_data *allo = AllocType("@" + fp->name, ((koopa_raw_value_t)par[i])->ty);
            if(allo->ty->data.pointer.base->tag == KOOPA_RTT_POINTER)
                symbol_list.AddSymbol(fp->name, LValSymbol(LValSymbol::Pointer, allo));
            else
                symbol_list.AddSymbol(fp->name, LValSymbol(LValSymbol::Var, allo));
            block_maintainer.AddInst(allo);
            koopa_raw_value_data *sto = new koopa_raw_value_data();
            sto->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
            sto->name = nullptr;
            sto->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
            sto->kind.tag = KOOPA_RVT_STORE;
            sto->kind.data.store.value = (koopa_raw_value_t)par[i];
            sto->kind.data.store.dest = allo;
            block_maintainer.AddInst(sto);
        }
        block->build_koopa_values_no_env();
        symbol_list.DeleteEnv();
        block_maintainer.FinishCurrentBlock();

        res->bbs = make_koopa_rs_from_vector(blocks, KOOPA_RSIK_BASIC_BLOCK);

        return res;
    }
};

class ReturnAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> ret_num;
    ReturnAST()
    {
        ret_num = nullptr;
    }
    ReturnAST(std::unique_ptr<BaseAST> &_ret_num)
    {
        ret_num = std::move(_ret_num);
    }
    void *build_koopa_values() const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
        res->name = nullptr;
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_RETURN;
        if(ret_num)
            res->kind.data.ret.value = (const koopa_raw_value_data *)ret_num->build_koopa_values();
        else
            res->kind.data.ret.value = nullptr;
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
    void *build_koopa_values() const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
        res->name = nullptr;
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_STORE;
        res->kind.data.store.value = (koopa_raw_value_t)exp->build_koopa_values();
        res->kind.data.store.dest = (koopa_raw_value_t)lval->koopa_leftvalue();
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
    void *build_koopa_values() const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
        res->name = nullptr;
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_BRANCH;
        res->kind.data.branch.cond = (koopa_raw_value_t)exp->build_koopa_values();
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

class WhileAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> exp;
    InstSet body_insts;
    WhileAST(std::unique_ptr<BaseAST> &_exp, InstSet &_body_insts)
    {
        for (auto &inst : _body_insts)
            body_insts.push_back(std::make_pair(inst.first, std::move(inst.second)));
        exp = std::move(_exp);
    }
    void *build_koopa_values() const override
    {
        koopa_raw_basic_block_data_t *while_entry = new koopa_raw_basic_block_data_t();
        koopa_raw_basic_block_data_t *while_body = new koopa_raw_basic_block_data_t();
        koopa_raw_basic_block_data_t *end_block = new koopa_raw_basic_block_data_t();
        loop_maintainer.AddLoop(while_entry, while_body, end_block);

        block_maintainer.AddInst(JumpInst(while_entry));

        while_entry->name = new_char_arr("%while_entry");
        while_entry->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
        while_entry->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        block_maintainer.AddNewBasicBlock(while_entry);

        koopa_raw_value_data *br = new koopa_raw_value_data();
        br->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
        br->name = nullptr;
        br->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        br->kind.tag = KOOPA_RVT_BRANCH;
        br->kind.data.branch.cond = (koopa_raw_value_t)exp->build_koopa_values();
        br->kind.data.branch.true_bb = while_body;
        br->kind.data.branch.false_bb = end_block;
        br->kind.data.branch.true_args = empty_koopa_rs(KOOPA_RSIK_VALUE);
        br->kind.data.branch.false_args = empty_koopa_rs(KOOPA_RSIK_VALUE);
        block_maintainer.AddInst(br);

        while_body->name = new_char_arr("%while_body");
        while_body->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
        while_body->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        block_maintainer.AddNewBasicBlock(while_body);
        BlockAST::add_InstSet(this->body_insts);
        block_maintainer.AddInst(JumpInst(while_entry));

        end_block->name = new_char_arr("%end");
        end_block->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
        end_block->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        block_maintainer.AddNewBasicBlock(end_block);
        
        loop_maintainer.PopLoop();
        return nullptr;
    }
};

class BreakAST : public BaseAST
{
public:
    void *build_koopa_values() const override
    {
        block_maintainer.AddInst(JumpInst(loop_maintainer.GetLoop().end_block));
        return nullptr;
    }
};

class ContinueAST : public BaseAST
{
public:
    void *build_koopa_values() const override
    {
        block_maintainer.AddInst(JumpInst(loop_maintainer.GetLoop().while_entry));
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

    void *build_koopa_values() const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
        res->name = nullptr;
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
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

    void *build_koopa_values() const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        res->ty = make_int_pointer_type();
        res->name = new_char_arr("@" + name);
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
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
            store->kind.data.store.value = (koopa_raw_value_t)exp->build_koopa_values();
            block_maintainer.AddInst(store);
        }

        return res;
    }
};

class GlobalVarDefAST : public BaseAST
{
public:
    std::string name;
    std::unique_ptr<BaseAST> exp;

    GlobalVarDefAST(std::unique_ptr<BaseAST> &vardef_ast)
    {
        VarDefAST *var = dynamic_cast<VarDefAST*>(vardef_ast.release());
        name = var->name;
        exp = std::move(var->exp);
        delete var;
    }

    void *build_koopa_values() const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        res->ty = make_int_pointer_type();
        res->name = new_char_arr("@" + name);
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_GLOBAL_ALLOC;
        if(exp)
            res->kind.data.global_alloc.init = make_koopa_interger(exp->CalcValue());//(koopa_raw_value_data*)exp->build_koopa_values();
        else
            res->kind.data.global_alloc.init = ZeroInit();
        block_maintainer.AddInst(res);
        symbol_list.AddSymbol(name, LValSymbol(LValSymbol::Var, res));
        return res;
    }
};
