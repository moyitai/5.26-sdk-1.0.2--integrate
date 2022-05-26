/*********************************************************************************************
    *   Filename        : le_server_module.c

    *   Description     :

    *   Author          :

    *   Email           : zh-jieli.com

    *   Last modifiled  : 2017-01-17 11:14

    *   Copyright:(c)JIELI  2011-2016  @ , All Rights Reserved.
*********************************************************************************************/

// *****************************************************************************
/* EXAMPLE_START(le_counter): LE Peripheral - Heartbeat Counter over GATT
 *
 * @text All newer operating systems provide GATT Client functionality.
 * The LE Counter examples demonstrates how to specify a minimal GATT Database
 * with a custom GATT Service and a custom Characteristic that sends periodic
 * notifications.
 */
// *****************************************************************************
#include "system/app_core.h"
#include "system/includes.h"
#include "smartbox/config.h"

#include "app_action.h"

#include "btstack/btstack_task.h"
#include "btstack/bluetooth.h"
#include "user_cfg.h"
#include "vm.h"
#include "app_power_manage.h"
#include "btcontroller_modules.h"
#include "bt_common.h"
#include "3th_profile_api.h"
#include "btstack/avctp_user.h"
#include "app_chargestore.h"

#include "btcrypt.h"
#include "custom_cfg.h"
#include "smartbox_music_info_setting.h"
#include "le_smartbox_adv.h"
#include "le_smartbox_module.h"
#include "classic/tws_api.h"
#include "smartbox_update.h"

#if (TCFG_BLE_DEMO_SELECT == DEF_BLE_DEMO_RCSP_DEMO)

//TRANS ANCS
#define TRANS_ANCS_EN  			  	 1

#if (defined(TCFG_UI_ENABLE_NOTICE) && (!TCFG_UI_ENABLE_NOTICE))
#undef TRANS_ANCS_EN
#define TRANS_ANCS_EN  			  	 0
#endif

#if TRANS_ANCS_EN
#include "btstack/btstack_event.h"
#endif

#if 1
extern void printf_buf(u8 *buf, u32 len);
#define log_info          printf
#define log_info_hexdump  printf_buf
#else
#define log_info(...)
#define log_info_hexdump(...)
#endif

#define EIR_TAG_STRING   0xd6, 0x05, 0x08, 0x00, 'J', 'L', 'A', 'I', 'S', 'D','K'
static const char user_tag_string[] = {EIR_TAG_STRING};

//------
#define ATT_LOCAL_PAYLOAD_SIZE    (200)                   //note: need >= 20
#define ATT_SEND_CBUF_SIZE        (512)                   //note: need >= 20,缓存大小，可修改
#define ATT_RAM_BUFSIZE           (ATT_CTRL_BLOCK_SIZE + ATT_LOCAL_PAYLOAD_SIZE + ATT_SEND_CBUF_SIZE)                   //note:
static u8 att_ram_buffer[ATT_RAM_BUFSIZE] __attribute__((aligned(4)));
//---------------

//---------------
#define ADV_INTERVAL_MIN          (160*5) //n*0.625ms
/* #define ADV_INTERVAL_MIN          (0x30) */

static hci_con_handle_t con_handle;

//加密设置
static const uint8_t sm_min_key_size = 7;

//连接参数设置
static const uint8_t connection_update_enable = 1; ///0--disable, 1--enable
static uint8_t connection_update_cnt = 0; //
static const struct conn_update_param_t connection_param_table[] = {
    {16, 24, 16, 600},
    {12, 28, 14, 600},//11
    {8,  20, 20, 600},//3.7
    /* {12, 28, 4, 600},//3.7 */
    /* {12, 24, 30, 600},//3.05 */
};

static const struct conn_update_param_t connection_param_table_update[] = {
    {96, 120, 0,  600},
    {60,  80, 0,  600},
    {60,  80, 0,  600},
    {8,   20, 0,  600},
};

