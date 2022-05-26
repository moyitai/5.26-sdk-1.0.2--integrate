#include "smartbox/config.h"
#include "sport_data_training_load.h"
#include "sport_data_func.h"

#if (SMART_BOX_EN && JL_SMART_BOX_SENSORS_DATA_OPT)

#define TRAINING_LOAD_DATA_MAX_LEN		0
static u8 *training_load_data = NULL;

u32 sport_data_func_attr_training_load_get(void *priv, u8 attr, u8 sub_mask, u8 *buf, u16 buf_size, u32 offset)
{
    u32 rlen = 0;
    u16 data_len = 0;

    rlen = add_one_attr_with_submask_huge(buf, buf_size, offset, attr, sub_mask, training_load_data, data_len);
    // 如果需要每次都free某一变量，就把概变量放到概函数最后一个参数中，并以二级指针的方式传入
    // 如果这个指针是NULL，或者直接传入NULL，都不会free
    sport_data_global_var_deal(priv, attr, sub_mask, training_load_data, data_len, rlen, (void *)&training_load_data);
    return rlen;
}

#endif
