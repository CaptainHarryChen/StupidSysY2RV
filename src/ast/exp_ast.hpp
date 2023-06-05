#pragma once

#include "ast/base_ast.hpp"

class ExpAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> unaryExp;

    ExpAST(std::unique_ptr<BaseAST> &_unaryExp)
    {
        unaryExp = std::move(_unaryExp);
    }

    void *build_koopa_values(koopa_raw_slice_t parent) const override
    {
        return unaryExp->build_koopa_values(parent);
    }

    int CalcValue() const override
    {
        return unaryExp->CalcValue();
    }
};

class LValAST : public BaseAST
{
public:
    std::string name;
    LValAST(const char *_name) : name(_name) {}

    // 将变量作为左值返回（返回该左值的变量本身）
    void *to_koopa_item(koopa_raw_slice_t parent) const override
    {
        return (void *)symbol_list.GetSymbol(name).number;
    }

    // 将变量作为右值返回（读取变量里存储的值）
    void *build_koopa_values(koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = new koopa_raw_value_data();
        auto var = symbol_list.GetSymbol(name);
        if (var.type == LValSymbol::Const)
            return (void *)var.number;
        else if (var.type == LValSymbol::Var)
        {
            res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
            res->name = nullptr;
            res->used_by = parent;
            res->kind.tag = KOOPA_RVT_LOAD;
            res->kind.data.load.src = var.number;
            block_maintainer.AddInst(res);
        }
        return res;
    }

    int CalcValue() const override
    {
        auto var = symbol_list.GetSymbol(name);
        assert(var.type == LValSymbol::Const);
        return var.number->kind.data.integer.value;
    }
};

class PrimaryExpAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> nextExp; // Exp or Number
    PrimaryExpAST(std::unique_ptr<BaseAST> &_nextExp)
    {
        nextExp = std::move(_nextExp);
    }
    void *build_koopa_values(koopa_raw_slice_t parent) const override
    {
        return nextExp->build_koopa_values(parent);
    }
    int CalcValue() const override
    {
        return nextExp->CalcValue();
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

    void *build_koopa_values(koopa_raw_slice_t parent) const override
    {
        NumberAST zero(0);
        koopa_raw_value_data *res = nullptr;
        switch (type)
        {
        case Primary:
            res = (koopa_raw_value_data *)nextExp->build_koopa_values(parent);
            break;
        case Op:
            if (op == "+")
            {
                res = (koopa_raw_value_data *)nextExp->build_koopa_values(parent);
                break;
            }
            res = new koopa_raw_value_data();
            koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
            res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
            res->name = nullptr;
            res->used_by = parent;
            res->kind.tag = KOOPA_RVT_BINARY;
            auto &binary = res->kind.data.binary;
            if (op == "-")
                binary.op = KOOPA_RBO_SUB;
            else if (op == "!")
                binary.op = KOOPA_RBO_EQ;
            binary.lhs = (koopa_raw_value_t)zero.build_koopa_values(child_used_by);
            binary.rhs = (koopa_raw_value_t)nextExp->build_koopa_values(child_used_by);
            block_maintainer.AddInst(res);
            break;
        }
        return res;
    }
    int CalcValue() const override
    {
        if (type == Primary)
            return nextExp->CalcValue();
        int res = 0;
        if (op == "+")
            res = nextExp->CalcValue();
        else if (op == "-")
            res = -nextExp->CalcValue();
        else if (op == "!")
            res = !nextExp->CalcValue();
        return res;
    }
};

class MulExpAST : public BaseAST
{
public:
    enum
    {
        Primary,
        Op
    } type;
    std::string op;
    std::unique_ptr<BaseAST> leftExp; // may be primary
    std::unique_ptr<BaseAST> rightExp;