static u8 conn_update_param_index = 0;
static u8 conn_param_len = sizeof(connection_param_table) / sizeof(struct conn_update_param_t);
#define CONN_PARAM_TABLE_CNT      (sizeof(connection_param_table)/sizeof(struct conn_update_param_t))
#define CONN_TRY_NUM			  10 // 重复尝试次数

#if (ATT_RAM_BUFSIZE < 64)
#error "adv_data & rsp_data buffer error!!!!!!!!!!!!"
#endif


static char *gap_device_name = "AC697N_ble_test";
static u8 gap_device_name_len = 0;
volatile static u8 ble_work_state = 0;
static u8 test_read_write_buf[4];
static u8 adv_ctrl_en;
static u8 ble_init_flag;


static void (*app_recieve_callback)(void *priv, void *buf, u16 len) = NULL;
static void (*app_ble_state_callback)(void *priv, ble_state_e state) = NULL;
static void (*ble_resume_send_wakeup)(void) = NULL;
static u32 channel_priv;

extern u8 get_rcsp_connect_status(void);

const char *const phy_result[] = {
    "None",
    "1M",
    "2M",
    "Coded"
};
//------------------------------------------------------
//ANCS
#if TRANS_ANCS_EN
#define ANCS_MESSAGE_MANAGE_EN                                      1
#define ANCS_SUBEVENT_CLIENT_NOTIFICATION                           0xF1
void ancs_client_init(void);
void ancs_client_register_callback(btstack_packet_handler_t callback);
const char *ancs_client_attribute_name_for_id(int id);
void ancs_set_notification_buffer(u8 *buffer, u16 buffer_size);
u32 get_notification_uid(void);
u16 get_controlpoint_handle(void);

//ancs info buffer
#define ANCS_INFO_BUFFER_SIZE  (1024)
static u8 ancs_info_buffer[ANCS_INFO_BUFFER_SIZE];
#else
#define ANCS_MESSAGE_MANAGE_EN                                      0
#endif

//------------------------------------------------------

static int app_send_user_data_check(u16 len);
static int app_send_user_data_do(void *priv, u8 *data, u16 len);
static int app_send_user_data(u16 handle, u8 *data, u16 len, u8 handle_type);
//------------------------------------------------------
//广播参数设置
static void advertisements_setup_init();
extern const char *bt_get_local_name();
static int set_adv_enable(void *priv, u32 en);
static int get_buffer_vaild_len(void *priv);
//------------------------------------------------------
static void send_request_connect_parameter(u8 table_index)
{
    struct conn_update_param_t *param = NULL; //static ram
    switch (conn_update_param_index) {
    case 0:
        param = (void *)&connection_param_table[table_index];
        break;
    case 1:
        param = (void *)&connection_param_table_update[table_index];
        break;
    default:
        break;
    }

    log_info("update_request:-%d-%d-%d-%d-\n", param->interval_min, param->interval_max, param->latency, param->timeout);
    if (con_handle) {
        ble_user_cmd_prepare(BLE_CMD_REQ_CONN_PARAM_UPDATE, 2, con_handle, param);
    }
}

static void check_connetion_updata_deal(void)
{
    if (connection_update_enable) {
        if (connection_update_cnt < (CONN_PARAM_TABLE_CNT * CONN_TRY_NUM)) {
            send_request_connect_parameter(connection_update_cnt / CONN_TRY_NUM);
        }
    }
}

void notify_update_connect_parameter(u8 table_index)
{
    u8 conn_update_param_index_record = conn_update_param_index;
    if ((u8) - 1 != table_index) {
        conn_update_param_index = 1;
        send_request_connect_parameter(table_index);
    } else {
        if (connection_update_cnt >= (CONN_PARAM_TABLE_CNT * CONN_TRY_NUM)) {
            log_info("connection_update_cnt >= CONN_PARAM_TABLE_CNT");
            connection_update_cnt = 0;
        }
        send_request_connect_parameter(connection_update_cnt / CONN_TRY_NUM);
    }
    conn_update_param_index = conn_update_param_index_record;
}

