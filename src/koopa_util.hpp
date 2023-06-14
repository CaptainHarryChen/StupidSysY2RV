#pragma once

#include <string>
#include <vector>
#include <koopa.h>

koopa_raw_slice_t empty_koopa_rs(koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);
koopa_raw_slice_t make_koopa_rs_from_vector(const std::vector<const void*> &vec, koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);
koopa_raw_slice_t make_koopa_rs_single_element(const void *ele, koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);
koopa_raw_slice_t add_element_to_koopa_rs(koopa_raw_slice_t origin, const void *ele);

koopa_raw_type_kind* make_array_type(const std::vector<int> &sz, int st_pos = 0);
koopa_raw_type_kind* simple_koopa_raw_type_kind(koopa_raw_type_tag_t tag);
koopa_raw_type_kind* make_int_pointer_type();

koopa_raw_value_data *make_koopa_interger(int x);
koopa_raw_value_data *JumpInst(koopa_raw_basic_block_t target);
koopa_raw_value_data *AllocIntInst(const std::string &name);
koopa_raw_value_data *AllocType(const std::string &name, koopa_raw_type_t ty);
koopa_raw_value_data *ZeroInit(koopa_raw_type_kind *_type = nullptr);

char *new_char_arr(std::string str);