    MulExpAST(std::unique_ptr<BaseAST> &_primary_exp)
    {
        type = Primary;
        leftExp = std::move(_primary_exp);
    }
    MulExpAST(std::unique_ptr<BaseAST> &_left_exp, const char *_op, std::unique_ptr<BaseAST> &_right_exp)
    {
        type = Op;
        leftExp = std::move(_left_exp);
        op = std::string(_op);
        rightExp = std::move(_right_exp);
    }

    void *build_koopa_values(koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = nullptr;
        switch (type)
        {
        case Primary:
            res = (koopa_raw_value_data *)leftExp->build_koopa_values(parent);
            break;
        case Op:
            res = new koopa_raw_value_data();
            koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
            res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
            res->name = nullptr;
            res->used_by = parent;
            res->kind.tag = KOOPA_RVT_BINARY;
            auto &binary = res->kind.data.binary;
            if (op == "*")
                binary.op = KOOPA_RBO_MUL;
            else if (op == "/")
                binary.op = KOOPA_RBO_DIV;
            else if (op == "%")
                binary.op = KOOPA_RBO_MOD;
            binary.lhs = (koopa_raw_value_t)leftExp->build_koopa_values(child_used_by);
            binary.rhs = (koopa_raw_value_t)rightExp->build_koopa_values(child_used_by);
            block_maintainer.AddInst(res);
            break;
        }
        return res;
    }
    int CalcValue() const override
    {
        if (type == Primary)
            return leftExp->CalcValue();
        int res = 0;
        if (op == "*")
            res = leftExp->CalcValue() * rightExp->CalcValue();
        else if (op == "/")
            res = leftExp->CalcValue() / rightExp->CalcValue();
        else if (op == "%")
            res = leftExp->CalcValue() % rightExp->CalcValue();
        return res;
    }
};

class AddExpAST : public BaseAST
{
public:
    enum
    {
        Primary,
        Op
    } type;
    std::string op;
    std::unique_ptr<BaseAST> leftExp; // may be primary
    std::unique_ptr<BaseAST> rightExp;

    AddExpAST(std::unique_ptr<BaseAST> &_primary_exp)
    {
        type = Primary;
        leftExp = std::move(_primary_exp);
    }
    AddExpAST(std::unique_ptr<BaseAST> &_left_exp, const char *_op, std::unique_ptr<BaseAST> &_right_exp)
    {
        type = Op;
        leftExp = std::move(_left_exp);
        op = std::string(_op);
        rightExp = std::move(_right_exp);
    }

    void *build_koopa_values(koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = nullptr;
        switch (type)
        {
        case Primary:
            res = (koopa_raw_value_data *)leftExp->build_koopa_values(parent);
            break;
        case Op:
            res = new koopa_raw_value_data();
            koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
            res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
            res->name = nullptr;
            res->used_by = parent;
            res->kind.tag = KOOPA_RVT_BINARY;
            auto &binary = res->kind.data.binary;
            if (op == "+")
                binary.op = KOOPA_RBO_ADD;
            else if (op == "-")
                binary.op = KOOPA_RBO_SUB;
            binary.lhs = (koopa_raw_value_t)leftExp->build_koopa_values(child_used_by);
            binary.rhs = (koopa_raw_value_t)rightExp->build_koopa_values(child_used_by);
            block_maintainer.AddInst(res);
            break;
        }
        return res;
    }
    int CalcValue() const override
    {
        if (type == Primary)
            return leftExp->CalcValue();
        int res = 0;
        if (op == "+")
            res = leftExp->CalcValue() + rightExp->CalcValue();
        else if (op == "-")
            res = leftExp->CalcValue() - rightExp->CalcValue();
        return res;
    }
};

class RelExpAST : public BaseAST
{
public:
    enum
    {
        Primary,
        Op
    } type;
    std::string op;
    std::unique_ptr<BaseAST> leftExp; // may be primary
    std::unique_ptr<BaseAST> rightExp;