static void connection_update_complete_success(u8 *packet)
{
    int con_handle, conn_interval, conn_latency, conn_timeout;

    con_handle = hci_subevent_le_connection_update_complete_get_connection_handle(packet);
    conn_interval = hci_subevent_le_connection_update_complete_get_conn_interval(packet);
    conn_latency = hci_subevent_le_connection_update_complete_get_conn_latency(packet);
    conn_timeout = hci_subevent_le_connection_update_complete_get_supervision_timeout(packet);

    log_info("conn_interval = %d\n", conn_interval);
    log_info("conn_latency = %d\n", conn_latency);
    log_info("conn_timeout = %d\n", conn_timeout);
}


static void set_ble_work_state(ble_state_e state)
{
    if (state != ble_work_state) {
        log_info("ble_work_st:%x->%x\n", ble_work_state, state);
        ble_work_state = state;
        if (app_ble_state_callback) {
            app_ble_state_callback((void *)channel_priv, state);
        }
    }
}

static ble_state_e get_ble_work_state(void)
{
    return ble_work_state;
}

ble_state_e smartbox_get_ble_work_state(void)
{
    return ble_work_state;
}

static void cbk_sm_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    sm_just_event_t *event = (void *)packet;
    u32 tmp32;
    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {
        case SM_EVENT_JUST_WORKS_REQUEST:
            sm_just_works_confirm(sm_event_just_works_request_get_handle(packet));
            log_info("Just Works Confirmed.\n");
            break;
        case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
            log_info_hexdump(packet, size);
            memcpy(&tmp32, event->data, 4);
            log_info("Passkey display: %06u.\n", tmp32);
            break;
        }
        break;
    }
}

static void can_send_now_wakeup(void)
{
    putchar('E');
    if (ble_resume_send_wakeup) {
        ble_resume_send_wakeup();
    }

}

_WEAK_
u8 ble_update_get_ready_jump_flag(void)
{
    return 0;
}

/*
 * @section Packet Handler
 *
 * @text The packet handler is used to:
 *        - stop the counter after a disconnect
 *        - send a notification when the requested ATT_EVENT_CAN_SEND_NOW is received
 */


