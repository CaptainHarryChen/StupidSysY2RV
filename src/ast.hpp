#pragma once

#include <memory>
#include <string>

// 所有 AST 的基类
class BaseAST
{
public:
    virtual ~BaseAST() = default;
    virtual std::string to_string() const = 0;
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
};

class BlockAST : public BaseAST
{
public:
    std::unique_ptr<BaseAST> stmt;
    BlockAST(std::unique_ptr<BaseAST> &_stmt)
    {
        stmt = std::move(_stmt);
    }
    std::string to_string() const override
    {
        return "BlockAST { " + stmt->to_string() + " }";
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
};

class NumberAST : public BaseAST
{
public:
    int val;
    NumberAST(int _val) : val(_val) {}
    std::string to_string() const override
    {
        return "NumberAST { int " + std::to_string(val) + " }";
    }
};
