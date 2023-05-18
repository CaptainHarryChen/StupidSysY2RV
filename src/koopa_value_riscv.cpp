#include "koopa_value_riscv.hpp"
#include "koopa_riscv.hpp"

void gen_riscv_value_return(const koopa_raw_return_t *kval, std::string &res)
{
    //TODO: kval->value ???
    res += "li a0, ";
    gen_riscv_value(kval->value, res);
    res += "\nret\n";
}