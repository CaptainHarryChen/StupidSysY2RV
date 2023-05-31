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

    void *build_koopa_values(std::vector<const void *> &buf, koopa_raw_slice_t parent) const override
    {
        return unaryExp->build_koopa_values(buf, parent);
    }
};

class NumberAST : public BaseAST
{
public:
    int val;
    NumberAST(int _val) { val = _val; }
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
    void *build_koopa_values(std::vector<const void *> &buf, koopa_raw_slice_t parent) const override
    {
        auto res = to_koopa_item(parent);
        // buf.push_back(res);
        return res;
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
    void *build_koopa_values(std::vector<const void *> &buf, koopa_raw_slice_t parent) const override
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

    void *build_koopa_values(std::vector<const void *> &buf, koopa_raw_slice_t parent) const override
    {
        NumberAST zero(0);
        koopa_raw_value_data *res = nullptr;
        switch (type)
        {
        case Primary:
            res = (koopa_raw_value_data *)nextExp->build_koopa_values(buf, parent);
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
            else if (op == "!")
                binary.op = KOOPA_RBO_EQ;
            binary.lhs = (koopa_raw_value_t)zero.build_koopa_values(buf, child_used_by);
            binary.rhs = (koopa_raw_value_t)nextExp->build_koopa_values(buf, child_used_by);
            buf.push_back(res);
            break;
        }
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

    void *build_koopa_values(std::vector<const void *> &buf, koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = nullptr;
        switch (type)
        {
        case Primary:
            res = (koopa_raw_value_data *)leftExp->build_koopa_values(buf, parent);
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
            binary.lhs = (koopa_raw_value_t)leftExp->build_koopa_values(buf, child_used_by);
            binary.rhs = (koopa_raw_value_t)rightExp->build_koopa_values(buf, child_used_by);
            buf.push_back(res);
            break;
        }
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

    void *build_koopa_values(std::vector<const void *> &buf, koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = nullptr;
        switch (type)
        {
        case Primary:
            res = (koopa_raw_value_data *)leftExp->build_koopa_values(buf, parent);
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
            binary.lhs = (koopa_raw_value_t)leftExp->build_koopa_values(buf, child_used_by);
            binary.rhs = (koopa_raw_value_t)rightExp->build_koopa_values(buf, child_used_by);
            buf.push_back(res);
            break;
        }
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

    void *build_koopa_values(std::vector<const void *> &buf, koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = nullptr;
        switch (type)
        {
        case Primary:
            res = (koopa_raw_value_data *)leftExp->build_koopa_values(buf, parent);
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
            binary.lhs = (koopa_raw_value_t)leftExp->build_koopa_values(buf, child_used_by);
            binary.rhs = (koopa_raw_value_t)rightExp->build_koopa_values(buf, child_used_by);
            buf.push_back(res);
            break;
        }
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

    void *build_koopa_values(std::vector<const void *> &buf, koopa_raw_slice_t parent) const override
    {
        koopa_raw_value_data *res = nullptr;
        switch (type)
        {
        case Primary:
            res = (koopa_raw_value_data *)leftExp->build_koopa_values(buf, parent);
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
            binary.lhs = (koopa_raw_value_t)leftExp->build_koopa_values(buf, child_used_by);
            binary.rhs = (koopa_raw_value_t)rightExp->build_koopa_values(buf, child_used_by);
            buf.push_back(res);
            break;
        }
        return res;
    }
};

class LAndExpAST : public BaseAST
{
    void *make_bool_koopa(std::vector<const void *> &buf, koopa_raw_slice_t parent, koopa_raw_value_t exp) const
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
        binary.rhs = (koopa_raw_value_t)zero.build_koopa_values(buf, child_used_by);
        buf.push_back(res);
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

    void *build_koopa_values(std::vector<const void *> &buf, koopa_raw_slice_t parent) const override
    {
        std::unique_ptr<NumberAST> zero(new NumberAST(0));
        koopa_raw_value_data *res = nullptr;
        switch (type)
        {
        case Primary:
            res = (koopa_raw_value_data *)leftExp->build_koopa_values(buf, parent);
            break;
        case Op:
            res = new koopa_raw_value_data();
            koopa_raw_slice_t child_used_by = make_koopa_rs_single_element(res, KOOPA_RSIK_VALUE);
            res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
            res->name = nullptr;
            res->used_by = parent;
            res->kind.tag = KOOPA_RVT_BINARY;
            auto &binary = res->kind.data.binary;
            if (op == "&&")
                binary.op = KOOPA_RBO_AND;
            binary.lhs = (koopa_raw_value_t)make_bool_koopa(buf, child_used_by, (koopa_raw_value_t)leftExp->build_koopa_values(buf, child_used_by));
            binary.rhs = (koopa_raw_value_t)make_bool_koopa(buf, child_used_by, (koopa_raw_value_t)rightExp->build_koopa_values(buf, child_used_by));
            buf.push_back(res);
            break;
        }
        return res;
    }
};

class LOrExpAST : public BaseAST
{
    void *make_bool_koopa(std::vector<const void *> &buf, koopa_raw_slice_t parent, koopa_raw_value_t exp) const
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
        binary.rhs = (koopa_raw_value_t)zero.build_koopa_values(buf, child_used_by);
        buf.push_back(res);
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

    void *build_koopa_values(std::vector<const void *> &buf, koopa_raw_slice_t parent) const override
    {
        std::unique_ptr<NumberAST> zero(new NumberAST(0));
        koopa_raw_value_data *res = nullptr;
        switch (type)
        {
        case Primary:
            res = (koopa_raw_value_data *)leftExp->build_koopa_values(buf, parent);
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
            binary.lhs = (koopa_raw_value_t)make_bool_koopa(buf, child_used_by, (koopa_raw_value_t)leftExp->build_koopa_values(buf, child_used_by));
            binary.rhs = (koopa_raw_value_t)make_bool_koopa(buf, child_used_by, (koopa_raw_value_t)rightExp->build_koopa_values(buf, child_used_by));
            buf.push_back(res);
            break;
        }
        return res;
    }
};
