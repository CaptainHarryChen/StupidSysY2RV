#include "builder/riscv_builder.hpp"

void RISCVBuilder::load_to_reg(koopa_raw_value_t kval, const char *reg)
{
    if(kval->kind.tag == KOOPA_RVT_INTEGER)
        output << "\tli " << reg << ", " << kval->kind.data.integer.value << endl;
    else
        output << "\tlw " << reg << ", " << env.GetAddr(kval) << "(sp)" << endl;
}

void RISCVBuilder::gen_riscv_value_load(const koopa_raw_load_t *kload, int addr)
{
    output << endl;

    load_to_reg(kload->src, "t0");
    output << "\tsw t0, " << addr << "(sp)" << endl;
}

void RISCVBuilder::gen_riscv_value_store(const koopa_raw_store_t *kstore)
{
    output << endl;

    load_to_reg(kstore->value, "t0");
    output << "\tsw t0, " << env.GetAddr(kstore->dest) << "(sp)" << endl;
}

void RISCVBuilder::gen_riscv_value_binary(const koopa_raw_binary_t *kbinary, int addr)
{
    output << endl;

    load_to_reg(kbinary->lhs, "t0");
    load_to_reg(kbinary->rhs, "t1");
    switch(kbinary->op)
    {
    case KOOPA_RBO_NOT_EQ:
        output << "\txor t0, t0, t1" << endl;
        output << "\tsnez t0, t0" << endl;
        break;
    case KOOPA_RBO_EQ:
        output << "\txor t0, t0, t1" << endl;
        output << "\tseqz t0, t0" << endl;
        break;
    case KOOPA_RBO_GT:
        output << "\tsgt t0, t0, t1" << endl;
        break;
    case KOOPA_RBO_LT:
        output << "\tslt t0, t0, t1" << endl;
        break;
    case KOOPA_RBO_GE:
        output << "\tslt t0, t0, t1" << endl;
        output << "\txori t0, t0, 1" << endl;
        break;
    case KOOPA_RBO_LE:
        output << "\tsgt t0, t0, t1" << endl;
        output << "\txori t0, t0, 1" << endl;
        break;
    case KOOPA_RBO_ADD:
        output << "\tadd t0, t0, t1" << endl;
        break;
    case KOOPA_RBO_SUB:
        output << "\tsub t0, t0, t1" << endl;
        break;
    case KOOPA_RBO_MUL:
        output << "\tmul t0, t0, t1" << endl;
        break;
    case KOOPA_RBO_DIV:
        output << "\tdiv t0, t0, t1" << endl;
        break;
    case KOOPA_RBO_MOD:
        output << "\trem t0, t0, t1" << endl;
        break;
    case KOOPA_RBO_AND:
        output << "\tand t0, t0, t1" << endl;
        break;
    case KOOPA_RBO_OR:
        output << "\tor t0, t0, t1" << endl;
        break;
    case KOOPA_RBO_XOR:
        output << "\txor t0, t0, t1" << endl;
        break;
    case KOOPA_RBO_SHL:
        output << "\tsll t0, t0, t1" << endl;
        break;
    case KOOPA_RBO_SHR:
        output << "\tsrl t0, t0, t1" << endl;
        break;
    case KOOPA_RBO_SAR:
        output << "\tsra t0, t0, t1" << endl;
        break;
    }
    output << "\tsw t0, " << addr << "(sp)" << endl;
}

void RISCVBuilder::gen_riscv_value_return(const koopa_raw_return_t *kret)
{
    output << endl;

    if(kret->value->kind.tag == KOOPA_RVT_INTEGER)
        output << "\tli a0, " << kret->value->kind.data.integer.value << endl;
    else
        output << "\tlw a0, " << env.GetAddr(kret->value) << "(sp)" << endl;
    output << "\taddi sp, sp, " << env.GetTotalSize() << endl;
    output << "\tret" << endl;
}