/* LISTING_START(packetHandler): Packet Handler */
static void cbk_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    int mtu;
    u32 tmp;
    const char *attribute_name;

    switch (packet_type) {
    case HCI_EVENT_PACKET:
        switch (hci_event_packet_get_type(packet)) {

        /* case DAEMON_EVENT_HCI_PACKET_SENT: */
        /* break; */
        case ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE:
            log_info("ATT_EVENT_HANDLE_VALUE_INDICATION_COMPLETE\n");
        case ATT_EVENT_CAN_SEND_NOW:
            can_send_now_wakeup();
            break;

        case HCI_EVENT_LE_META:
            switch (hci_event_le_meta_get_subevent_code(packet)) {
            case HCI_SUBEVENT_LE_CONNECTION_COMPLETE: {
                set_app_connect_type(TYPE_BLE);
                con_handle = little_endian_read_16(packet, 4);
                log_info("HCI_SUBEVENT_LE_CONNECTION_COMPLETE: %0x\n", con_handle);
                connection_update_complete_success(packet + 8);
                ble_user_cmd_prepare(BLE_CMD_ATT_SEND_INIT, 4, con_handle, att_ram_buffer, ATT_RAM_BUFSIZE, ATT_LOCAL_PAYLOAD_SIZE);

#if (0 == BT_CONNECTION_VERIFY)
                JL_rcsp_auth_reset();
#endif
                bt_3th_type_dev_select(RCSP_BLE);
                set_ble_work_state(BLE_ST_CONNECT);
                set_adv_notify_state(1);
            }
            break;

            case HCI_SUBEVENT_LE_CONNECTION_UPDATE_COMPLETE:
                connection_update_complete_success(packet);
                break;

            case HCI_SUBEVENT_LE_PHY_UPDATE_COMPLETE:
                log_info("APP HCI_SUBEVENT_LE_PHY_UPDATE %s\n", hci_event_le_meta_get_phy_update_complete_status(packet) ? "Fail" : "Succ");
                log_info("Tx PHY: %s\n", phy_result[hci_event_le_meta_get_phy_update_complete_tx_phy(packet)]);
                log_info("Rx PHY: %s\n", phy_result[hci_event_le_meta_get_phy_update_complete_rx_phy(packet)]);
                break;
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            log_info("HCI_EVENT_DISCONNECTION_COMPLETE: %0x\n", packet[5]);
            con_handle = 0;
#if RCSP_ADV_MUSIC_INFO_ENABLE
            extern void stop_get_music_timer(u8 en);
            stop_get_music_timer(1);
#endif
            /* #if (0 == BT_CONNECTION_VERIFY) */
            /*             JL_rcsp_auth_reset(); */
            /* #endif */
            ble_user_cmd_prepare(BLE_CMD_ATT_SEND_INIT, 4, con_handle, 0, 0, 0);
            set_ble_work_state(BLE_ST_DISCONN);
            set_app_connect_type(TYPE_NULL);
#if RCSP_UPDATE_EN
            if (get_jl_update_flag()) {
                break;
            }
#endif
            if (!ble_update_get_ready_jump_flag()) {
#if TCFG_USER_TWS_ENABLE
                if (!(tws_api_get_tws_state() & TWS_STA_SIBLING_CONNECTED)) {
                    printf(")))))))) 1\n");
                    bt_ble_adv_enable(1);
                } else {
                    printf("))))))))>>>> %d\n", tws_api_get_role());
                    if (tws_api_get_role() == TWS_ROLE_MASTER) {
                        bt_ble_adv_enable(1);
                    }
                }
#else
                bt_ble_adv_enable(1);
#endif
            }
            connection_update_cnt = 0;
            break;

        case ATT_EVENT_MTU_EXCHANGE_COMPLETE:
            mtu = att_event_mtu_exchange_complete_get_MTU(packet) - 3;
            log_info("ATT MTU = %u\n", mtu);
            ble_user_cmd_prepare(BLE_CMD_ATT_MTU_SIZE, 1, mtu);
            break;

        case HCI_EVENT_VENDOR_REMOTE_TEST:
            log_info("--- HCI_EVENT_VENDOR_REMOTE_TEST\n");
            break;

        case L2CAP_EVENT_CONNECTION_PARAMETER_UPDATE_RESPONSE:
            tmp = little_endian_read_16(packet, 4);
            log_info("-update_rsp: %02x\n", tmp);
            if (tmp) {
                connection_update_cnt++;
                log_info("remoter reject!!!\n");
                check_connetion_updata_deal();
            } else {
                connection_update_cnt = (CONN_PARAM_TABLE_CNT * CONN_TRY_NUM);
            }
            break;

        case HCI_EVENT_ENCRYPTION_CHANGE:
            log_info("HCI_EVENT_ENCRYPTION_CHANGE= %d\n", packet[2]);
            check_connetion_updata_deal();
            break;

#if TRANS_ANCS_EN
        case HCI_EVENT_ANCS_META:
            switch (hci_event_ancs_meta_get_subevent_code(packet)) {
            case ANCS_SUBEVENT_CLIENT_NOTIFICATION:
                printf("ANCS_SUBEVENT_CLIENT_NOTIFICATION \n");
                attribute_name = ancs_client_attribute_name_for_id(ancs_subevent_client_notification_get_attribute_id(packet));
                if (!attribute_name) {
                    printf("ancs unknow attribute_id :%d \n", ancs_subevent_client_notification_get_attribute_id(packet));
                    break;
                } else {
                    u16 attribute_strlen = little_endian_read_16(packet, 7);
                    u8 *attribute_str = (void *)little_endian_read_32(packet, 9);
                    r_printf("Notification: %s - %s \n", attribute_name, attribute_str);
#if ANCS_MESSAGE_MANAGE_EN
                    extern void notice_set_info_from_ancs(void *name, void *data, u16 len);
                    notice_set_info_from_ancs(attribute_name, attribute_str, attribute_strlen);
#endif
                }
                break;
            default:
                break;
            }

            break;
#endif
        }
        break;
    }
}

