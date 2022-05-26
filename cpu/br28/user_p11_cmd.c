#include "app_config.h"
#include "includes.h"
#include "user_p11_cmd.h"
#include "asm/power/p11.h"
#include "data_export.h"
#include "data_export_to_file.h"

struct M2P11_RAM {
    u16 len;
    u8 *data;
};

static struct M2P11_RAM share_ram;

static u8  busy = 0;
static u8  online = 0;

static cbuffer_t *sensor_cbuffer[3];


void __share_ram_busy()
{
    local_irq_disable();
    if (!busy) {
        mpu_set(3, P11_RAM_BASE, P11_RAM_PROTECT_END - 1, 0, "CxrwPrw", get_dev_id("DBG_REV"));
    }
    busy ++;
    local_irq_enable();
}

void __share_ram_unbusy()
{
    local_irq_disable();
    if (busy) {
        busy--;
        if (!busy) {
            mpu_set(3, P11_RAM_BASE, P11_RAM_PROTECT_END - 1, 0, "0r", get_dev_id("DBG_REV"));
        }
    }
    local_irq_enable();
}


/**
 * @brief 注册命令0的响应函数
 */
static int config_m2p_ram_init(int type, u16 len, u8 *data)
{
    ASSERT(len == 8);
    int *msg = (int *)data;
    share_ram.len = msg[0];
    share_ram.data = (u8 *)(msg[1] + P11_RAM_BASE);
    P11_M2P_INT_IE |= BIT(M2P_USER_INDEX);
    online = 1;
    return TRUE;
}

P11_IRQ_EVENT_HANDLER(0, config_m2p_ram_init);


/**
 * @brief 注册命令1的响应函数
 */

static int config_m2p_cubuff_init(int type, u16 len, u8 *data)
{
    ASSERT(len == 8);
    int *msg = (int *)data;
    sensor_cbuffer[0] = msg[1];
    sensor_cbuffer[1] = (sensor_cbuffer[0] + 1);
    sensor_cbuffer[2] = (sensor_cbuffer[1] + 1);
    printf(">>>>>%s %d %x %x %x\n", __FUNCTION__, __LINE__, sensor_cbuffer[0], sensor_cbuffer[1], sensor_cbuffer[2]);
    return TRUE;
}

P11_IRQ_EVENT_HANDLER(1, config_m2p_cubuff_init);

/**
 * @brief 默认响应函数
 */

void p33_user_p2m_irq_handler_default(int type, u16 len, u8 *data)
{
    /* printf("%s %d >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", __func__, type); */
    //if (len) {
    //    put_buf(data, len);
    //}
}

/**
 * @brief p11 到master 主要响应入口
 */

void p33_user_p2m_irq_handler()
{
    u8 cmd;
    u8 type;
    u16 len;
    u8 *data;
    volatile u8 *pnd = (u8 *)(P2M_MESSAGE_RAM_BEGIN +  P2M_USER_PEND);
    cmd = pnd[1];
    len = pnd[2] | (pnd[3] << 8);
    data = (u8 *)((pnd[4] | (pnd[5] << 8) | (pnd[6] << 16) | (pnd[7] << 24)) + P11_RAM_BASE);

    struct p11_event_handler *p;

    /* printf("%s %d >>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", __func__, cmd); */
    printf("%s %d\n", __func__, cmd);
    __share_ram_busy();
    list_for_each_p11_event_handler(p) {
        if (p->event_type == cmd && p->handler) {
            if (p->handler(cmd, len, data)) {
                pnd[0]  = 0;
                __share_ram_unbusy();
                return ;
            }
        }
    }
    p33_user_p2m_irq_handler_default(cmd, len, data);
    __share_ram_unbusy();
    pnd[0]  = 0;

}

/**
 * @brief 用户主核发送数据到小核
 *
 * @param [in] cmd 命令
 * @param [in] len 数据长度(最大长度有限制，由小核配置)
 * @param [in] data 数据地址
 * @param [in] wait 是否等待传输结束
 */
void user_main_post_to_p11_system(u8 cmd, u16 len, u8 *data, u8 wait)
{
    volatile u8 *pnd = (u8 *)(M2P_MESSAGE_RAM_BEGIN +  M2P_USER_PEND);
    int time_out = 10000;
    if (!online) {
        return;
    }
    local_irq_disable();

    do {
        asm("csync");
        time_out--;
    } while (time_out && pnd[0]);
    ASSERT(time_out);
    pnd[0] = 1;
    local_irq_enable();

    ASSERT(len <= share_ram.len);

    __share_ram_busy();
    memcpy(share_ram.data, data, len);
    __share_ram_unbusy();
    /* mpu_set(3, P11_RAM_BASE, P11_RAM_PROTECT_END - 1, 0, "0r", get_dev_id("DBG_REV")); */
    pnd[0] = 1;
    pnd[1] = cmd;
    pnd[2] = (u8)(len & 0xff);
    pnd[3] = (u8)(len >> 8) & 0xff;
    pnd[4] = (u8)((int)share_ram.data - P11_RAM_BASE) & 0xff;
    pnd[5] = (u8)(((int)share_ram.data - P11_RAM_BASE) >> 8) & 0xff;
    pnd[6] = (u8)(((int)share_ram.data - P11_RAM_BASE) >> 16) & 0xff;
    pnd[7] = (u8)(((int)share_ram.data - P11_RAM_BASE) >> 24) & 0xff;
    P11_M2P_INT_SET |= BIT(M2P_USER_INDEX);
    time_out = 1000000;
    do {
        asm("csync");
        time_out--;
    } while (time_out && wait && pnd[0]);
    ASSERT(time_out);
}

