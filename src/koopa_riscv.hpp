#pragma once

#include <string>
#include <koopa.h>

std::string gen_riscv_from_koopa_raw_program(const koopa_raw_program_t *raw);

void gen_riscv_func(koopa_raw_function_t kfunc, std::string &res);
void gen_riscv_block(koopa_raw_basic_block_t kblk, std::string &res);
void gen_riscv_value(koopa_raw_value_t kval, std::string &res);
void traversal_raw_slice(const koopa_raw_slice_t *rs, std::string &res);
