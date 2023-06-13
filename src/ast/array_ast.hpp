#pragma once

#include "ast/base_ast.hpp"
#include "koopa_util.hpp"

class InitValAST : public BaseAST
{
    enum ValType{
        Exp,
        Array
    };

public:
    ValType type;
    std::unique_ptr<BaseAST> exp;
    std::vector<std::unique_ptr<InitValAST>> arr_list;

    InitValAST(std::unique_ptr<BaseAST> &_exp)
    {
        type = Exp;
        exp = std::move(_exp);
    }
    InitValAST(std::vector<BaseAST*> &_arr_list)
    {
        type = Array;
        for(auto t : _arr_list)
            arr_list.emplace_back(dynamic_cast<InitValAST*>(t));

    }

    koopa_raw_value_t At(int idx)
    {
        if(type == Array)
        {
            if(idx >= (int)arr_list.size())
                return make_koopa_interger(0);
            return arr_list[idx]->At(0);
        }
        else if(type == Exp)
            return (koopa_raw_value_t)exp->build_koopa_values();
        return nullptr;
    }

    koopa_raw_value_t make_aggerate()
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        koopa_raw_type_kind *ty = new koopa_raw_type_kind();
        ty->tag = KOOPA_RTT_ARRAY;
        ty->data.array.len = arr_list.size();
        ty->data.array.base = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
        res->ty = ty;
        res->name = nullptr;
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_AGGREGATE;

        std::vector<const void*> elems;
        for(auto &elem : arr_list)
            elems.push_back(make_koopa_interger(elem->exp->CalcValue()));
        res->kind.data.aggregate.elems = make_koopa_rs_from_vector(elems, KOOPA_RSIK_VALUE);
        return res;
    }
};

class ArrayDefAST : public BaseAST
{
public:
    std::string name;
    std::unique_ptr<BaseAST> sz_exp;
    std::unique_ptr<InitValAST> init_val;

    ArrayDefAST(const char *_name, std::unique_ptr<BaseAST> &_exp) : name(_name)
    {
        sz_exp = std::move(_exp);
        init_val = nullptr;
    }
    ArrayDefAST(const char *_name, std::unique_ptr<BaseAST> &_exp, std::unique_ptr<BaseAST> &_init_val) : name(_name)
    {
        sz_exp = std::move(_exp);
        init_val = std::unique_ptr<InitValAST>(dynamic_cast<InitValAST*>(_init_val.release()));
    }

    void *build_koopa_values() const override
    {
        int sz = sz_exp->CalcValue();
        koopa_raw_value_data *res = new koopa_raw_value_data();
        koopa_raw_type_kind *ty = new koopa_raw_type_kind();
        ty->tag = KOOPA_RTT_ARRAY;
        ty->data.array.len = sz;
        ty->data.array.base = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
        koopa_raw_type_kind *tty = new koopa_raw_type_kind();
        tty->tag = KOOPA_RTT_POINTER;
        tty->data.pointer.base = ty;
        res->ty = tty;
        res->name = new_char_arr("@" + name);
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_ALLOC;
        block_maintainer.AddInst(res);
        symbol_list.AddSymbol(name, LValSymbol(LValSymbol::Array, res));

        if(init_val)
        {
            for(int i = 0; i < sz; i++)
            {
                koopa_raw_value_data *get = new koopa_raw_value_data();
                get->ty = make_int_pointer_type();
                get->name = nullptr;
                get->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
                get->kind.tag = KOOPA_RVT_GET_ELEM_PTR;
                get->kind.data.get_elem_ptr.src = res;
                get->kind.data.get_elem_ptr.index = make_koopa_interger(i);
                block_maintainer.AddInst(get);

                koopa_raw_value_data *st = new koopa_raw_value_data();
                st->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
                st->name = nullptr;
                st->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
                st->kind.tag = KOOPA_RVT_STORE;
                st->kind.data.store.dest = get;
                st->kind.data.store.value = init_val->At(i);
                block_maintainer.AddInst(st);
            }
        }
        return res;
    }
};

class GlobalArrayDefAST : public BaseAST
{
public:
    std::string name;
    std::unique_ptr<BaseAST> sz_exp;
    std::unique_ptr<InitValAST> init_val;

    GlobalArrayDefAST(std::unique_ptr<BaseAST> &arraydef_ast)
    {
        ArrayDefAST *arraydef = dynamic_cast<ArrayDefAST*>(arraydef_ast.release());
        name = arraydef->name;
        sz_exp = std::move(arraydef->sz_exp);
        init_val = std::move(arraydef->init_val);
    }

    void *build_koopa_values() const override
    {
        int sz = sz_exp->CalcValue();
        koopa_raw_value_data *res = new koopa_raw_value_data();
        koopa_raw_type_kind *ty = new koopa_raw_type_kind();
        ty->tag = KOOPA_RTT_ARRAY;
        ty->data.array.len = sz;
        ty->data.array.base = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
        koopa_raw_type_kind *tty = new koopa_raw_type_kind();
        tty->tag = KOOPA_RTT_POINTER;
        tty->data.pointer.base = ty;
        res->ty = tty;
        res->name = new_char_arr("@" + name);
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_GLOBAL_ALLOC;
        if(init_val)
            res->kind.data.global_alloc.init = init_val->make_aggerate();
        else
            res->kind.data.global_alloc.init = ZeroInit();
        symbol_list.AddSymbol(name, LValSymbol(LValSymbol::Array, res));

        return res;
    }
};
