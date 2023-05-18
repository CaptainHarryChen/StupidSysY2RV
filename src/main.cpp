#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <assert.h>

#include "ast.hpp"

// 声明 lexer 的输入, 以及 parser 函数
extern FILE *yyin;
extern int yyparse(std::unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[])
{
    // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
    // compiler 模式 输入文件 -o 输出文件
    assert(argc == 5);
    auto mode = argv[1];
    auto input = argv[2];
    auto output = argv[4];

    std::cout << "mode: " << mode << std::endl;
    // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
    yyin = fopen(input, "r");
    assert(yyin);

    // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
    std::unique_ptr<BaseAST> ast;
    auto ret = yyparse(ast);
    assert(!ret);

    // 输出解析得到的 AST, 其实就是个字符串
    std::cout << ast->to_string() << std::endl;

    std::unique_ptr<CompUnitAST> comp_ast(dynamic_cast<CompUnitAST *>(ast.release()));
    koopa_raw_program_t res = comp_ast->to_koopa_raw_program();

    koopa_program_t kp;
    koopa_error_code_t eno = koopa_generate_raw_to_koopa(&res, &kp);
    if (eno != KOOPA_EC_SUCCESS)
    {
        std::cout << "generate raw to koopa error: " << (int)eno << std::endl;
        return 0;
    }
    char *buffer = new char[1000];
    size_t sz = 1000u;
    eno = koopa_dump_to_string(kp, buffer, &sz);
    if (eno != KOOPA_EC_SUCCESS)
    {
        std::cout << "dump to string error: " << (int)eno << std::endl;
        return 0;
    }
    std::cout << buffer << std::endl;
    std::ofstream yyout(output);
    yyout << buffer;

    return 0;
}