void ancs_update_status(u8 status)
{
    switch (status) {
    case 1:
        g_printf("ancs trunk start \n");
        break;
    case 2:
        g_printf("ancs trunk end \n");
#if ANCS_MESSAGE_MANAGE_EN
        extern void notice_add_info_from_ancs();
        notice_add_info_from_ancs();
#endif
        break;
    default:
        break;
    }
}

/* LISTING_END */

/*
 * @section ATT Read
 *
 * @text The ATT Server handles all reads to constant data. For dynamic data like the custom characteristic, the registered
 * att_read_callback is called. To handle long characteristics and long reads, the att_read_callback is first called
 * with buffer == NULL, to request the total value length. Then it will be called again requesting a chunk of the value.
 * See Listing attRead.
 */


static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{

    uint16_t  att_value_len = 0;
    uint16_t handle = att_handle;

    /* printf("READ Callback, handle %04x, offset %u, buffer size %u\n", handle, offset, buffer_size); */

    log_info("read_callback, handle= 0x%04x,buffer= %08x\n", handle, (u32)buffer);

    switch (handle) {
    case ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE:
        att_value_len = strlen(gap_device_name);

        if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
            break;
        }

        if (buffer) {
            memcpy(buffer, &gap_device_name[offset], buffer_size);
            att_value_len = buffer_size;
            log_info("\n------read gap_name: %s \n", gap_device_name);
        }
        break;

    case ATT_CHARACTERISTIC_ae10_01_VALUE_HANDLE:
        att_value_len = sizeof(test_read_write_buf);
        if ((offset >= att_value_len) || (offset + buffer_size) > att_value_len) {
            break;
        }

        if (buffer) {
            memcpy(buffer, &test_read_write_buf[offset], buffer_size);
            att_value_len = buffer_size;
        }
        break;


    case ATT_CHARACTERISTIC_ae04_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae05_01_CLIENT_CONFIGURATION_HANDLE:
        if (buffer) {
            buffer[0] = att_get_ccc_config(handle);
            buffer[1] = 0;
        }
        att_value_len = 2;
        break;

    default:
        break;
    }

    log_info("att_value_len= %d\n", att_value_len);
    return att_value_len;

}


/* LISTING_END */
/*
 * @section ATT Write
 *
 * @text The only valid ATT write in this example is to the Client Characteristic Configuration, which configures notification
 * and indication. If the ATT handle matches the client configuration handle, the new configuration value is stored and used
 * in the heartbeat handler to decide if a new value should be sent. See Listing attWrite.
 */