    RelExpAST(std::unique_ptr<BaseAST> &_primary_exp)
    {
        type = Primary;
        leftExp = std::move(_primary_exp);
    }
    RelExpAST(std::unique_ptr<BaseAST> &_left_exp, const char *_op, std::unique_ptr<BaseAST> &_right_exp)
    {
        type = Op;
        leftExp = std::move(_left_exp);
        op = std::string(_op);
        rightExp = std::move(_right_exp);
    }

    void *build_koopa_values(koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = nullptr;
        switch (type)
        {
        case Primary:
            res = (koopa_raw_value_data *)leftExp->build_koopa_values(parent);
            break;
        case Op:
            res = new koopa_raw_value_data();
            koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
            res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
            res->name = nullptr;
            res->used_by = parent;
            res->kind.tag = KOOPA_RVT_BINARY;
            auto &binary = res->kind.data.binary;
            if (op == "<")
                binary.op = KOOPA_RBO_LT;
            else if (op == "<=")
                binary.op = KOOPA_RBO_LE;
            else if (op == ">")
                binary.op = KOOPA_RBO_GT;
            else if (op == ">=")
                binary.op = KOOPA_RBO_GE;
            binary.lhs = (koopa_raw_value_t)leftExp->build_koopa_values(child_used_by);
            binary.rhs = (koopa_raw_value_t)rightExp->build_koopa_values(child_used_by);
            block_maintainer.AddInst(res);
            break;
        }
        return res;
    }
    int CalcValue() const override
    {
        if (type == Primary)
            return leftExp->CalcValue();
        int res = 0;
        if (op == "<")
            res = leftExp->CalcValue() < rightExp->CalcValue();
        else if (op == "<=")
            res = leftExp->CalcValue() <= rightExp->CalcValue();
        else if (op == ">")
            res = leftExp->CalcValue() > rightExp->CalcValue();
        else if (op == ">=")
            res = leftExp->CalcValue() >= rightExp->CalcValue();
        return res;
    }
};

class EqExpAST : public BaseAST
{
public:
    enum
    {
        Primary,
        Op
    } type;
    std::string op;
    std::unique_ptr<BaseAST> leftExp; // may be primary
    std::unique_ptr<BaseAST> rightExp;

    EqExpAST(std::unique_ptr<BaseAST> &_primary_exp)
    {
        type = Primary;
        leftExp = std::move(_primary_exp);
    }
    EqExpAST(std::unique_ptr<BaseAST> &_left_exp, const char *_op, std::unique_ptr<BaseAST> &_right_exp)
    {
        type = Op;
        leftExp = std::move(_left_exp);
        op = std::string(_op);
        rightExp = std::move(_right_exp);
    }

    void *build_koopa_values(koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = nullptr;
        switch (type)
        {
        case Primary:
            res = (koopa_raw_value_data *)leftExp->build_koopa_values(parent);
            break;
        case Op:
            res = new koopa_raw_value_data();
            koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
            res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
            res->name = nullptr;
            res->used_by = parent;
            res->kind.tag = KOOPA_RVT_BINARY;
            auto &binary = res->kind.data.binary;
            if (op == "==")
                binary.op = KOOPA_RBO_EQ;
            else if (op == "!=")
                binary.op = KOOPA_RBO_NOT_EQ;
            binary.lhs = (koopa_raw_value_t)leftExp->build_koopa_values(child_used_by);
            binary.rhs = (koopa_raw_value_t)rightExp->build_koopa_values(child_used_by);
            block_maintainer.AddInst(res);
            break;
        }
        return res;
    }
    int CalcValue() const override
    {
        if (type == Primary)
            return leftExp->CalcValue();
        int res = 0;
        if (op == "==")
            res = leftExp->CalcValue() == rightExp->CalcValue();
        else if (op == "!=")
            res = leftExp->CalcValue() != rightExp->CalcValue();
        return res;
    }
};

