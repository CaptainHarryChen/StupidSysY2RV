#pragma once

#include "ast/base_ast.hpp"
#include "koopa_util.hpp"

class InitValAST : public BaseAST
{
    enum ValType{
        Exp,
        Array
    };

    std::vector<koopa_raw_value_t> cache;

public:
    bool is_const = false;
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

    void sub_preprocess(std::vector<int> &pro, int align_pos, std::vector<koopa_raw_value_t> &buf)
    {
        int target_size = buf.size() + pro[align_pos];
        for(size_t i = 0; i < arr_list.size(); i++)
        {
            auto &t = arr_list[i];
            if(t->type == Exp)
            {
                if(is_const)
                    buf.push_back(make_koopa_interger(t->exp->CalcValue()));
                else
                    buf.push_back((koopa_raw_value_t)t->exp->build_koopa_values());
            }
            else
            {
                int new_align_pos = align_pos + 1;
                while(cache.size() % pro[new_align_pos] != 0)
                    new_align_pos ++;
                arr_list[i]->sub_preprocess(pro, new_align_pos, buf);
            }
        }
        while(buf.size() < target_size)
            buf.push_back(make_koopa_interger(0));
    }

    void preprocess(const std::vector<int> &sz)
    {
        std::vector<int> pro(sz.size() + 1);
        pro[sz.size()] = 1;
        for(int i = sz.size() - 1; i >= 0; i--)
            pro[i] = pro[i + 1] * sz[i];
        sub_preprocess(pro, 0, cache);
    }

    koopa_raw_value_t At(int idx)
    {
        if(type == Array)
            return cache[idx];
        else if(type == Exp)
            return (koopa_raw_value_t)exp->build_koopa_values();
        return nullptr;
    }
    
    koopa_raw_value_t sub_make_aggerate(const std::vector<int> &sz, std::vector<int> &pro, int align_pos, std::vector<koopa_raw_value_t> &buf, int st_pos)
    {
        if(pro[align_pos] == 1)
            return buf[st_pos];
        koopa_raw_value_data *res = new koopa_raw_value_data();
        res->ty = make_array_type(sz, align_pos);
        res->name = nullptr;
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_AGGREGATE;

        std::vector<const void*> elems;
        for(int i = 0; i < sz[align_pos]; i++)
            elems.push_back(sub_make_aggerate(sz, pro, align_pos + 1, buf, st_pos + pro[align_pos + 1] * i));
        res->kind.data.aggregate.elems = make_koopa_rs_from_vector(elems, KOOPA_RSIK_VALUE);
        return res;
    }

    koopa_raw_value_t make_aggerate(const std::vector<int> &sz)
    {
        std::vector<int> pro(sz.size() + 1);
        pro[sz.size()] = 1;
        for(int i = sz.size() - 1; i >= 0; i--)
            pro[i] = pro[i + 1] * sz[i];
        return sub_make_aggerate(sz, pro, 0, cache, 0);
    }
};

class ArrayDefAST : public BaseAST
{
    koopa_raw_value_data *get_index(int i, std::vector<int> &pro, koopa_raw_value_data *src, int cur_pos = 0) const
    {
        if(cur_pos >= pro.size())
            return src;
        koopa_raw_value_data *get = new koopa_raw_value_data();
        koopa_raw_type_kind *ty = new koopa_raw_type_kind();
        ty->tag = KOOPA_RTT_POINTER;
        ty->data.pointer.base = src->ty->data.pointer.base->data.array.base;
        get->ty = ty;
        get->name = nullptr;
        get->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        get->kind.tag = KOOPA_RVT_GET_ELEM_PTR;
        get->kind.data.get_elem_ptr.src = src;
        get->kind.data.get_elem_ptr.index = make_koopa_interger(i / pro[cur_pos]);
        block_maintainer.AddInst(get);
        return get_index(i % pro[cur_pos], pro, get, cur_pos + 1);
    }

public:
    std::string name;
    std::vector<std::unique_ptr<BaseAST>> sz_exp;
    std::unique_ptr<InitValAST> init_val;

    ArrayDefAST(const char *_name, std::vector<BaseAST*> &_exp) : name(_name)
    {
        for(auto &e : _exp)
            sz_exp.emplace_back(e);
        init_val = nullptr;
    }
    ArrayDefAST(const char *_name, std::vector<BaseAST*> &_exp, std::unique_ptr<BaseAST> &_init_val) : name(_name)
    {
        for(auto &e : _exp)
            sz_exp.emplace_back(e);
        init_val = std::unique_ptr<InitValAST>(dynamic_cast<InitValAST*>(_init_val.release()));
    }

    void *build_koopa_values() const override
    {
        int total_size = 1;
        std::vector<int> sz;
        for(auto &e : sz_exp)
        {
            int tmp = e->CalcValue();
            total_size *= tmp;
            sz.push_back(tmp);
        }
        
        koopa_raw_value_data *res = new koopa_raw_value_data();
        koopa_raw_type_kind *ty = make_array_type(sz);
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
            init_val->preprocess(sz);
            std::vector<int> pro(sz.size());
            pro[sz.size() - 1] = 1;
            for(int i = sz.size() - 2; i >= 0; i--)
                pro[i] = pro[i + 1] * sz[i + 1];
            for(int i = 0; i < total_size; i++)
            {
                koopa_raw_value_data *get = get_index(i, pro, res);

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
    std::vector<std::unique_ptr<BaseAST>> sz_exp;
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
        std::vector<int> sz;
        for(auto &e : sz_exp)
            sz.push_back(e->CalcValue());
        koopa_raw_value_data *res = new koopa_raw_value_data();
        koopa_raw_type_kind *ty = make_array_type(sz);
        koopa_raw_type_kind *tty = new koopa_raw_type_kind();
        tty->tag = KOOPA_RTT_POINTER;
        tty->data.pointer.base = ty;
        res->ty = tty;
        res->name = new_char_arr("@" + name);
        res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
        res->kind.tag = KOOPA_RVT_GLOBAL_ALLOC;
        if(init_val)
        {
            init_val->is_const = true;
            init_val->preprocess(sz);
            res->kind.data.global_alloc.init = init_val->make_aggerate(sz);
        }
        else
            res->kind.data.global_alloc.init = ZeroInit(ty);
        symbol_list.AddSymbol(name, LValSymbol(LValSymbol::Array, res));

        return res;
    }
};