/* LISTING_START(attWrite): ATT Write */
static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    int result = 0;
    u16 tmp16;

    u16 handle = att_handle;

    /* log_info("write_callback, handle= 0x%04x\n", handle); */

    switch (handle) {

    case ATT_CHARACTERISTIC_2a00_01_VALUE_HANDLE:
        tmp16 = BT_NAME_LEN_MAX;
        if ((offset >= tmp16) || (offset + buffer_size) > tmp16) {
            break;
        }

        if (offset == 0) {
            memset(gap_device_name, 0x00, BT_NAME_LEN_MAX);
        }
        memcpy(&gap_device_name[offset], buffer, buffer_size);
        log_info("\n------write gap_name:");
        break;

    case ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae04_01_CLIENT_CONFIGURATION_HANDLE:
    case ATT_CHARACTERISTIC_ae05_01_CLIENT_CONFIGURATION_HANDLE:
#if (0 == BT_CONNECTION_VERIFY)
        JL_rcsp_auth_reset();
#endif
        set_ble_work_state(BLE_ST_NOTIFY_IDICATE);
        check_connetion_updata_deal();
        log_info("\n------write ccc:%04x, %02x\n", handle, buffer[0]);
        att_set_ccc_config(handle, buffer[0]);
        can_send_now_wakeup();
        break;

    case ATT_CHARACTERISTIC_ae10_01_VALUE_HANDLE:
        tmp16 = sizeof(test_read_write_buf);
        if ((offset >= tmp16) || (offset + buffer_size) > tmp16) {
            break;
        }
        memcpy(&test_read_write_buf[offset], buffer, buffer_size);
        break;

    case ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE:
        /* if (!JL_rcsp_get_auth_flag()) { */
        /*     JL_rcsp_auth_recieve(buffer, buffer_size); */
        /*     break; */
        /* } */
        if (app_recieve_callback) {
            app_recieve_callback(0, buffer, buffer_size);
        }
        //		JL_rcsp_auth_recieve(data, len);
        break;

    case ATT_CHARACTERISTIC_ae03_01_VALUE_HANDLE:
        break;

    default:
        break;
    }

    return 0;
}

static int app_send_user_data(u16 handle, u8 *data, u16 len, u8 handle_type)
{
    u32 ret = APP_BLE_NO_ERROR;

    if (!con_handle) {
        return APP_BLE_OPERATION_ERROR;
    }
    ret = ble_user_cmd_prepare(BLE_CMD_ATT_SEND_DATA, 4, handle, data, len, handle_type);
    if (ret == BLE_BUFFER_FULL) {
        ret = APP_BLE_BUFF_FULL;
    }

    if (ret) {
        log_info("app_send_fail:%d !!!!!!\n", ret);
    }
    return ret;
}


static int make_set_adv_data(void)
{
    return smartbox_make_set_adv_data();
}

static int make_set_rsp_data(void)
{
    return smartbox_make_set_rsp_data();
}


u8 *ble_get_gatt_profile_data(u16 *len)
{
    *len = sizeof(profile_data);
    return (u8 *)profile_data;
}

//广播参数设置
static void advertisements_setup_init()
{
    uint8_t adv_type = 0;
    uint8_t adv_channel = 7;
    int   ret = 0;
    u16 adv_interval = ADV_INTERVAL_MIN;//0x30;

    ble_user_cmd_prepare(BLE_CMD_ADV_PARAM, 3, adv_interval, adv_type, adv_channel);

    ret |= make_set_adv_data();
    ret |= make_set_rsp_data();

    if (ret) {
        puts("advertisements_setup_init fail !!!!!!\n");
        return;
    }

}


void ble_sm_setup_init(io_capability_t io_type, u8 auth_req, uint8_t min_key_size, u8 security_en)
{
    //setup SM: Display only
    sm_init();
    sm_set_io_capabilities(io_type);
    sm_set_authentication_requirements(auth_req);
    sm_set_encryption_key_size_range(min_key_size, 16);
    sm_set_request_security(security_en);
    sm_event_callback_set(&cbk_sm_packet_handler);
}

void ancs_remove_message(u8 *packet, u16 size)
{
#if ANCS_MESSAGE_MANAGE_EN
    u8 *value;
    u32 ancs_notification_uid;
    value = &packet[8];
    ancs_notification_uid = little_endian_read_32(value, 4);
    if (value[0] == 2) { //0:added 1:modifiled 2:removed
        y_printf("remove message:ancs_notification_uid %04x", ancs_notification_uid);
        extern void notice_remove_info_from_ancs(u32 uid);
        notice_remove_info_from_ancs(ancs_notification_uid);
    } else if (value[0] == 0) {
        extern void notice_set_info_from_ancs(void *name, void *data, u16 len);
        g_printf("add message:ancs_notification_uid %04x", ancs_notification_uid);
        notice_set_info_from_ancs("UID", &ancs_notification_uid, sizeof(ancs_notification_uid));
    }
#endif
}

