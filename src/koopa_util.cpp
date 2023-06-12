#include "koopa_util.hpp"

#include <cassert>
#include <cstring>

koopa_raw_slice_t empty_koopa_rs(koopa_raw_slice_item_kind_t kind)
{
    koopa_raw_slice_t res;
    res.buffer = nullptr;
    res.kind = kind;
    res.len = 0;
    return res;
}

koopa_raw_slice_t make_koopa_rs_from_vector(const std::vector<const void *> &vec, koopa_raw_slice_item_kind_t kind)
{
    koopa_raw_slice_t res;
    res.buffer = new const void *[vec.size()];
    std::copy(vec.begin(), vec.end(), res.buffer);
    res.kind = kind;
    res.len = vec.size();
    return res;
}

koopa_raw_slice_t make_koopa_rs_single_element(const void *ele, koopa_raw_slice_item_kind_t kind)
{
    koopa_raw_slice_t res;
    res.buffer = new const void *[1];
    res.buffer[0] = ele;
    res.kind = kind;
    res.len = 1;
    return res;
}

koopa_raw_slice_t add_element_to_koopa_rs(koopa_raw_slice_t origin, const void *ele)
{
    koopa_raw_slice_t res;
    res.buffer = new const void *[origin.len + 1];
    memcpy(res.buffer, origin.buffer, sizeof(void *) * origin.len);
    res.buffer[origin.len] = ele;
    res.len = origin.len + 1;
    res.kind = origin.kind;
    delete origin.buffer;

    return res;
}

koopa_raw_type_kind *simple_koopa_raw_type_kind(koopa_raw_type_tag_t tag)
{
    assert(tag == KOOPA_RTT_INT32 || tag == KOOPA_RTT_UNIT);
    koopa_raw_type_kind *res = new koopa_raw_type_kind();
    res->tag = tag;
    return res;
}

koopa_raw_type_kind* make_int_pointer_type()
{
    koopa_raw_type_kind *res = new koopa_raw_type_kind();
    res->tag = KOOPA_RTT_POINTER;
    res->data.pointer.base = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
    return res;
}

koopa_raw_value_data *JumpInst(koopa_raw_basic_block_t target)
{
    koopa_raw_value_data *res = new koopa_raw_value_data();
    res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_UNIT);
    res->name = nullptr;
    res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
    res->kind.tag = KOOPA_RVT_JUMP;
    res->kind.data.jump.args = empty_koopa_rs(KOOPA_RSIK_VALUE);
    res->kind.data.jump.target = target;
    return res;
}

koopa_raw_value_data *AllocIntInst(const std::string &name)
{
    koopa_raw_value_data *res = new koopa_raw_value_data();
    res->ty = make_int_pointer_type();
    res->name = new_char_arr(name);
    res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
    res->kind.tag = KOOPA_RVT_ALLOC;
    return res;
}

koopa_raw_value_data *ZeroInit()
{
    koopa_raw_value_data *res = new koopa_raw_value_data();
    res->ty = simple_koopa_raw_type_kind(KOOPA_RTT_INT32);
    res->name = nullptr;
    res->used_by = empty_koopa_rs(KOOPA_RSIK_VALUE);
    res->kind.tag = KOOPA_RVT_ZERO_INIT;
    return res;
}
