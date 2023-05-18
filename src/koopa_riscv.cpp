#include "koopa_riscv.hpp"
#include "koopa_value_riscv.hpp"

void gen_riscv_func(koopa_raw_function_t kfunc, std::string &res)
{
    const char *name = kfunc->name + 1;
    res += std::string(".globl ") + name + "\n";
    res += std::string(name) + ":\n";

    // TODO: ty ???
    // TODO: params ????

    // blocks
    traversal_raw_slice(&kfunc->bbs, res);
}

void gen_riscv_block(koopa_raw_basic_block_t kblk, std::string &res)
{
    //TODO: name
    //TODO: params
    //TODO: used_by
    traversal_raw_slice(&kblk->insts, res);
}

void gen_riscv_value(koopa_raw_value_t kval, std::string &res)
{
    //TODO: ty
    //TODO: name
    //TODO: used_by
    switch(kval->kind.tag)
    {
    case KOOPA_RVT_INTEGER:
        res += std::to_string(kval->kind.data.integer.value);
        break;
    case KOOPA_RVT_RETURN:
        gen_riscv_value_return((const koopa_raw_return_t *)&kval->kind.data.ret, res);
        break;
    }
}

void traversal_raw_slice(const koopa_raw_slice_t *rs, std::string &res)
{
    for (uint32_t i = 0; i < rs->len; i++)
    {
        const void *data = rs->buffer[i];
        switch (rs->kind)
        {
        case KOOPA_RSIK_FUNCTION:
            gen_riscv_func((koopa_raw_function_t)data, res);
            break;
        case KOOPA_RSIK_BASIC_BLOCK:
            gen_riscv_block((koopa_raw_basic_block_t)data, res);
            break;
        case KOOPA_RSIK_VALUE:
            gen_riscv_value((koopa_raw_value_t)data, res);
            break;
        }
    }
}

std::string gen_riscv_from_koopa_raw_program(const koopa_raw_program_t *raw)
{
    std::string res;
    res.reserve(1024000);
    res += ".text\n";
    traversal_raw_slice(&raw->funcs, res);
    return res;
}
