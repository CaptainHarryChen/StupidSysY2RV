#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>

#include "koopa_util.hpp"

class BlockMaintainer
{
    std::vector<const void *> current_insts_buf;
    std::vector<const void *> *basic_block_buf;

public:
    void SetBasicBlockBuf(std::vector<const void *> *_basic_block_buf)
    {
        basic_block_buf = _basic_block_buf;
    }
    void FinishCurrentBlock()
    {
        if (basic_block_buf->size() > 0)
        {
            koopa_raw_basic_block_data_t *last_block = (koopa_raw_basic_block_data_t *)(*basic_block_buf)[basic_block_buf->size() - 1];
            for (size_t i = 0; i < current_insts_buf.size(); i++)
            {
                koopa_raw_value_t t = (koopa_raw_value_t)current_insts_buf[i];
                if (t->kind.tag == KOOPA_RVT_BRANCH || t->kind.tag == KOOPA_RVT_RETURN || t->kind.tag == KOOPA_RVT_JUMP)
                {
                    current_insts_buf.resize(i + 1);
                    break;
                }
            }
            if (!last_block->insts.buffer)
                last_block->insts = make_koopa_rs_from_vector(current_insts_buf, KOOPA_RSIK_VALUE);
        }
        current_insts_buf.clear();
    }
    void AddNewBasicBlock(koopa_raw_basic_block_data_t *basic_block)
    {
        FinishCurrentBlock();
        basic_block->insts.buffer = nullptr;
        basic_block_buf->push_back(basic_block);
    }
    void AddInst(const void *inst)
    {
        current_insts_buf.push_back(inst);
    }
};
