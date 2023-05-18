#pragma once

#include <vector>
#include <koopa.h>

koopa_raw_slice_t empty_koopa_rs(koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);
koopa_raw_slice_t make_koopa_rs_from_vector(const std::vector<const void*> &vec, koopa_raw_slice_item_kind_t kind = KOOPA_RSIK_UNKNOWN);

koopa_raw_type_kind* simple_koopa_raw_type_kind(koopa_raw_type_tag_t tag);