class LAndExpAST : public BaseAST
{
    koopa_raw_value_data *make_bool_koopa(koopa_raw_slice_t parent, koopa_raw_value_t exp) const
    {
        NumberAST zero(0);
        koopa_raw_value_data *res = new koopa_raw_value_data();
        koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
        res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
        res->name = nullptr;
        res->used_by = parent;
        res->kind.tag = KOOPA_RVT_BINARY;
        auto &binary = res->kind.data.binary;
        binary.op = KOOPA_RBO_NOT_EQ;
        binary.lhs = exp;
        binary.rhs = (koopa_raw_value_t)zero.build_koopa_values(child_used_by);
        block_maintainer.AddInst(res);
        return res;
    }

public:
    enum
    {
        Primary,
        Op
    } type;
    std::string op;
    std::unique_ptr<BaseAST> leftExp; // may be primary
    std::unique_ptr<BaseAST> rightExp;

    LAndExpAST(std::unique_ptr<BaseAST> &_primary_exp)
    {
        type = Primary;
        leftExp = std::move(_primary_exp);
    }
    LAndExpAST(std::unique_ptr<BaseAST> &_left_exp, const char *_op, std::unique_ptr<BaseAST> &_right_exp)
    {
        type = Op;
        leftExp = std::move(_left_exp);
        op = std::string(_op);
        rightExp = std::move(_right_exp);
    }

    void *build_koopa_values(koopa_raw_slice_t parent) const override
    {
        std::unique_ptr<NumberAST> zero(new NumberAST(0));
        koopa_raw_value_data *res = nullptr;
        switch (type)
        {
        case Primary:
            res = (koopa_raw_value_data *)leftExp->build_koopa_values(parent);
            break;
        case Op:
            koopa_raw_value_data *temp_var = new koopa_raw_value_data();
            koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(temp_var, KOOPA_RSIK_VALUE);
            temp_var->ty = make_int_pointer_type();
            temp_var->name = new_char_arr("%temp");
            temp_var->used_by = parent;
            temp_var->kind.tag = KOOPA_RVT_ALLOC;
            block_maintainer.AddInst(temp_var);
            
            koopa_raw_value_data *temp_store = new koopa_raw_value_data();
            child_used_by = make_koopa_rs_single_element(temp_store, KOOPA_RSIK_VALUE);
            temp_store->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
            temp_store->name = nullptr;
            temp_store->used_by = empty_koopa_rs();
            temp_store->kind.tag = KOOPA_RVT_STORE;
            temp_store->kind.data.store.dest = temp_var;
            temp_store->kind.data.store.value = (koopa_raw_value_t)zero->build_koopa_values(child_used_by);
            block_maintainer.AddInst(temp_store);

            koopa_raw_value_data *br = new koopa_raw_value_data();
            child_used_by = make_koopa_rs_single_element(br, KOOPA_RSIK_VALUE);
            br->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
            br->name = nullptr;
            br->used_by = parent;
            br->kind.tag = KOOPA_RVT_BRANCH;
            br->kind.data.branch.cond = make_bool_koopa(child_used_by, (koopa_raw_value_t)leftExp->build_koopa_values(child_used_by));
            koopa_raw_basic_block_data_t *true_block = new koopa_raw_basic_block_data_t();
            koopa_raw_basic_block_data_t *end_block = new koopa_raw_basic_block_data_t();
            br->kind.data.branch.true_bb = true_block;
            br->kind.data.branch.false_bb = end_block;
            br->kind.data.branch.true_args = empty_koopa_rs(KOOPA_RSIK_VALUE);
            br->kind.data.branch.false_args = empty_koopa_rs(KOOPA_RSIK_VALUE);
            block_maintainer.AddInst(br);

            true_block->name = new_char_arr("%true");
            true_block->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
            true_block->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
            block_maintainer.AddNewBasicBlock(true_block);
            
            koopa_raw_value_data *b_store = new koopa_raw_value_data();
            child_used_by = make_koopa_rs_single_element(temp_store, KOOPA_RSIK_VALUE);
            b_store->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
            b_store->name = nullptr;
            b_store->used_by = empty_koopa_rs();
            b_store->kind.tag = KOOPA_RVT_STORE;
            b_store->kind.data.store.dest = temp_var;
            b_store->kind.data.store.value = make_bool_koopa(child_used_by, (koopa_raw_value_t)rightExp->build_koopa_values(child_used_by));
            block_maintainer.AddInst(b_store);
            block_maintainer.AddInst(JumpInst(end_block));

            end_block->name = new_char_arr("%end");
            end_block->params = empty_koopa_rs(KOOPA_RSIK_VALUE);
            end_block->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
            block_maintainer.AddNewBasicBlock(end_block);

            res = new koopa_raw_value_data();
            res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
            res->name = nullptr;
            res->used_by = parent;
            res->kind.tag = KOOPA_RVT_LOAD;
            res->kind.data.load.src = temp_var;
            block_maintainer.AddInst(res);

            break;
        }
        return res;
    }
    int CalcValue() const override
    {
        if (type == Primary)
            return leftExp->CalcValue();
        int res = 0;
        if (op == "&&")
            res = leftExp->CalcValue() && rightExp->CalcValue();
        return res;
    }
};

