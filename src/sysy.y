%code requires {
    #include <memory>
    #include <string>
    #include "ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "ast.hpp"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

%}

// 定义 parser 函数和错误处理函数的附加参数
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
%union {
    std::string *str_val;
    int int_val;
    BaseAST *base_ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN
%token <str_val> IDENT UNARYOP MULOP ADDOP
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <base_ast_val> FuncDef FuncType Block Stmt Exp PrimaryExp UnaryExp MulExp AddExp Number

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
    : FuncDef {
        auto func = std::unique_ptr<BaseAST>($1);
        ast = std::unique_ptr<BaseAST>(new CompUnitAST(func));
    }
    ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
FuncDef
    : FuncType IDENT '(' ')' Block {
        auto type = std::unique_ptr<BaseAST>($1);
        auto ident = std::unique_ptr<std::string>($2);
        auto block = std::unique_ptr<BaseAST>($5);
        $$ = new FuncDefAST(type, ident->c_str(), block);
    }
    ;

FuncType
    : INT {
        $$ = new FuncTypeAST("int");
    }
    ;

Block
    : '{' Stmt '}' {
        auto stmt = std::unique_ptr<BaseAST>($2);
        $$ = new BlockAST(stmt);
    }
    ;

Stmt
    : RETURN Exp ';' {
        auto number = std::unique_ptr<BaseAST>($2);
        $$ = new StmtAST(number);
    }
    ;

Exp 
    : AddExp {
        auto add_exp = std::unique_ptr<BaseAST>($1);
        $$ = new ExpAST(add_exp);
    }
    ;

PrimaryExp  
    : '(' Exp ')' {
        auto exp = std::unique_ptr<BaseAST>($2);
        $$ = new PrimaryExpAST(exp);
    }
    | Number {
        auto number = std::unique_ptr<BaseAST>($1);
        $$ = new PrimaryExpAST(number);
    }
    ;
UnaryExp
    : PrimaryExp {
        auto primary_exp = std::unique_ptr<BaseAST>($1);
        $$ = new UnaryExpAST(primary_exp);
    }
    | UNARYOP UnaryExp {
        auto op = std::unique_ptr<std::string>($1);
        auto unary_exp = std::unique_ptr<BaseAST>($2);
        $$ = new UnaryExpAST(op->c_str(), unary_exp);
    }
    | ADDOP UnaryExp {
        auto op = std::unique_ptr<std::string>($1);
        auto unary_exp = std::unique_ptr<BaseAST>($2);
        $$ = new UnaryExpAST(op->c_str(), unary_exp);
    }
    ;

MulExp
    : UnaryExp {
        auto unary_exp = std::unique_ptr<BaseAST>($1);
        $$ = new MulExpAST(unary_exp);
    }
    | MulExp MULOP UnaryExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        auto op = std::unique_ptr<std::string>($2);
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new MulExpAST(left_exp, op->c_str(), right_exp);
    };

AddExp
    : MulExp {
        auto mul_exp = std::unique_ptr<BaseAST>($1);
        $$ = new MulExpAST(mul_exp);
    }
    | AddExp ADDOP MulExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        auto op = std::unique_ptr<std::string>($2);
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new AddExpAST(left_exp, op->c_str(), right_exp);
    };

Number
    : INT_CONST {
        $$ = new NumberAST($1);
    }
    ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s) {
    std::cerr << "error: " << s << std::endl;
}
