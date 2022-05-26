

#include "system/includes.h"
#include "media/includes.h"

#include "app_config.h"
#include "app_task.h"

#include "btstack/avctp_user.h"
#include "btstack/btstack_task.h"
#include "btstack/bluetooth.h"
#include "btstack/btstack_error.h"
#include "btctrler/btctrler_task.h"
#include "classic/hci_lmp.h"

#include "bt/bt_tws.h"
#include "bt/bt_ble.h"
#include "bt/bt.h"
#include "bt/vol_sync.h"
#include "bt/bt_emitter.h"
#include "bt_common.h"
#include "aec_user.h"

#include "math.h"
#include "spp_user.h"


#include "app_chargestore.h"
#include "app_charge.h"
#include "app_main.h"
#include "app_power_manage.h"
#include "user_cfg.h"

#include "asm/pwm_led.h"
#include "asm/timer.h"
#include "asm/hwi.h"
#include "cpu.h"

#include "ui/ui_api.h"
#include "ui_manage.h"
#include "ui/ui_style.h"

#include "key_event_deal.h"
#include "clock_cfg.h"
#include "gSensor/gSensor_manage.h"
/* #include "soundcard/soundcard.h" */

#include "audio_dec.h"
/* #include "audio_reverb.h" */
#include "tone_player.h"
#include "dac.h"
#define LOG_TAG_CONST        BT
#define LOG_TAG             "[BT]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#define LOG_DUMP_ENABLE
#define LOG_CLI_ENABLE
#include "debug.h"

#if TCFG_APP_BT_EN
#define __this 	(&app_bt_hdl)

/*************************************************************

             此文件函数主要是蓝牙量产测试的函数处理

**************************************************************/

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙快速测试
   @param
   @return
   @note     样机和蓝牙测试盒链接开启快速测试，开启mic扩音功能，
   			 按键就播放按键音来检测硬件是否焊接正常
