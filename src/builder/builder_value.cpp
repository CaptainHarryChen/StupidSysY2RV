#include "builder/riscv_builder.hpp"

void RISCVBuilder::load_to_reg(koopa_raw_value_t kval, const char *reg)
{
    if (kval->kind.tag == KOOPA_RVT_INTEGER)
        output << "\tli " << reg << ", " << kval->kind.data.integer.value << endl;
    else if(kval->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
    {
        output << "\tla t0, " << kval->name + 1 << endl;
        output << "\tlw " << reg << ", 0(t0)" << endl;
    }
    else
    {
        int addr = env.GetAddr(kval);
        if(addr < -2048 || addr > 2047)
        {
            output << "\tli t6, " << addr << endl;
            output << "\tadd t6, t6, sp" << endl;
            output << "\tlw " << reg << ", " << "0(t6)" << endl;
        }
        else
            output << "\tlw " << reg << ", " << addr << "(sp)" << endl;
    }
}

void RISCVBuilder::store_to_stack(int addr, const char *reg)
{
    if(addr < -2048 || addr > 2047)
    {
        output << "\tli t6, " << addr << endl;
        output << "\tadd t6, t6, sp" << endl;
        output << "\tsw " << reg << ", " << "0(t6)" << endl;
    }
    else
        output << "\tsw " << reg << ", " << addr << "(sp)" << endl;
}

void RISCVBuilder::gen_riscv_value_aggregate(koopa_raw_value_t kval)
{
    if(kval->ty->tag == KOOPA_RTT_ARRAY)
    {
        for(int i = 0; i < kval->kind.data.aggregate.elems.len; i++)
            gen_riscv_value_aggregate((koopa_raw_value_t)kval->kind.data.aggregate.elems.buffer[i]);
    }
    else
        output << "\t.word " << kval->kind.data.integer.value << endl;
}

void RISCVBuilder::gen_riscv_value_global_alloc(koopa_raw_value_t kalloc)
{
    output << ".globl " << kalloc->name + 1 << endl;
    output << kalloc->name + 1 << ":" << endl;
    if (kalloc->kind.data.global_alloc.init->kind.tag == KOOPA_RVT_ZERO_INIT)
    {
        output << "\t.zero " << calc_type_size(kalloc->ty->data.pointer.base) << endl;
    }
    else if(kalloc->kind.data.global_alloc.init->kind.tag == KOOPA_RVT_AGGREGATE)
        gen_riscv_value_aggregate(kalloc->kind.data.global_alloc.init);
    else
        output << "\t.word " << kalloc->kind.data.global_alloc.init->kind.data.integer.value << endl;
}

void RISCVBuilder::gen_riscv_value_load(const koopa_raw_load_t *kload, int addr)
{
    output << endl;

    if(kload->src->kind.tag == KOOPA_RVT_GET_ELEM_PTR || kload->src->kind.tag == KOOPA_RVT_GET_PTR)
    {
        load_to_reg(kload->src, "t0");
        output << "\tlw t0, 0(t0)" << endl;
        store_to_stack(addr, "t0");
    }
    else
    {
        load_to_reg(kload->src, "t0");
        store_to_stack(addr, "t0");
    }
}

void RISCVBuilder::gen_riscv_value_store(const koopa_raw_store_t *kstore)
{
    output << endl;
    
    std::string dest;
    if(kstore->dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
    {
        output << "\tla t1, " << kstore->dest->name + 1 << endl;
        dest = "0(t1)";
    }
    else if(kstore->dest->kind.tag == KOOPA_RVT_GET_ELEM_PTR || kstore->dest->kind.tag == KOOPA_RVT_GET_PTR)
    {
        load_to_reg(kstore->dest, "t1");
        dest = "0(t1)";
    }
    else
    {
        int addr = env.GetAddr(kstore->dest);
        if(addr < -2048 || addr > 2047)
        {
            output << "\tli t1, " << addr << endl;
            output << "\tadd t1, t1, sp" << endl;
            dest = "0(t1)";
        }
        else
            dest = std::to_string(addr) + "(sp)";
    }
    if(kstore->value->kind.tag == KOOPA_RVT_FUNC_ARG_REF)
    {
        if(kstore->value->kind.data.func_arg_ref.index < 8)
            output << "\tsw a" << kstore->value->kind.data.func_arg_ref.index << ", " << dest << endl;
        else
        {
            int offset = (kstore->value->kind.data.func_arg_ref.index - 8) * 4;
            if(offset < -2048 || offset > 2047)
            {
                output << "\tli t2, " << offset << endl;
                output << "\taddi t2, t2, sp" << endl;
                output << "\tlw t0, 0(t2)" << endl;
            }
            else
                output << "\tlw t0, " << offset << "(sp)" << endl;
            output << "\tsw t0, " << dest << endl;
        }
    }
    else
    {
        load_to_reg(kstore->value, "t0");
        output << "\tsw t0, " << dest << endl;
    }
}

void RISCVBuilder::gen_riscv_value_get_ptr(const koopa_raw_get_ptr_t *kget, int addr)
{
    output << endl;

    int src_addr = env.GetAddr(kget->src);
    if(src_addr > 2047 || src_addr < -2048)
    {
        output << "\tli t0, " << src_addr << endl;
        output << "\tadd t0, sp, t0" << endl;
    }
    else
        output << "\taddi t0, sp, " << src_addr << endl;
    output << "\tlw t0, 0(t0)" << endl;

    load_to_reg(kget->index, "t1");
    int n = calc_type_size(kget->src->ty->data.pointer.base);
    output << "\tli t2, " << n << endl;
    output << "\tmul t1, t1, t2" << endl;
    output << "\tadd t0, t0, t1" << endl;
    store_to_stack(addr, "t0");
}

void RISCVBuilder::gen_riscv_value_get_elem_ptr(const koopa_raw_get_elem_ptr_t *kget, int addr)
{
    output << endl;

    if(kget->src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
    {
        output << "\tla t0, " << kget->src->name + 1 << endl;
    }
    else
    {
        int src_addr = env.GetAddr(kget->src);
        if(src_addr > 2047 || src_addr < -2048)
        {
            output << "\tli t0, " << src_addr << endl;
            output << "\tadd t0, sp, t0" << endl;
        }
        else
            output << "\taddi t0, sp, " << src_addr << endl;
        if(kget->src->kind.tag == KOOPA_RVT_GET_ELEM_PTR || kget->src->kind.tag == KOOPA_RVT_GET_PTR)
            output << "\tlw t0, 0(t0)" << endl;
    }
    load_to_reg(kget->index, "t1");
    int n = calc_type_size(kget->src->ty->data.pointer.base->data.array.base);
    output << "\tli t2, " << n << endl;
    output << "\tmul t1, t1, t2" << endl;
    output << "\tadd t0, t0, t1" << endl;
    store_to_stack(addr, "t0");
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
    store_to_stack(addr, "t0");
}

void RISCVBuilder::gen_riscv_value_branch(const koopa_raw_branch_t *kbranch)
{
    output << endl;
    load_to_reg(kbranch->cond, "t0");
    // output << "\tbnez t0, " << current_func_name << "_" << kbranch->true_bb->name + 1 << endl;
    // 解决跳转超限问题
    output << "\tbeqz t0, " << current_func_name << "_" << "skip" << magic_cnt_num << endl;
    output << "\tj " << current_func_name << "_" << kbranch->true_bb->name + 1 << endl;
    output << current_func_name << "_" << "skip" << magic_cnt_num++ << ":" << endl;

    output << "\tj " << current_func_name << "_" << kbranch->false_bb->name + 1 << endl;
}

void RISCVBuilder::gen_riscv_value_jump(const koopa_raw_jump_t *kjump)
{
    output << endl;
    output << "\tj " << current_func_name << "_" << kjump->target->name + 1 << endl;
}

void RISCVBuilder::gen_riscv_value_call(const koopa_raw_call_t *kcall, int addr)
{
    output << endl;
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
        int offset = (i - 8) * 4 - func_sz;
        if(offset < -2048 || offset > 2047)
        {
            output << "\tli t6, " << offset << endl;
            output << "\tadd t6, t6, sp" << endl;
            output << "\tsw t0, 0(t6)" << endl;
        }
        else
            output << "\tsw t0, " << offset << "(sp)" << endl;
    }
    output << "\tcall " << kcall->callee->name + 1 << endl;
    if (addr != -1)
        store_to_stack(addr, "a0");
}

void RISCVBuilder::gen_riscv_value_return(const koopa_raw_return_t *kret)
{
    output << endl;

    if (kret->value)
        load_to_reg(kret->value, "a0");
    if (env.has_call)
    {
        int offset = env.GetTotalSize() - 4;
        if(offset < -2048 || offset > 2047)
        {
            output << "\tli t6, " << offset << endl;
            output << "\tadd t6, t6, sp" << endl;
            output << "\tlw ra, 0(t6)" << endl;
        }
        else
            output << "\tlw ra, " << offset << "(sp)" << endl;
    }
    if (env.GetTotalSize() != 0)
    {
        int sz = env.GetTotalSize();
        if(sz < -2048 || sz > 2047)
        {
            output << "\tli t0, " << sz << endl;
            output << "\tadd sp, sp, t0" << endl;
        }
        else
            output << "\taddi sp, sp, " << sz << endl;
    }
    output << "\tret" << endl;
}
