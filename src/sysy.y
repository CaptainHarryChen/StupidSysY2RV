%code requires {
    #include <memory>
    #include <string>
    #include "ast/ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include "ast/ast.hpp"


std::vector<std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>>> env_stk;

void add_inst(InstType instType, BaseAST *ast)
{
    env_stk[env_stk.size()-1].push_back(make_pair(instType, std::unique_ptr<BaseAST>(ast)));
}


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
%token INT RETURN CONST
%token <str_val> IDENT UNARYOP MULOP ADDOP RELOP EQOP LANDOP LOROP
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <base_ast_val> FuncDef FuncType Block
%type <base_ast_val> LVal Number
%type <base_ast_val> Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp

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

Block :
    {
        env_stk.push_back(std::vector<std::pair<InstType, std::unique_ptr<BaseAST>>>());
    }
    '{' BlockItems '}' {
        $$ = new BlockAST(env_stk[env_stk.size()-1]);
        env_stk.pop_back();
    };

BlockItems : BlockItem | BlockItem BlockItems ;

BlockItem : Decl | Stmt ;

Stmt
    : RETURN Exp ';' {
        auto number = std::unique_ptr<BaseAST>($2);
        add_inst(InstType::Stmt, new ReturnAST(number));
    } | LVal '=' Exp ';' {
        auto lval = std::unique_ptr<BaseAST>($1);
        auto exp = std::unique_ptr<BaseAST>($3);
        add_inst(InstType::Stmt, new AssignmentAST(lval, exp));
    };

Decl : ConstDecl | VarDecl;

ConstDecl : CONST INT ConstDefList ';';
ConstDefList : ConstDef | ConstDefList ',' ConstDef
ConstDef
    : IDENT '=' Exp {
        auto exp = std::unique_ptr<BaseAST>($3);
        add_inst(InstType::ConstDecl, new ConstDefAST($1->c_str(), exp));
    };

VarDecl : INT VarDefList ';';
VarDefList : VarDef | VarDefList ',' VarDef
VarDef
    : IDENT {
        add_inst(InstType::Decl, new VarDefAST($1->c_str()));
    }
    | IDENT '=' Exp {
        auto exp = std::unique_ptr<BaseAST>($3);
        add_inst(InstType::Decl, new VarDefAST($1->c_str(), exp));
    };

LVal
    : IDENT {
        $$ = new LValAST($1->c_str());
    };

Exp 
    : LOrExp {
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
    | LVal {
        auto lval = std::unique_ptr<BaseAST>($1);
        $$ = new PrimaryExpAST(lval);
    };

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

RelExp
    : AddExp {
        auto add_exp = std::unique_ptr<BaseAST>($1);
        $$ = new RelExpAST(add_exp);
    }
    | RelExp RELOP AddExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        auto op = std::unique_ptr<std::string>($2);
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new RelExpAST(left_exp, op->c_str(), right_exp);
    };

EqExp
    : RelExp {
        auto rel_exp = std::unique_ptr<BaseAST>($1);
        $$ = new EqExpAST(rel_exp);
    }
    | EqExp EQOP RelExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        auto op = std::unique_ptr<std::string>($2);
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new EqExpAST(left_exp, op->c_str(), right_exp);
    };

LAndExp
    : EqExp {
        auto eq_exp = std::unique_ptr<BaseAST>($1);
        $$ = new LAndExpAST(eq_exp);
    }
    | LAndExp LANDOP EqExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        auto op = std::unique_ptr<std::string>($2);
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new LAndExpAST(left_exp, op->c_str(), right_exp);
    };

LOrExp
    : LAndExp {
        auto land_exp = std::unique_ptr<BaseAST>($1);
        $$ = new LOrExpAST(land_exp);
    }
    | LOrExp LOROP LAndExp {
        auto left_exp = std::unique_ptr<BaseAST>($1);
        auto op = std::unique_ptr<std::string>($2);
        auto right_exp = std::unique_ptr<BaseAST>($3);
        $$ = new LOrExpAST(left_exp, op->c_str(), right_exp);
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