#define SMART_TCFG_BLE_SECURITY_EN          0/*是否发请求加密命令*/
extern void le_device_db_init(void);
void ble_profile_init(void)
{
    printf("ble profile init\n");
    le_device_db_init();
    ble_sm_setup_init(IO_CAPABILITY_NO_INPUT_NO_OUTPUT, SM_AUTHREQ_MITM_PROTECTION | SM_AUTHREQ_BONDING, 7, SMART_TCFG_BLE_SECURITY_EN);
    /* setup ATT server */
    att_server_init(profile_data, att_read_callback, att_write_callback);
    att_server_register_packet_handler(cbk_packet_handler);
    /* gatt_client_register_packet_handler(packet_cbk); */

    // register for HCI events
    hci_event_callback_set(&cbk_packet_handler);
    /* ble_l2cap_register_packet_handler(packet_cbk); */
    /* sm_event_packet_handler_register(packet_cbk); */
    le_l2cap_register_packet_handler(&cbk_packet_handler);

#if TRANS_ANCS_EN
    g_printf("ANCS init...");
    //setup GATT client
    gatt_client_init();

    //setup ANCS clent
    ancs_client_init();
    ancs_set_notification_buffer(ancs_info_buffer, sizeof(ancs_info_buffer));
    ancs_client_register_callback(&cbk_packet_handler);
    extern void ancs_set_out_callback(void *cb);
    ancs_set_out_callback(ancs_remove_message);
#endif
}


static int set_adv_enable(void *priv, u32 en)
{
    ble_state_e next_state, cur_state;

    if (get_app_connect_type() == TYPE_SPP) {
        return APP_BLE_OPERATION_ERROR;
    }

    if (!adv_ctrl_en) {
        return APP_BLE_OPERATION_ERROR;
    }

    printf("con_handle %d\n", con_handle);
    if (con_handle) {
        return APP_BLE_OPERATION_ERROR;
    }

    // 防止ios只连上ble的情况下，android(spp)回连导致ble断开后重新开广播的情况
    if (get_rcsp_connect_status()) {
        printf("spp is connecting\n");
        en = 0;
    }

    if (en) {
        next_state = BLE_ST_ADV;
    } else {
        next_state = BLE_ST_IDLE;
    }

    cur_state =  get_ble_work_state();
    printf("get_ble_work_state  %d\n", cur_state);
    switch (cur_state) {
    case BLE_ST_ADV:
    case BLE_ST_IDLE:
    case BLE_ST_INIT_OK:
    case BLE_ST_NULL:
    case BLE_ST_DISCONN:
        break;
    default:
        return APP_BLE_OPERATION_ERROR;
        break;
    }

    if (cur_state == next_state) {
        printf("cur_state == next_state \n");
        return APP_BLE_NO_ERROR;
    }
    log_info("adv_en:%d\n", en);
    set_ble_work_state(next_state);
    if (en) {
        advertisements_setup_init();
    }
    ble_user_cmd_prepare(BLE_CMD_ADV_ENABLE, 1, en);
    printf(">  set_adv_enable  %d\n", en);
    return APP_BLE_NO_ERROR;
}

int smartbox_set_adv_enable(void *priv, u32 en)
{
    return set_adv_enable(priv, en);
}

static int ble_disconnect(void *priv)
{
    if (con_handle) {
        if (BLE_ST_SEND_DISCONN != get_ble_work_state()) {
            log_info(">>>ble send disconnect\n");
            set_ble_work_state(BLE_ST_SEND_DISCONN);
            ble_user_cmd_prepare(BLE_CMD_DISCONNECT, 1, con_handle);
        } else {
            log_info(">>>ble wait disconnect...\n");
        }
        return APP_BLE_NO_ERROR;
    } else {
        return APP_BLE_OPERATION_ERROR;
    }
}


static int get_buffer_vaild_len(void *priv)
{
    u32 vaild_len = 0;
    ble_user_cmd_prepare(BLE_CMD_ATT_VAILD_LEN, 1, &vaild_len);
    return vaild_len;
}

