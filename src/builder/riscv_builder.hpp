#pragma once

#include <iostream>
#include <string>
#include <map>

#include <koopa.h>

using std::ostream, std::endl, std::map;

class RISCVBuilder
{
    class Env
    {
        int total_size;
        int cur;
        map<koopa_raw_value_t, int> addr;

    public:
        void NewEnv(int size)
        {
            total_size = cur = size;
            addr.clear();
        }
        int GetTotalSize()
        {
            return total_size;
        }
        int GetAddr(koopa_raw_value_t kval)
        {
            if (addr.count(kval))
                return addr[kval];
            int t = calc_inst_size(kval);
            if (t == 0)
                return 2333333;
            cur -= t;
            addr[kval] = cur;
            return cur;
        }
    };

    Env env;
    ostream &output;

    static int calc_func_size(koopa_raw_function_t kfunc);
    static int calc_blk_size(koopa_raw_basic_block_t kblk);
    static int calc_inst_size(koopa_raw_value_t kval);

    void traversal_raw_slice(const koopa_raw_slice_t *rs);
    void gen_riscv_func(koopa_raw_function_t kfunc);
    void gen_riscv_block(koopa_raw_basic_block_t kblk);
    void gen_riscv_value(koopa_raw_value_t kval);

    void load_to_reg(koopa_raw_value_t kval, const char *reg);
    void gen_riscv_value_load(const koopa_raw_load_t *kload, int addr);
    void gen_riscv_value_store(const koopa_raw_store_t *kstore);
    void gen_riscv_value_binary(const koopa_raw_binary_t *kbinary, int addr);
    void gen_riscv_value_branch(const koopa_raw_branch_t *kbranch);
    void gen_riscv_value_jump(const koopa_raw_jump_t *kjump);
    void gen_riscv_value_return(const koopa_raw_return_t *kret);

public:
    RISCVBuilder(ostream &_out) : output(_out) {}

    void build(const koopa_raw_program_t *raw);
};
