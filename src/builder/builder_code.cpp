#include "builder/riscv_builder.hpp"

int RISCVBuilder::calc_func_size(koopa_raw_function_t kfunc, bool &has_call)
{
    int total_size = 0;
    for (uint32_t i = 0; i < kfunc->bbs.len; i++)
    {
        const void *data = kfunc->bbs.buffer[i];
        total_size += calc_blk_size((koopa_raw_basic_block_t)data, has_call);
    }
    total_size += has_call ? 4 : 0;
    return total_size;
}

int RISCVBuilder::calc_blk_size(koopa_raw_basic_block_t kblk, bool &has_call)
{
    // TODO: did not count params 
    int total_size = 0;
    for (uint32_t i = 0; i < kblk->insts.len; i++)
    {
        const void *data = kblk->insts.buffer[i];
        if(((koopa_raw_value_t)data)->kind.tag == KOOPA_RVT_CALL)
            has_call = true;
        total_size += calc_inst_size((koopa_raw_value_t)data);
    }
    return total_size;
}

int RISCVBuilder::calc_inst_size(koopa_raw_value_t kval)
{
    if(kval->kind.tag == KOOPA_RVT_ALLOC)
        return calc_type_size(kval->ty->data.pointer.base);
    return calc_type_size(kval->ty);
}

int RISCVBuilder::calc_type_size(koopa_raw_type_t ty)
{
    switch(ty->tag)
    {
    case KOOPA_RTT_INT32:
        return 4;
    case KOOPA_RTT_UNIT:
        return 0;
    case KOOPA_RTT_POINTER:
        return 4;
    case KOOPA_RTT_ARRAY:
        return calc_type_size(ty->data.array.base) * ty->data.array.len;
    }
}

void RISCVBuilder::gen_riscv_func(koopa_raw_function_t kfunc)
{
    if(kfunc->bbs.len == 0)
        return;
    const char *name = kfunc->name + 1;
    output << ".globl " << name << endl;
    output << name << ":" << endl;

    bool has_call = false;
    int size = calc_func_size(kfunc, has_call);
    if(size != 0)
    {
        size = ((size - 1) / 16 + 1) * 16;
        if(-size < -2048 || -size > 2047)
        {
            output << "\tli t0, " << -size << endl;
            output << "\tadd sp, sp, t0" << endl;
        }
        else
            output << "\taddi sp, sp, " << -size << endl;
    }
    if(has_call)
    {
        int offset = size - 4;
        if(offset < -2048 || offset > 2047)
        {
            output << "\tli t0, " << offset << endl;
            output << "\tadd t0, sp, t0" << endl;
            output << "\tsw ra, 0(t0)" << endl;
        }
        else
            output << "\tsw ra, " << offset << "(sp)" << endl;
    }
    env.NewEnv(size, has_call);
    env.cur -= (has_call ? 4 : 0);
    // blocks
    current_func_name = kfunc->name + 1;
    traversal_raw_slice(&kfunc->bbs);
}

void RISCVBuilder::gen_riscv_block(koopa_raw_basic_block_t kblk)
{
    //TODO: params
    //TODO: used_by
    output << endl;
    output << current_func_name << "_" << kblk->name + 1 << ":" << endl;
    traversal_raw_slice(&kblk->insts);
}

void RISCVBuilder::gen_riscv_value(koopa_raw_value_t kval)
{
    int addr = env.GetAddr(kval);
    switch(kval->kind.tag)
    {
    case KOOPA_RVT_INTEGER:
        output << kval->kind.data.integer.value;
        break;
    case KOOPA_RVT_ALLOC:
        break;
    case KOOPA_RVT_GLOBAL_ALLOC:
        gen_riscv_value_global_alloc(kval);
        break;
    case KOOPA_RVT_LOAD:
        gen_riscv_value_load(&kval->kind.data.load, addr);
        break;
    case KOOPA_RVT_STORE:
        gen_riscv_value_store(&kval->kind.data.store);
        break;
    case KOOPA_RVT_GET_PTR:
        gen_riscv_value_get_ptr(&kval->kind.data.get_ptr, addr);
        break;
    case KOOPA_RVT_GET_ELEM_PTR:
        gen_riscv_value_get_elem_ptr(&kval->kind.data.get_elem_ptr, addr);
        break;
    case KOOPA_RVT_BINARY:
        gen_riscv_value_binary(&kval->kind.data.binary, addr);
        break;
    case KOOPA_RVT_BRANCH:
        gen_riscv_value_branch(&kval->kind.data.branch);
        break;
    case KOOPA_RVT_JUMP:
        gen_riscv_value_jump(&kval->kind.data.jump);
        break;
    case KOOPA_RVT_CALL:
        gen_riscv_value_call(&kval->kind.data.call, kval->ty->tag == KOOPA_RTT_UNIT ? -1 : addr);
        break;
    case KOOPA_RVT_RETURN:
        gen_riscv_value_return(&kval->kind.data.ret);
        break;
    }
}

void RISCVBuilder::traversal_raw_slice(const koopa_raw_slice_t *rs)
{
    for (uint32_t i = 0; i < rs->len; i++)
    {
        const void *data = rs->buffer[i];
        switch (rs->kind)
        {
        case KOOPA_RSIK_FUNCTION:
            gen_riscv_func((koopa_raw_function_t)data);
            break;
        case KOOPA_RSIK_BASIC_BLOCK:
            gen_riscv_block((koopa_raw_basic_block_t)data);
            break;
        case KOOPA_RSIK_VALUE:
            gen_riscv_value((koopa_raw_value_t)data);
            break;
        }
    }
}

void RISCVBuilder::build(const koopa_raw_program_t *raw)
{
    output << ".data" << endl;
    traversal_raw_slice(&raw->values);
    output << ".text" << endl;
    traversal_raw_slice(&raw->funcs);
}