static int app_send_user_data_do(void *priv, u8 *data, u16 len)
{
    return app_send_user_data(ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE, data, len, ATT_OP_AUTO_READ_CCC);
}

static int app_send_user_data_check(u16 len)
{
    u32 buf_space = get_buffer_vaild_len(0);
    if (len <= buf_space) {
        return 1;
    }
    return 0;
}

static int regiest_wakeup_send(void *priv, void *cbk)
{
    ble_resume_send_wakeup = cbk;
    return APP_BLE_NO_ERROR;
}

static int regiest_recieve_cbk(void *priv, void *cbk)
{
    channel_priv = (u32)priv;
    app_recieve_callback = cbk;
    return APP_BLE_NO_ERROR;
}

static int regiest_state_cbk(void *priv, void *cbk)
{
    channel_priv = (u32)priv;
    app_ble_state_callback = cbk;
    return APP_BLE_NO_ERROR;
}

void bt_ble_adv_enable(u8 enable)
{
    set_adv_enable(0, enable);
}

void ble_module_enable(u8 en)
{
    log_info("mode_en:%d\n", en);
    if (en) {
        adv_ctrl_en = 1;
#if (!MUTIl_CHARGING_BOX_EN)
        bt_ble_adv_enable(1);
#endif
    } else {
        if (con_handle) {
            adv_ctrl_en = 0;
            ble_disconnect(NULL);
        } else {
            bt_ble_adv_enable(0);
            adv_ctrl_en = 0;
        }
    }
}


const struct ble_server_operation_t ble_operation = {
    .adv_enable = set_adv_enable,
    .disconnect = ble_disconnect,
    .get_buffer_vaild = get_buffer_vaild_len,
    .send_data = (void *)app_send_user_data_do,
    .regist_wakeup_send = regiest_wakeup_send,
    .regist_recieve_cbk = regiest_recieve_cbk,
    .regist_state_cbk = regiest_state_cbk,
};

void ble_get_server_operation_table(struct ble_server_operation_t **interface_pt)
{
    *interface_pt = (void *)&ble_operation;
}

void bt_ble_init(void)
{
    log_info("***** ble_init******\n");

    if (ble_init_flag) {
        return;
    }

    gap_device_name = bt_get_local_name();
    gap_device_name_len = strlen(gap_device_name);
    if (gap_device_name_len > BT_NAME_LEN_MAX) {
        gap_device_name_len = BT_NAME_LEN_MAX;
    }

    log_info("ble name(%d): %s \n", gap_device_name_len, gap_device_name);

    set_ble_work_state(BLE_ST_INIT_OK);
    bt_ble_init_do();


    bt_ble_rcsp_adv_enable();
    ble_module_enable(1);

    ble_init_flag = 1;
}

void bt_ble_exit(void)
{
    log_info("***** ble_exit******\n");
    if (!ble_init_flag) {
        return;
    }

    ble_module_enable(0);
#if SMART_BOX_EN
    extern void smartbox_exit(void);
    smartbox_exit();
#endif
    ble_init_flag = 0;
}


void ble_app_disconnect(void)
{
    ble_disconnect(NULL);
}

hci_con_handle_t smartbox_get_con_handle(void)
{
    return con_handle;
}

#if TRANS_ANCS_EN
void hangup_ans_call_handle(u8 en)
{

    u32 notification_id;
    u16 control_point_handle;

    r_printf("hang_up or answer\n");
    notification_id = get_notification_uid();
    control_point_handle = get_controlpoint_handle();
    u8 ble_hangup[] = {0x02, 0, 0, 0, 0, en};
    memcpy(&ble_hangup[1], &notification_id, 4);
    put_buf(ble_hangup, 6);
    u8 ble_hangup_size = 6;
    ble_op_att_send_data(control_point_handle, (void *)&ble_hangup, ble_hangup_size, ATT_OP_WRITE);

}
#endif
#endif

