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

static std::vector<InstSet> env_stk;
static std::vector<BaseAST*> value_list;
static std::vector<BaseAST*> func_list;
static std::vector<BaseAST*> fparams;
static std::vector<std::vector<BaseAST*>> rparams;
static std::vector<BaseAST*> arr_size;
static std::vector<std::vector<BaseAST*>> idx_stk;

enum InitValType
{
    array,
    exp
};
static std::vector<std::vector<BaseAST*>> arr_list;

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
%token INT VOID RETURN CONST IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT UNARYOP MULOP ADDOP RELOP EQOP LANDOP LOROP
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <base_ast_val> FuncDef BType Block IfExp
%type <base_ast_val> LVal Number InitVal
%type <base_ast_val> Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit 
    : {
        env_stk.push_back(InstSet());
        }
        GlobalList {
        ast = std::unique_ptr<BaseAST>(new CompUnitAST(func_list, env_stk[env_stk.size()-1]));
        env_stk.pop_back();
    };

GlobalList
    : FuncDef {
        func_list.push_back($1);
    } | GlobalList FuncDef {
        func_list.push_back($2);
    } 
    | Decl | GlobalList Decl ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
FuncDef
    : BType IDENT '('  {
            fparams.clear();
        } FuncFParams ')' Block {
            auto rettype = std::unique_ptr<BaseAST>($1);
            auto ident = std::unique_ptr<std::string>($2);
            auto block = std::unique_ptr<BaseAST>($7);
            $$ = new FuncDefAST(rettype, ident->c_str(), fparams, block);
    } | BType IDENT '(' ')' Block {
        auto rettype = std::unique_ptr<BaseAST>($1);
        auto ident = std::unique_ptr<std::string>($2);
        auto block = std::unique_ptr<BaseAST>($5);
        fparams.clear();
        $$ = new FuncDefAST(rettype, ident->c_str(), fparams, block);
    };

BType
    : INT {
        $$ = new BTypeAST("int");
    } | VOID {
        $$ = new BTypeAST("void");
    };

FuncFParams : FuncFParam | FuncFParams ',' FuncFParam;

FuncFParam
    : INT IDENT {
        fparams.push_back(new FuncFParamAST(FuncFParamAST::Int, $2->c_str(), fparams.size()));
    }
    | INT IDENT '[' ']' {
        fparams.push_back(new FuncFParamAST(FuncFParamAST::Array, $2->c_str(), fparams.size(), arr_size));
    }
    | INT IDENT '[' ']' ArraySizeList {
        fparams.push_back(new FuncFParamAST(FuncFParamAST::Array, $2->c_str(), fparams.size(), arr_size));
        arr_size.clear();
    };

Block :
    '{' {
        env_stk.push_back(InstSet());
    }
    BlockItems '}' {
        $$ = new BlockAST(env_stk[env_stk.size()-1]);
        env_stk.pop_back();
    } | '{' '}' {
        $$ = new BlockAST();
    };

BlockItems : BlockItem | BlockItem BlockItems ;

BlockItem : Decl | Stmt ;

Stmt
    : RETURN ';' {
        add_inst(InstType::Stmt, new ReturnAST());
    }
    | RETURN Exp ';' {
        auto number = std::unique_ptr<BaseAST>($2);
        add_inst(InstType::Stmt, new ReturnAST(number));
    } | LVal '=' Exp ';' {
        auto lval = std::unique_ptr<BaseAST>($1);
        auto exp = std::unique_ptr<BaseAST>($3);
        add_inst(InstType::Stmt, new AssignmentAST(lval, exp));
    } | IfExp Stmt ELSE {
            env_stk.push_back(InstSet());
        } Stmt {
            auto exp = std::unique_ptr<BaseAST>($1);
            InstSet true_instset, false_instset;
            for(auto &inst : env_stk[env_stk.size()-2])
                true_instset.push_back(std::make_pair(inst.first, std::move(inst.second)));
            for(auto &inst : env_stk[env_stk.size()-1])
                false_instset.push_back(std::make_pair(inst.first, std::move(inst.second)));
            env_stk.pop_back();
            env_stk.pop_back();
            add_inst(InstType::Branch, new BranchAST(exp, true_instset, false_instset));
        } 
    | IfExp Stmt {
            auto exp = std::unique_ptr<BaseAST>($1);
            InstSet true_instset;
            for(auto &inst : env_stk[env_stk.size()-1])
                true_instset.push_back(std::make_pair(inst.first, std::move(inst.second)));
            env_stk.pop_back();
            add_inst(InstType::Branch, new BranchAST(exp, true_instset));
    } | WHILE '(' Exp ')' {
            env_stk.push_back(InstSet());
        } Stmt {
            auto exp = std::unique_ptr<BaseAST>($3);
            InstSet while_body;
            for(auto &inst : env_stk[env_stk.size()-1])
                while_body.push_back(std::make_pair(inst.first, std::move(inst.second)));
            env_stk.pop_back();
            add_inst(InstType::While, new WhileAST(exp, while_body));
    } | BREAK ';' {
        add_inst(InstType::Break, new BreakAST());
    } | CONTINUE ';' {
        add_inst(InstType::Continue, new ContinueAST());
    }
    | ';' | Exp ';' {
        add_inst(InstType::Stmt, $1);
    } | Block {
        add_inst(InstType::Stmt, $1);
    };