/**
 * @brief 用户主核读小核数据缓存
 * @param [in] index 0:gsensor 1:hr sensor 2：spo2 sensor
 * @param [in] buf 数据地址
 * @param [in] len  数据长度
 */
u32 master_p11cbuf_read(int index, void *buf, u32 len)
{
    u32 flags;
    u32 r_len = len;
    u32 copy_len;
    u8 *read_ptr;

    cbuffer_t *cbuffer = sensor_cbuffer[index];

    if ((!cbuffer) || (!buf)) {
        return 0;
    }


    __share_ram_busy();
    cbuffer = (cbuffer_t *)(((int)(cbuffer)) + P11_RAM_BASE);

    u32 data_len = cbuffer->data_len;

    if ((u32)cbuffer->read_ptr >= (u32)cbuffer->end) {
        cbuffer->read_ptr = (u8 *)cbuffer->begin;
    }

    if (data_len < len) {
        memset(buf, 0, len);
        __share_ram_unbusy();
        return 0;
    }

    read_ptr = cbuffer->read_ptr;
    copy_len = (u32)cbuffer->end - (u32)read_ptr;
    if (copy_len > len) {
        copy_len = len;
    }
    len -= copy_len;
    memcpy(buf, read_ptr + P11_RAM_BASE, copy_len);
    if (len == 0) {
        read_ptr += copy_len;
    } else {
        memcpy((u8 *)buf + copy_len, cbuffer->begin + P11_RAM_BASE, len);
        read_ptr = cbuffer->begin + len;
    }
    cbuffer->data_len -= r_len;
    cbuffer->tmp_len -= r_len;
    cbuffer->read_ptr = read_ptr;
    __share_ram_unbusy();
    return r_len;
}

/**
 * @brief 用户主核查小核数据缓存
 * @param [in] index 0:gsensor 1:hr sensor 2：spo2 sensor
 * @param [out] len  数据长度
 */
u32 master_p11cbuf_get_data_len(int index)
{
    cbuffer_t *cbuffer = sensor_cbuffer[index];
    if (!cbuffer) {
        return 0;
    }
    __share_ram_busy();
    cbuffer = (cbuffer_t *)(((int)(cbuffer)) + P11_RAM_BASE);
    u32 len = cbuffer->data_len;
    __share_ram_unbusy();
    return len;
}



#if TCFG_SENSOR_DEBUG_ENABLE

#define FILE_END_COUNTER  (1000000)

void *sensor_de_file = NULL;
int sensor_debug_close(int *P)
{
    printf("%s\n", __func__);
    if (sensor_de_file) {
        data_export_file_close(sensor_de_file);
        sensor_de_file = NULL;
    }
    return 0;

}

static u8 de_to_file_idle(void)
{
    return !sensor_de_file;
}

REGISTER_LP_TARGET(de_to_file_lp_target) = {
    .name = "sensor_to_file",
    .is_idle = de_to_file_idle,
};




static int config_m2p_cubuff_kick(int type, u16 len, u8 *data)
{

    static int counter = 0;

    if (sensor_de_file) {
        u8 *rdata = NULL;
        int dlen = master_p11cbuf_get_data_len(0);

        if (dlen) {
            rdata = data_export_file_write_alloc(sensor_de_file, 0, dlen);
            /* printf("%x %d %d \n", (int)rdata, __LINE__, dlen); */
            master_p11cbuf_read(0, rdata, dlen);
            data_export_file_write_alloc_push(sensor_de_file, 0, rdata);
        }

        dlen = master_p11cbuf_get_data_len(1);
        if (dlen) {
            rdata = data_export_file_write_alloc(sensor_de_file, 1, dlen);
            /* printf("%x %d %d \n", (int)rdata, __LINE__, dlen); */
            master_p11cbuf_read(1, rdata, dlen);
            data_export_file_write_alloc_push(sensor_de_file, 1, rdata);
        }

        dlen = master_p11cbuf_get_data_len(2);
        if (dlen) {

            rdata = data_export_file_write_alloc(sensor_de_file, 2, dlen);
            /* printf("%x %d %d \n", (int)rdata, __LINE__, dlen); */
            master_p11cbuf_read(2, rdata, dlen);
            data_export_file_write_alloc_push(sensor_de_file, 2, rdata);
        }

        if (counter++ > FILE_END_COUNTER) {
            int argv[3];
            argv[0] = (int)sensor_debug_close;
            argv[1] = 1;
            argv[2] = 0;
            os_taskq_post_type("app_core", Q_CALLBACK, 3, argv);
        }

    }

    return TRUE;
}


P11_IRQ_EVENT_HANDLER(3, config_m2p_cubuff_kick);


#else
#ifdef CONFIG_WATCH_CASE_ENABLE

static int config_m2p_cubuff_kick(int type, u16 len, u8 *data)
{

    extern void sensor_kick_start();
    sensor_kick_start();
    return true;
}

P11_IRQ_EVENT_HANDLER(3, config_m2p_cubuff_kick);

#endif



#endif