*/
/*----------------------------------------------------------------------------*/
void bt_fast_test_api(void)
{
    log_info("------------bt_fast_test_api---------\n");
    //进入快速测试模式，用户根据此标志判断测试，如测试按键-开按键音  、测试mic-开扩音 、关闭蓝牙进入powerdown、关闭可发现可连接
    bt_user_priv_var.fast_test_mode = 1;
#ifdef AUDIO_MIC_TEST
    mic_test_start();
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙模式样机被测试仪链接上的回调函数，把其他状态关闭
   @param
   @return
   @note
*/
/*----------------------------------------------------------------------------*/
void bt_dut_api(u8 value)
{
    log_info("bt in dut \n");
    sys_auto_shut_down_disable();
#if TCFG_USER_TWS_ENABLE
    extern 	void tws_cancle_all_noconn();
    tws_cancle_all_noconn() ;
#else
    sys_timer_del(app_var.auto_stop_page_scan_timer);
    extern void bredr_close_all_scan();
    bredr_close_all_scan();
#endif

#if TCFG_USER_BLE_ENABLE
    bt_ble_adv_enable(0);
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief    蓝牙模式进入定频状态
   @param
   @return
   @note     量产的时候可以通过按键等来触发进入定频状态，这时候蓝牙会在一个通道里
   			 发送信号,可以通过设置下面变量来设置定频的频点
		  	const int config_bredr_fcc_fix_fre = 0;
*/
/*----------------------------------------------------------------------------*/
void bt_fix_fre_api(u8 fre)
{
    log_info("bt in fix fre \n");
    bt_dut_api(0);

    bit_clr_ie(IRQ_BREDR_IDX);
    bit_clr_ie(IRQ_BT_CLKN_IDX);

    bredr_fcc_init(BT_FRE, fre);

}

extern void ble_fix_fre_api();
/*----------------------------------------------------------------------------*/
/**@brief    蓝牙量产串口控制处理
   @param
   @return
   @note     下面是蓝牙通过串口来控制进行量产测试的简单例子demo
			 命令格式可以查看<<杰理蓝牙串口量产控制.pdf>>文档命令、
			 客户可以按照需求自行添加修改
			 需要把uart_bt_product.c 里面代码流程开启或者自己调试串口发送接收


*/
/*----------------------------------------------------------------------------*/

#if 0
#include"asm/uart_dev.h"
#define  UART_HEAD  0x12345678


#define  GET_BT_ADDR     0x01    //获取小机蓝牙地址
#define  SET_BT_ADDR     0x02    //设置小机蓝牙地址
#define  SET_DUT         0x03    //设置小机进入dut模式，可以被仪器链接测试性能
#define  SET_BREDR_FRE   0x04    //设置小机进入bredr定频
#define  SET_BLE_FRE     0x05    //设置小机进入ble定频
#define  SAVE_OFFSET     0x06    //更新并且保存小机频偏,确定测试的频偏没有问题可以保存
#define  UPDATA_OFFSET   0x07    //更新小机频偏
#define  SET_RESET       0x08    //小机复位


static  uart_bus_t *product_uart_bus;
void set_bt_uart_bus(uart_bus_t *uart_bus)
{
    product_uart_bus = uart_bus;
}

u16 bt_product_checksum(u8 *data, u16 len)
{
    u16 sum = 0;
    u16 i ;
    for (i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}

void bt_product_get_addr(u8 cmd, u8 *addr)
{
    u16 sum = 0;
    u8 return_data[20];

    return_data[0] = (u8)(UART_HEAD >> 24);
    return_data[1] = (u8)(UART_HEAD >> 16);
    return_data[2] = (u8)(UART_HEAD >> 8);
    return_data[3] = (u8)UART_HEAD;

    return_data[4] = 7;   //len
    return_data[5] = cmd; //cmd

    memcpy(&return_data[6], addr, 6);
    sum = bt_product_checksum(return_data, 11);

    return_data[12] = sum >> 8;
    return_data[13] = sum;

    if (product_uart_bus) {
        product_uart_bus->write(return_data, 14);
    }
}

void bt_product_set_addr(u8 cmd, u8 *addr)
{
    u16 sum = 0;
    u8 return_data[20];

    /* put_buf(addr,6); */

////从新设置地址，然后要从新开启搜索发现等
    bt_update_mac_addr(addr);
#if TCFG_USER_TWS_ENABLE
    extern 	void tws_cancle_all_noconn();
    tws_cancle_all_noconn() ;
#else
    sys_timer_del(app_var.auto_stop_page_scan_timer);
    extern void bredr_close_all_scan();
    bredr_close_all_scan();
#endif

#if TCFG_USER_BLE_ENABLE
    bt_ble_adv_enable(0);
#endif
    lmp_hci_write_local_address(addr);

    user_send_cmd_prepare(USER_CTRL_WRITE_SCAN_ENABLE, 0, NULL);
    user_send_cmd_prepare(USER_CTRL_WRITE_CONN_ENABLE, 0, NULL);

    return_data[0] = (u8)(UART_HEAD >> 24);
    return_data[1] = (u8)(UART_HEAD >> 16);
    return_data[2] = (u8)(UART_HEAD >> 8);
    return_data[3] = (u8)UART_HEAD;

    return_data[4] = 1;   //len
    return_data[5] = cmd; //cmd

    sum = bt_product_checksum(return_data, 6);
    return_data[6] = sum >> 8;
    return_data[7] = sum;

    //// 更新协议栈 地址
    if (product_uart_bus) {
        product_uart_bus->write(return_data, 8);
    }
}

void bt_product_save_offset(u8 cmd, u8 *data)
{
    u32 offset = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    u16 sum = 0;
    u8 return_data[20];

    printf("%d \n", offset);

    bt_osc_offset_ext_save(offset);

    return_data[0] = (u8)(UART_HEAD >> 24);
    return_data[1] = (u8)(UART_HEAD >> 16);
    return_data[2] = (u8)(UART_HEAD >> 8);
    return_data[3] = (u8)UART_HEAD;

    return_data[4] = 1;    //len
    return_data[5] = cmd; //cmd

    sum = bt_product_checksum(return_data, 6);
    return_data[6] = sum >> 8;
    return_data[7] = sum;

    if (product_uart_bus) {
        product_uart_bus->write(return_data, 8);
    }
}


void bt_product_updata_offset(u8 cmd, u8 *data)
{
    u32 offset = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
    u16 sum = 0;
    u8 return_data[20];

    printf("%d \n", offset);

    bt_osc_offset_ext_updata(offset);

    return_data[0] = (u8)(UART_HEAD >> 24);
    return_data[1] = (u8)(UART_HEAD >> 16);
    return_data[2] = (u8)(UART_HEAD >> 8);
    return_data[3] = (u8)UART_HEAD;

    return_data[4] = 1;    //len
    return_data[5] = cmd; //cmd

    sum = bt_product_checksum(return_data, 6);
    return_data[6] = sum >> 8;
    return_data[7] = sum;

    if (product_uart_bus) {
        product_uart_bus->write(return_data, 8);
    }
}

void bt_product_test_uart(u8 *data, u8 len)
{
    u32 head = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

    /* y_printf(" head %x \n",head); */

    if (head != UART_HEAD) {
        return;
    }

    u8 param_len = data[4];
    u8 cmd  = data[5];
    u16 checksum = data[4 + 1 + param_len] << 8 | data[4 + 1 + param_len + 1];
    u16 checksum1 = bt_product_checksum(data, 4 + 1 + param_len);

    /* y_printf(" param_len=%d cmd=%x %x %x\n",param_len,cmd,checksum,checksum1); */

    switch (cmd) {
    case GET_BT_ADDR:
        printf("  GET_BT_ADDR  \n");
        bt_product_get_addr(cmd, bt_get_mac_addr());
        break;
    case SET_BT_ADDR:
        printf("  SET_BT_ADDR  \n");
        bt_product_set_addr(cmd, &data[6]);
        break;
    case SET_DUT:
        printf("  SET_DUT  \n");
        bredr_set_dut_enble(1, 0);
        break;
    case SET_BREDR_FRE:
        printf("  SET_BREDR_FRE  \n");
        bt_fix_fre_api(0);
        break;
    case SET_BLE_FRE:
        printf("  SET_BLE_FRE  \n");
        ble_fix_fre_api();
        break;
    case SAVE_OFFSET:
        printf("  SET_OFFSET  \n");
        bt_product_save_offset(cmd, &data[6]);
        break;
    case UPDATA_OFFSET:
        printf("  UPDATA_OFFSET  \n");
        bt_product_updata_offset(cmd, &data[6]);
        break;
    case SET_RESET:
        printf("  SET_RESET  \n");
        cpu_reset();
        break;
    }
}
#endif
#endif
