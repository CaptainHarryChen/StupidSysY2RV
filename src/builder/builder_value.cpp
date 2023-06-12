#include "builder/riscv_builder.hpp"

void RISCVBuilder::load_to_reg(koopa_raw_value_t kval, const char *reg)
{
    if (kval->kind.tag == KOOPA_RVT_INTEGER)
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

    if(kstore->value->kind.tag == KOOPA_RVT_FUNC_ARG_REF)
    {
        if(kstore->value->kind.data.func_arg_ref.index < 8)
            output << "\tsw a" << kstore->value->kind.data.func_arg_ref.index << ", " << env.GetAddr(kstore->dest) << "(sp)" << endl;
        else
        {
            output << "\tlw t0, " << (kstore->value->kind.data.func_arg_ref.index - 8) * 4 << "(sp)" << endl;
            output << "\tsw t0, " << env.GetAddr(kstore->dest) << "(sp)" << endl;
        }
    }
    else
    {
        load_to_reg(kstore->value, "t0");
        output << "\tsw t0, " << env.GetAddr(kstore->dest) << "(sp)" << endl;
    }
}

void RISCVBuilder::gen_riscv_value_binary(const koopa_raw_binary_t *kbinary, int addr)
{
    output << endl;

    load_to_reg(kbinary->lhs, "t0");
    load_to_reg(kbinary->rhs, "t1");
    switch (kbinary->op)
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

void RISCVBuilder::gen_riscv_value_branch(const koopa_raw_branch_t *kbranch)
{
    load_to_reg(kbranch->cond, "t0");
    output << "\tbnez t0, " << kbranch->true_bb->name + 1 << endl;
    output << "\tj " << kbranch->false_bb->name + 1 << endl;
}

void RISCVBuilder::gen_riscv_value_jump(const koopa_raw_jump_t *kjump)
{
    output << "\tj " << kjump->target->name + 1 << endl;
}

void RISCVBuilder::gen_riscv_value_call(const koopa_raw_call_t *kcall, int addr)
{
    for (int i = 0; i < kcall->args.len && i < 8; i++)
    {
        char reg[3] = "a0";
        reg[1] += i;
        load_to_reg((koopa_raw_value_t)kcall->args.buffer[i], reg);
    }
    bool has_call = false;
    int func_sz = calc_func_size(kcall->callee, has_call);
    if(func_sz)
        func_sz = ((func_sz - 1) / 16 + 1) * 16;
    for (int i = 8; i < kcall->args.len; i++)
    {
        load_to_reg((koopa_raw_value_t)kcall->args.buffer[i], "t0");
        output << "\tsw t0, " << (i - 8) * 4 - func_sz << "(sp)" << endl;
    }
    output << "\tcall " << kcall->callee->name + 1 << endl;
    if (addr != -1)
        output << "\tsw a0, " << addr << "(sp)" << endl;
}

void RISCVBuilder::gen_riscv_value_return(const koopa_raw_return_t *kret)
{
    output << endl;

    if (kret->value)
    {
        if (kret->value->kind.tag == KOOPA_RVT_INTEGER)
            output << "\tli a0, " << kret->value->kind.data.integer.value << endl;
        else
            output << "\tlw a0, " << env.GetAddr(kret->value) << "(sp)" << endl;
    }
    if (env.has_call)
        output << "\tlw ra, " << env.GetTotalSize() - 4 << "(sp)" << endl;
    if (env.GetTotalSize() != 0)
        output << "\taddi sp, sp, " << env.GetTotalSize() << endl;
    output << "\tret" << endl;
}
