#include "ast/code_ast.hpp"
#include "ast/array_ast.hpp"

void CompUnitAST::add_lib_funcs(std::vector<const void*> &funcs) const
{
    koopa_raw_function_data_t *func;
    koopa_raw_type_kind_t *ty;
    std::vector<const void *> fparams;

    func = new koopa_raw_function_data_t();
    ty = new koopa_raw_type_kind_t();
    ty->tag = KOOPA_RTT_FUNCTION;
    ty->data.function.params = empty_koopa_rs(KOOPA_RSIK_TYPE);
    ty->data.function.ret = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
    func->ty = ty;
    func->name = "@getint";
    func->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
    func->bbs = empty_koopa_rs(KOOPA_RSIK_BASIC_BLOCK);
    symbol_list.AddSymbol("getint", LValSymbol(LValSymbol::Function, func));
    funcs.push_back(func);

    func = new koopa_raw_function_data_t();
    ty = new koopa_raw_type_kind_t();
    ty->tag = KOOPA_RTT_FUNCTION;
    ty->data.function.params = empty_koopa_rs(KOOPA_RSIK_TYPE);
    ty->data.function.ret = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
    func->ty = ty;
    func->name = "@getch";
    func->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
    func->bbs = empty_koopa_rs(KOOPA_RSIK_BASIC_BLOCK);
    symbol_list.AddSymbol("getch", LValSymbol(LValSymbol::Function, func));
    funcs.push_back(func);

    func = new koopa_raw_function_data_t();
    ty = new koopa_raw_type_kind_t();
    ty->tag = KOOPA_RTT_FUNCTION;
    fparams.clear();
    fparams.push_back(make_int_pointer_type());
    ty->data.function.params = make_koopa_rs_from_vector(fparams, KOOPA_RSIK_TYPE);
    ty->data.function.ret = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
    func->ty = ty;
    func->name = "@getarray";
    func->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
    func->bbs = empty_koopa_rs(KOOPA_RSIK_BASIC_BLOCK);
    symbol_list.AddSymbol("getarray", LValSymbol(LValSymbol::Function, func));
    funcs.push_back(func);

    func = new koopa_raw_function_data_t();
    ty = new koopa_raw_type_kind_t();
    ty->tag = KOOPA_RTT_FUNCTION;
    fparams.clear();
    fparams.push_back(simple_koopa_raw_type_kind(KOOPA_RTT_INT32));
    ty->data.function.params = make_koopa_rs_from_vector(fparams, KOOPA_RSIK_TYPE);
    ty->data.function.ret = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
    func->ty = ty;
    func->name = "@putint";
    func->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
    func->bbs = empty_koopa_rs(KOOPA_RSIK_BASIC_BLOCK);
    symbol_list.AddSymbol("putint", LValSymbol(LValSymbol::Function, func));
    funcs.push_back(func);

    func = new koopa_raw_function_data_t();
    ty = new koopa_raw_type_kind_t();
    ty->tag = KOOPA_RTT_FUNCTION;
    fparams.clear();
    fparams.push_back(simple_koopa_raw_type_kind(KOOPA_RTT_INT32));
    ty->data.function.params = make_koopa_rs_from_vector(fparams, KOOPA_RSIK_TYPE);
    ty->data.function.ret = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
    func->ty = ty;
    func->name = "@putch";
    func->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
    func->bbs = empty_koopa_rs(KOOPA_RSIK_BASIC_BLOCK);
    symbol_list.AddSymbol("putch", LValSymbol(LValSymbol::Function, func));
    funcs.push_back(func);

    func = new koopa_raw_function_data_t();
    ty = new koopa_raw_type_kind_t();
    ty->tag = KOOPA_RTT_FUNCTION;
    fparams.clear();
    fparams.push_back(simple_koopa_raw_type_kind(KOOPA_RTT_INT32));
    fparams.push_back(make_int_pointer_type());
    ty->data.function.params = make_koopa_rs_from_vector(fparams, KOOPA_RSIK_TYPE);
    ty->data.function.ret = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
    func->ty = ty;
    func->name = "@putarray";
    func->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
    func->bbs = empty_koopa_rs(KOOPA_RSIK_BASIC_BLOCK);
    symbol_list.AddSymbol("putarray", LValSymbol(LValSymbol::Function, func));
    funcs.push_back(func);

    func = new koopa_raw_function_data_t();
    ty = new koopa_raw_type_kind_t();
    ty->tag = KOOPA_RTT_FUNCTION;
    ty->data.function.params = empty_koopa_rs(KOOPA_RSIK_TYPE);
    ty->data.function.ret = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
    func->ty = ty;
    func->name = "@starttime";
    func->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
    func->bbs = empty_koopa_rs(KOOPA_RSIK_BASIC_BLOCK);
    symbol_list.AddSymbol("starttime", LValSymbol(LValSymbol::Function, func));
    funcs.push_back(func);

    func = new koopa_raw_function_data_t();
    ty = new koopa_raw_type_kind_t();
    ty->tag = KOOPA_RTT_FUNCTION;
    ty->data.function.params = empty_koopa_rs(KOOPA_RSIK_TYPE);
    ty->data.function.ret = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
    func->ty = ty;
    func->name = "@stoptime";
    func->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
    func->bbs = empty_koopa_rs(KOOPA_RSIK_BASIC_BLOCK);
    symbol_list.AddSymbol("stoptime", LValSymbol(LValSymbol::Function, func));
    funcs.push_back(func);
}

CompUnitAST::CompUnitAST(std::vector<BaseAST*> &_func_list, InstSet &_value_list)
{
    for(BaseAST* func : _func_list)
        func_list.emplace_back(func);
    for(auto &pa : _value_list)
    {
        if(pa.first == Decl)
            value_list.push_back(make_pair(pa.first, std::unique_ptr<BaseAST>(new GlobalVarDefAST(pa.second))));
        else if(pa.first == ArrayDecl)
            value_list.push_back(make_pair(pa.first, std::unique_ptr<BaseAST>(new GlobalArrayDefAST(pa.second))));
        else
            value_list.push_back(make_pair(pa.first, std::move(pa.second)));
    }
}