IfExp 
    : IF '(' Exp ')' {
        env_stk.push_back(InstSet());
        $$ = $3;
    };

Decl : ConstDecl | VarDecl;

ConstDecl : CONST BType ConstDefList ';';
ConstDefList : ConstDef | ConstDefList ',' ConstDef
ConstDef
    : IDENT '=' Exp {
        auto exp = std::unique_ptr<BaseAST>($3);
        add_inst(InstType::ConstDecl, new ConstDefAST($1->c_str(), exp));
    }
    | IDENT ArraySizeList '=' InitVal {
        auto initval = std::unique_ptr<BaseAST>($4);
        add_inst(InstType::ArrayDecl, new ArrayDefAST($1->c_str(), arr_size, initval));
        arr_size.clear();
    };
    | IDENT ArraySizeList {
        add_inst(InstType::ArrayDecl, new ArrayDefAST($1->c_str(), arr_size));
        arr_size.clear();
    };

VarDecl : BType VarDefList ';';
VarDefList : VarDef | VarDefList ',' VarDef
VarDef
    : IDENT {
        add_inst(InstType::Decl, new VarDefAST($1->c_str()));
    }
    | IDENT '=' Exp {
        auto exp = std::unique_ptr<BaseAST>($3);
        add_inst(InstType::Decl, new VarDefAST($1->c_str(), exp));
    }
    | IDENT ArraySizeList '=' InitVal {
        auto initval = std::unique_ptr<BaseAST>($4);
        add_inst(InstType::ArrayDecl, new ArrayDefAST($1->c_str(), arr_size, initval));
        arr_size.clear();
    };
    | IDENT ArraySizeList {
        add_inst(InstType::ArrayDecl, new ArrayDefAST($1->c_str(), arr_size));
        arr_size.clear();
    }

ArraySizeList : ArraySize | ArraySizeList ArraySize;

ArraySize 
    : '[' Exp ']' {
        arr_size.push_back($2);
    };

InitVal : Exp {
        auto exp = std::unique_ptr<BaseAST>($1);
        $$ = new InitValAST(exp);
    }
    | '{' {
            arr_list.push_back(std::vector<BaseAST*>());
        } ArrInitList '}' {
        $$ = new InitValAST(arr_list[arr_list.size()-1]);
        arr_list.pop_back();
    }
    | '{' '}' {
        arr_list.push_back(std::vector<BaseAST*>());
        $$ = new InitValAST(arr_list[arr_list.size()-1]);
        arr_list.pop_back();
    };

ArrInitList : InitVal {
        arr_list[arr_list.size()-1].push_back($1);
    } 
    | ArrInitList ',' InitVal {
        arr_list[arr_list.size()-1].push_back($3);
    };

LVal
    : IDENT {
        $$ = new LValAST($1->c_str());
    }
    | IDENT {
            idx_stk.push_back(std::vector<BaseAST*>());
        } IndexList {
            $$ = new LValAST($1->c_str(), idx_stk[idx_stk.size()-1]);
            idx_stk.pop_back();
    };

IndexList : Index | IndexList Index

Index : '[' Exp ']' {
        idx_stk[idx_stk.size()-1].push_back($2);
    }

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
    | IDENT '(' {
            rparams.push_back(std::vector<BaseAST*>());
        } FuncRParams ')' {
            $$ = new UnaryExpAST($1->c_str(), rparams[rparams.size()-1]);
            rparams.pop_back();
    }
    | IDENT '(' ')' {
        rparams.push_back(std::vector<BaseAST*>());
        $$ = new UnaryExpAST($1->c_str(), rparams[rparams.size()-1]);
        rparams.pop_back();
    }
    ;

FuncRParams : FuncRParam | FuncRParams ',' FuncRParam;

FuncRParam 
    : Exp {
        rparams[rparams.size()-1].push_back($1);
    }

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