class LOrExpAST : public BaseAST
{
    void *make_bool_koopa(koopa_raw_slice_t parent, koopa_raw_value_t exp) const
    {
        NumberAST zero(0);
        koopa_raw_value_data *res = new koopa_raw_value_data();
        koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
        res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
        res->name = nullptr;
        res->used_by = parent;
        res->kind.tag = KOOPA_RVT_BINARY;
        auto &binary = res->kind.data.binary;
        binary.op = KOOPA_RBO_NOT_EQ;
        binary.lhs = exp;
        binary.rhs = (koopa_raw_value_t)zero.build_koopa_values(child_used_by);
        block_maintainer.AddInst(res);
        return res;
    }

public:
    enum
    {
        Primary,
        Op
    } type;
    std::string op;
    std::unique_ptr<BaseAST> leftExp; // may be primary
    std::unique_ptr<BaseAST> rightExp;

    LOrExpAST(std::unique_ptr<BaseAST> &_primary_exp)
    {
        type = Primary;
        leftExp = std::move(_primary_exp);
    }
    LOrExpAST(std::unique_ptr<BaseAST> &_left_exp, const char *_op, std::unique_ptr<BaseAST> &_right_exp)
    {
        type = Op;
        leftExp = std::move(_left_exp);
        op = std::string(_op);
        rightExp = std::move(_right_exp);
    }

    void *build_koopa_values(koopa_raw_slice_t parent) const override
    {
        std::unique_ptr<NumberAST> zero(new NumberAST(0));
        koopa_raw_value_data *res = nullptr;
        switch (type)
        {
        case Primary:
            res = (koopa_raw_value_data *)leftExp->build_koopa_values(parent);
            break;
        case Op:
            res = new koopa_raw_value_data();
            koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
            res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
            res->name = nullptr;
            res->used_by = parent;
            res->kind.tag = KOOPA_RVT_BINARY;
            auto &binary = res->kind.data.binary;
            if (op == "||")
                binary.op = KOOPA_RBO_OR;
            binary.lhs = (koopa_raw_value_t)make_bool_koopa(child_used_by, (koopa_raw_value_t)leftExp->build_koopa_values(child_used_by));
            binary.rhs = (koopa_raw_value_t)make_bool_koopa(child_used_by, (koopa_raw_value_t)rightExp->build_koopa_values(child_used_by));
            block_maintainer.AddInst(res);
            break;
        }
        return res;
    }
    int CalcValue() const override
    {
        if (type == Primary)
            return leftExp->CalcValue();
        int res = 0;
        if (op == "||")
            res = leftExp->CalcValue() || rightExp->CalcValue();
        return res;
    }
};
