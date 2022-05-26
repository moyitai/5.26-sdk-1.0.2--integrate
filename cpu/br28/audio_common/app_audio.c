#include "system/includes.h"
#include "media/includes.h"

#include "app_config.h"
#include "app_action.h"
#include "app_main.h"

#include "audio_config.h"

#include "audio_dec.h"
#include "audio_way.h"

#include "application/audio_output_dac.h"
#include "application/audio_dig_vol.h"
struct audio_dac_hdl dac_hdl;

#if (AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_FM)
#include "fm_emitter/fm_emitter_manage.h"
#endif

#include "update.h"

#if (AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_BT)
extern void bt_emitter_set_vol(u8 vol);
#endif

#define LOG_TAG             "[APP_AUDIO]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

#define DEFAULT_DIGTAL_VOLUME   16384

typedef short unaligned_u16 __attribute__((aligned(1)));
struct app_audio_config {
    u8 state;
    u8 prev_state;
    volatile u8 fade_gain_l;
    volatile u8 fade_gain_r;
    volatile s16 fade_dgain_l;
    volatile s16 fade_dgain_r;
    volatile s16 fade_dgain_step_l;
    volatile s16 fade_dgain_step_r;
    volatile int save_vol_timer;
    volatile u8  save_vol_cnt;
    s16 digital_volume;
    atomic_t ref;
    s16 max_volume[APP_AUDIO_MAX_STATE];
    u8 sys_cvol_max;
    u8 call_cvol_max;
    unaligned_u16 *sys_cvol;
    unaligned_u16 *call_cvol;
};
static const char *audio_state[] = {
    "idle",
    "music",
    "call",
    "tone",
    "linein",
    "err",
};

static struct app_audio_config app_audio_cfg = {0};

#define __this      (&app_audio_cfg)

/*关闭audio相关模块使能*/
void audio_disable_all(void)
{
    //DAC:DACEN
    JL_AUDIO->DAC_CON &= ~BIT(4);
    //ADC:ADCEN
    JL_AUDIO->ADC_CON &= ~BIT(4);
    //EQ:
    JL_EQ->CON0 &= ~BIT(0);
    //FFT:
    JL_FFT->CON = BIT(1);//置1强制关闭模块，不管是否已经运算完成
}

static u8 audio_dac_driver_close(void)
{
    audio_disable_all();
    return 0;
}

REGISTER_UPDATE_TARGET(audio_update_target) = {
    .name = "audio",
    .driver_close = audio_dac_driver_close,
};
/*
 *************************************************************
 *
 *	audio volume save
 *
 *************************************************************
 */

static void app_audio_volume_save_do(void *priv)
{
    /* log_info("app_audio_volume_save_do %d\n", __this->save_vol_cnt); */
    local_irq_disable();
    if (++__this->save_vol_cnt >= 5) {
        sys_timer_del(__this->save_vol_timer);
        __this->save_vol_timer = 0;
        __this->save_vol_cnt = 0;
        local_irq_enable();
        log_info("VOL_SAVE\n");
        syscfg_write(CFG_MUSIC_VOL, &app_var.music_volume, 1);//中断里不能操作vm 关中断不能操作vm
        return;
    }
    local_irq_enable();
}

static void app_audio_volume_change(void)
{
    local_irq_disable();
    __this->save_vol_cnt = 0;
    if (__this->save_vol_timer == 0) {
        __this->save_vol_timer = sys_timer_add(NULL, app_audio_volume_save_do, 1000);//中断里不能操作vm 关中断不能操作vm
    }
    local_irq_enable();
}

#if (AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_BT)
__attribute__((weak))
void bt_emitter_set_vol(u8 vol)
{

}
#endif

static int audio_vol_set(u8 gain_l, u8 gain_r, u8 fade)
{
#if (AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_FM)
    return 0;
#endif

#if (AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_IIS)
    extern void *iis_digvol_last;
    if (iis_digvol_last) {
        audio_dig_vol_set(iis_digvol_last, AUDIO_DIG_VOL_ALL_CH, gain_l);
    }
#endif

#if (AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_BT)
    bt_emitter_set_vol(gain_l);
#endif

    local_irq_disable();
    __this->fade_gain_l = gain_l;
    __this->fade_gain_r = gain_r;

#if 0
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_MONO_L)
    audio_dac_vol_set(TYPE_DAC_AGAIN, BIT(0), gain_l, fade);
    audio_dac_vol_set(TYPE_DAC_DGAIN, BIT(0), gain_l ? DEFAULT_DIGTAL_VOLUME : 0, fade);
#elif (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_MONO_R)
    audio_dac_vol_set(TYPE_DAC_AGAIN, BIT(1), gain_r, fade);
    audio_dac_vol_set(TYPE_DAC_DGAIN, BIT(0), gain_r ? DEFAULT_DIGTAL_VOLUME : 0, fade);
#else
    audio_dac_vol_set(TYPE_DAC_AGAIN, BIT(0), gain_l, fade);
    audio_dac_vol_set(TYPE_DAC_AGAIN, BIT(1), gain_r, fade);
    audio_dac_vol_set(TYPE_DAC_DGAIN, BIT(0), gain_l ? DEFAULT_DIGTAL_VOLUME : 0, fade);
    audio_dac_vol_set(TYPE_DAC_DGAIN, BIT(1), gain_r ? DEFAULT_DIGTAL_VOLUME : 0, fade);
#endif
#endif

    local_irq_enable();

    return 0;
}


#if (SYS_VOL_TYPE == VOL_TYPE_AD)

#define DGAIN_SET_MAX_STEP (300)
#define DGAIN_SET_MIN_STEP (30)

static unsigned short combined_vol_list[17][2] = {
    { 0,     0}, //0: None
    { 0,   412}, // 1:-50.00 db
    { 0,   604}, // 2:-46.67 db
    { 0,   887}, // 3:-43.33 db
    { 0,  1301}, // 4:-40.00 db
    { 0,  1910}, // 5:-36.67 db
    { 0,  2804}, // 6:-33.33 db
    { 0,  4115}, // 7:-30.00 db
    { 0,  6041}, // 8:-26.67 db
    { 0,  8867}, // 9:-23.33 db
    { 0, 13014}, // 10:-20.00 db
    { 1,  9574}, // 11:-16.67 db
    { 1, 14052}, // 12:-13.33 db
    { 2, 10338}, // 13:-10.00 db
    { 2, 15174}, // 14:-6.67 db
    { 3, 11162}, // 15:-3.33 db
    { 3, 16384}, // 16:0.00 db
};
static unsigned short call_combined_vol_list[16][2] = {
    { 0,     0}, //0: None
    { 0,   412}, // 1:-50.00 db
    { 0,   604}, // 2:-46.67 db
    { 0,   887}, // 3:-43.33 db
    { 0,  1301}, // 4:-40.00 db
    { 0,  1910}, // 5:-36.67 db
    { 0,  2804}, // 6:-33.33 db
    { 0,  4115}, // 7:-30.00 db
    { 0,  6041}, // 8:-26.67 db
    { 0,  8867}, // 9:-23.33 db
    { 0, 13014}, // 10:-20.00 db
    { 1,  9574}, // 11:-16.67 db
    { 1, 14052}, // 12:-13.33 db
    { 2, 10338}, // 13:-10.00 db
    { 2, 15174}, // 14:-6.67 db
    { 3, 11162}, // 15:-3.33 db
};

void audio_combined_vol_init(u8 cfg_en)
{
    u16 sys_cvol_len = 0;
    u16 call_cvol_len = 0;
    u8 *sys_cvol  = NULL;
    u8 *call_cvol  = NULL;
    s16 *cvol;

    log_info("audio_combined_vol_init\n");
    __this->sys_cvol_max = ARRAY_SIZE(combined_vol_list) - 1;
    __this->sys_cvol = combined_vol_list;
    __this->call_cvol_max = ARRAY_SIZE(call_combined_vol_list) - 1;
    __this->call_cvol = call_combined_vol_list;

    /* if (cfg_en) { */
    if (0) {
        sys_cvol  = syscfg_ptr_read(CFG_COMBINE_SYS_VOL_ID, &sys_cvol_len);
        if (sys_cvol && sys_cvol_len) {
            __this->sys_cvol = (unaligned_u16 *)sys_cvol;
            __this->sys_cvol_max = sys_cvol_len / 4 - 1;
            printf("read sys_cvol ok\n");
        } else {
            printf("read sys_cvol false:%x,%x\n", sys_cvol, sys_cvol_len);
        }

        call_cvol  = syscfg_ptr_read(CFG_COMBINE_CALL_VOL_ID, &call_cvol_len);
        if (call_cvol && call_cvol_len) {
            __this->call_cvol = (unaligned_u16 *)call_cvol;
            __this->call_cvol_max = call_cvol_len / 4 - 1;
            printf("read call_cvol ok\n");
        } else {
            printf("read call_combine_vol false:%x,%x\n", call_cvol, call_cvol_len);
        }
    }

    log_info("sys_cvol_max:%d,call_cvol_max:%d\n", __this->sys_cvol_max, __this->call_cvol_max);
}

static int audio_combined_vol_set(u8 gain_l, u8 gain_r, u8 fade)
{
#if (AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_FM)
    return 0;
#endif
    u8  gain_max;
    u8  target_again_l = 0;
    u8  target_again_r = 0;
    u16 target_dgain_l = 0;
    u16 target_dgain_r = 0;

    if (__this->state == APP_AUDIO_STATE_CALL) {
        gain_max = __this->call_cvol_max;
        gain_l = (gain_l > gain_max) ? gain_max : gain_l;
        gain_r = (gain_r > gain_max) ? gain_max : gain_r;
        target_again_l = *(&__this->call_cvol[gain_l * 2]);
        target_again_r = *(&__this->call_cvol[gain_r * 2]);
        target_dgain_l = *(&__this->call_cvol[gain_l * 2 + 1]);
        target_dgain_r = *(&__this->call_cvol[gain_r * 2 + 1]);
    } else {
        gain_max = __this->sys_cvol_max;
        gain_l = (gain_l > gain_max) ? gain_max : gain_l;
        gain_r = (gain_r > gain_max) ? gain_max : gain_r;
        target_again_l = *(&__this->sys_cvol[gain_l * 2]);
        target_again_r = *(&__this->sys_cvol[gain_r * 2]);
        target_dgain_l = *(&__this->sys_cvol[gain_l * 2 + 1]);
        target_dgain_r = *(&__this->sys_cvol[gain_r * 2 + 1]);
    }
    y_printf("[l]v:%d,Av:%d,Dv:%d", gain_l, target_again_l, target_dgain_l);

    local_irq_disable();

    __this->fade_gain_l  = target_again_l;
    __this->fade_gain_r  = target_again_r;
    __this->fade_dgain_l = target_dgain_l;
    __this->fade_dgain_r = target_dgain_r;

#if 0
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_MONO_L)
    audio_dac_vol_set(TYPE_DAC_AGAIN, BIT(0), __this->fade_gain_l, fade);
    audio_dac_vol_set(TYPE_DAC_DGAIN, BIT(0), __this->fade_dgain_l, fade);
#elif (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_MONO_R)
    audio_dac_vol_set(TYPE_DAC_AGAIN, BIT(1), __this->fade_gain_r, fade);
    audio_dac_vol_set(TYPE_DAC_DGAIN, BIT(0), __this->fade_dgain_r, fade);
#else
    audio_dac_vol_set(TYPE_DAC_AGAIN, BIT(0), __this->fade_gain_l, fade);
    audio_dac_vol_set(TYPE_DAC_AGAIN, BIT(1), __this->fade_gain_r, fade);
    audio_dac_vol_set(TYPE_DAC_DGAIN, BIT(0), __this->fade_dgain_l, fade);
    audio_dac_vol_set(TYPE_DAC_DGAIN, BIT(1), __this->fade_dgain_r, fade);
#endif
#endif

    local_irq_enable();

    return 0;
}

#endif  // (SYS_VOL_TYPE == VOL_TYPE_AD)

void audio_volume_list_init(u8 cfg_en)
{
#if (SYS_VOL_TYPE == VOL_TYPE_AD)
    audio_combined_vol_init(cfg_en);
#elif (SYS_VOL_TYPE == VOL_TYPE_DIGITAL_HW)
    /* audio_hw_digital_vol_init(cfg_en); */
#endif/*SYS_VOL_TYPE*/
}

void volume_up_down_direct(s8 value)
{
    // reserve
}

/* #if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL_HW) */
#define DVOL_HW_LEVEL_MAX	31	/*注意:总共是(DVOL_HW_LEVEL_MAX + 1)级*/
const u16 hw_dig_vol_table[DVOL_HW_LEVEL_MAX + 1] = {
    0	, //0
    93	, //1
    111	, //2
    132	, //3
    158	, //4
    189	, //5
    226	, //6
    270	, //7
    323	, //8
    386	, //9
    462	, //10
    552	, //11
    660	, //12
    789	, //13
    943	, //14
    1127, //15
    1347, //16
    1610, //17
    1925, //18
    2301, //19
    2751, //20
    3288, //21
    3930, //22
    4698, //23
    5616, //24
    6713, //25
    8025, //26
    9592, //27
    11466,//28
    15200,//29
    16000,//30
    16384 //31
};
/*
*********************************************************************
*                  Digital Volume Level To Gain
* Description: 数字音量等级转换为增益值
* Arguments  : level	音量等级
* Return	 : 增益值
* Note(s)    : None.
*********************************************************************
*/
int digital_vol_level_to_gain(int level)
{
    if (level > get_max_sys_vol()) {
        level = get_max_sys_vol();
    }
    u8 dvol_hw_level = 0;
    dvol_hw_level = level * DVOL_HW_LEVEL_MAX / get_max_sys_vol();
    return hw_dig_vol_table[dvol_hw_level];
}

/*
*********************************************************************
*                  Digital Volume Gain To Level
* Description: 数字音量增益值转换为等级
* Arguments  : gain		增益值
* Return	 : 音量等级
* Note(s)    : None.
*********************************************************************
*/
int digital_vol_gain_to_level(int gain)
{
    for (int i = 0; i < DVOL_HW_LEVEL_MAX; i++) {
        if (gain <= hw_dig_vol_table[i]) {
            return i * get_max_sys_vol() / DVOL_HW_LEVEL_MAX;
        }
    }
    return get_max_sys_vol();
}

/* #endif[>SYS_VOL_TYPE == VOL_TYPE_DIGITAL_HW<] */


void audio_fade_in_fade_out(u8 left_gain, u8 right_gain, u8 fade)
{
#if (SYS_VOL_TYPE == VOL_TYPE_AD)
    audio_combined_vol_set(left_gain, right_gain, fade);
#elif (SYS_VOL_TYPE == VOL_TYPE_DIGITAL_HW)
    u8 analog_vol = MAX_ANA_VOL;
    /* u8 dvol_hw_level = 0; */
    /* dvol_hw_level = left_gain * DVOL_HW_LEVEL_MAX / get_max_sys_vol(); */
    /* __this->digital_volume = hw_dig_vol_table[dvol_hw_level]; */
    __this->digital_volume = digital_vol_level_to_gain(left_gain);

    audio_way_set_analog_gain(AUDIO_OUT_WAY_TYPE, SOUND_CHMAP_FL | SOUND_CHMAP_FR, analog_vol);
    audio_way_set_digital_gain(AUDIO_OUT_WAY_TYPE, SOUND_CHMAP_FL | SOUND_CHMAP_FR, __this->digital_volume);
    printf(">>> A:%d D:%d\n", analog_vol, __this->digital_volume);
#else
    audio_vol_set(left_gain, right_gain, fade);
#endif/*SYS_VOL_TYPE == VOL_TYPE_AD*/
}



extern char *music_dig_logo[];

extern void *sys_digvol_group;


void app_audio_set_volume(u8 state, s8 volume, u8 fade)
{

    char *digvol_type = NULL ;

#if (SMART_BOX_EN && RCSP_ADV_EQ_SET_ENABLE)
    extern bool smartbox_set_volume(s8 volume);
    if (smartbox_set_volume(volume)) {
        return;
    }
#endif/*SMART_BOX_EN*/

    if (state == APP_AUDIO_CURRENT_STATE) {
        state = __this->state;
    }

    switch (state) {
    case APP_AUDIO_STATE_IDLE:
    case APP_AUDIO_STATE_MUSIC:
    case APP_AUDIO_STATE_LINEIN:
#if SYS_DIGVOL_GROUP_EN
        digvol_type = "music_type";
#endif/*SYS_DIGVOL_GROUP_EN*/
        app_var.music_volume = volume;
        if (app_var.music_volume > get_max_sys_vol()) {
            app_var.music_volume = get_max_sys_vol();
        }
        volume = app_var.music_volume;
        break;
    case APP_AUDIO_STATE_CALL:
#if SYS_DIGVOL_GROUP_EN
        digvol_type = "call_esco";
#endif/*SYS_DIGVOL_GROUP_EN*/
        app_var.call_volume = volume;
        if (app_var.call_volume > 15) {
            app_var.call_volume = 15;
        }
#if (SYS_VOL_TYPE == VOL_TYPE_ANALOG)
        /*
         *SYS_VOL_TYPE == VOL_TYPE_ANALOG的时候，
         *将通话音量最大值按照手机通话的音量等级进行等分
         *由于等级不匹配，会出现对应有些等级没有一一对应
         *的情况，如果在意，建议使用以下其中一种：
         *#define TCFG_CALL_USE_DIGITAL_VOLUME	1
         *#define SYS_VOL_TYPE 	VOL_TYPE_AD
         *#define SYS_VOL_TYPE 	VOL_TYPE_DIGITAL
         */
        volume = app_var.aec_dac_gain * app_var.call_volume / 15;
#endif/*SYS_VOL_TYPE == VOL_TYPE_ANALOG*/

#if TCFG_CALL_USE_DIGITAL_VOLUME
        audio_dac_vol_set(TYPE_DAC_DGAIN, \
                          BIT(0) | BIT(1), \
                          16384L * (s32)app_var.call_volume / (s32)__this->max_volume[APP_AUDIO_STATE_CALL], \
                          1);
        return;
#endif/*TCFG_CALL_USE_DIGITAL_VOLUME*/
        break;
    case APP_AUDIO_STATE_WTONE:
#if SYS_DIGVOL_GROUP_EN
        digvol_type = "tone_tone";
#endif/*SYS_DIGVOL_GROUP_EN*/

#if TONE_MODE_DEFAULE_VOLUME != 0
        app_var.wtone_volume = TONE_MODE_DEFAULE_VOLUME;
        volume = app_var.wtone_volume;
        break;
#endif

#if APP_AUDIO_STATE_WTONE_BY_MUSIC == 1


        app_var.wtone_volume = app_var.music_volume;
        if (app_var.wtone_volume < 5) {
            app_var.wtone_volume = 5;
        }
#else
        app_var.wtone_volume = volume;
#endif
        if (app_var.wtone_volume > get_max_sys_vol()) {
            app_var.wtone_volume = get_max_sys_vol();
        }
        volume = app_var.wtone_volume;
        break;
    default:
        return;
    }
    if (state == __this->state) {
#if (AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_FM)
        fm_emitter_manage_set_vol(volume);
#else
#if defined (VOL_TYPE_DIGGROUP) && defined (SYS_DIGVOL_GROUP_EN)
        if (state == APP_AUDIO_STATE_LINEIN) {
            //printf("linein analog vol:%d\n",volume);
            audio_fade_in_fade_out(volume, volume, fade);
            return;
        }
        if (SYS_VOL_TYPE == VOL_TYPE_DIGGROUP && SYS_DIGVOL_GROUP_EN) {
            if (sys_digvol_group == NULL) {
                /* printf("the sys_digvol_group is NULL\n-------------------------------------------------------------"); */
                return;
            }
            if (strcmp(digvol_type, "music_type") == 0) {
                for (int i = 0; music_dig_logo[i] != "NULL"; i++) {
                    /* printf("%s\n", music_dig_logo[i]); */
                    if (audio_dig_vol_group_hdl_get(sys_digvol_group, music_dig_logo[i]) == NULL) {
                        continue;
                    }
                    audio_dig_vol_group_vol_set(sys_digvol_group, music_dig_logo[i], AUDIO_DIG_VOL_ALL_CH, volume);
                }
            } else {
                if (audio_dig_vol_group_hdl_get(sys_digvol_group, digvol_type) == NULL) {
                    return;
                }
                audio_dig_vol_group_vol_set(sys_digvol_group, digvol_type, AUDIO_DIG_VOL_ALL_CH, volume);
            }
        }

        else {
            audio_fade_in_fade_out(volume, volume, fade);
        }

#else

        audio_fade_in_fade_out(volume, volume, fade);
#endif //vol_type_diggroup


#endif /*#if (AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_FM)*/

    }
    app_audio_volume_change();
}

void app_audio_volume_init(void)
{
    app_audio_set_volume(APP_AUDIO_STATE_MUSIC, app_var.music_volume, 1);
}

s8 app_audio_get_volume(u8 state)
{
    s8 volume = 0;
    switch (state) {
    case APP_AUDIO_STATE_IDLE:
    case APP_AUDIO_STATE_MUSIC:
    case APP_AUDIO_STATE_LINEIN:
        volume = app_var.music_volume;
        break;
    case APP_AUDIO_STATE_CALL:
        volume = app_var.call_volume;
        break;
    case APP_AUDIO_STATE_WTONE:
#if TONE_MODE_DEFAULE_VOLUME != 0
        app_var.wtone_volume = TONE_MODE_DEFAULE_VOLUME;
        volume = app_var.wtone_volume;
        break;
#endif
#if APP_AUDIO_STATE_WTONE_BY_MUSIC == 1
        volume = app_var.music_volume;
        break;
#else
        volume = app_var.wtone_volume;
        /* if (!volume) { */
        /* volume = app_var.music_volume; */
        /* } */
        break;
#endif
    case APP_AUDIO_CURRENT_STATE:
        volume = app_audio_get_volume(__this->state);
        break;
    default:
        break;
    }
    /* printf("app_audio_get_volume %d %d\n", state, volume); */
    return volume;
}


static const char *audio_mute_string[] = {
    "mute_default",
    "unmute_default",
    "mute_L",
    "unmute_L",
    "mute_R",
    "unmute_R",
};

void app_audio_mute(u8 value)
{
    u8 volume = 0;
    printf("audio_mute:%s", audio_mute_string[value]);
    switch (value) {
    case AUDIO_MUTE_DEFAULT:

#if (AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_FM)
        fm_emitter_manage_set_vol(0);
#else
        audio_dac_vol_mute(1, 1);
#endif
        break;
    case AUDIO_UNMUTE_DEFAULT:
#if (AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_FM)
        volume = app_audio_get_volume(APP_AUDIO_CURRENT_STATE);
        fm_emitter_manage_set_vol(volume);
#else
        audio_dac_vol_mute(0, 1);
#endif
        break;
    }
}


void app_audio_volume_up(u8 value)
{
#if (SMART_BOX_EN && RCSP_ADV_EQ_SET_ENABLE)
    extern bool smartbox_key_volume_up(u8 value);
    if (smartbox_key_volume_up(value)) {
        return;
    }
#endif
    s16 volume = 0;
    switch (__this->state) {
    case APP_AUDIO_STATE_IDLE:
    case APP_AUDIO_STATE_MUSIC:
    case APP_AUDIO_STATE_LINEIN:
        app_var.music_volume += value;
        if (app_var.music_volume > get_max_sys_vol()) {
            app_var.music_volume = get_max_sys_vol();
        }
        volume = app_var.music_volume;
        break;
    case APP_AUDIO_STATE_CALL:
        app_var.call_volume += value;
        if (app_var.call_volume > 15) {
            app_var.call_volume = 15;
        }
        volume = app_var.call_volume;
        break;
    case APP_AUDIO_STATE_WTONE:
#if TONE_MODE_DEFAULE_VOLUME != 0
        app_var.wtone_volume = TONE_MODE_DEFAULE_VOLUME;
        volume = app_var.wtone_volume;
        break;
#endif
#if APP_AUDIO_STATE_WTONE_BY_MUSIC == 1
        app_var.wtone_volume = app_var.music_volume;
#endif
        app_var.wtone_volume += value;
        if (app_var.wtone_volume > get_max_sys_vol()) {
            app_var.wtone_volume = get_max_sys_vol();
        }
        volume = app_var.wtone_volume;
#if APP_AUDIO_STATE_WTONE_BY_MUSIC == 1
        app_var.music_volume = app_var.wtone_volume;
#endif
        break;
    default:
        return;
    }

    app_audio_set_volume(__this->state, volume, 1);
}

void app_audio_volume_down(u8 value)
{
#if (SMART_BOX_EN && RCSP_ADV_EQ_SET_ENABLE)
    extern bool samrtbox_key_volume_down(u8 value);
    if (smartbox_key_volume_up(value)) {
        return;
    }
#endif
    s16 volume = 0;
    switch (__this->state) {
    case APP_AUDIO_STATE_IDLE:
    case APP_AUDIO_STATE_MUSIC:
    case APP_AUDIO_STATE_LINEIN:
        app_var.music_volume -= value;
        if (app_var.music_volume < 0) {
            app_var.music_volume = 0;
        }
        volume = app_var.music_volume;
        break;
    case APP_AUDIO_STATE_CALL:
        app_var.call_volume -= value;
        if (app_var.call_volume < 0) {
            app_var.call_volume = 0;
        }
        volume = app_var.call_volume;
        break;
    case APP_AUDIO_STATE_WTONE:
#if TONE_MODE_DEFAULE_VOLUME != 0
        app_var.wtone_volume = TONE_MODE_DEFAULE_VOLUME;
        volume = app_var.wtone_volume;
        break;
#endif
#if APP_AUDIO_STATE_WTONE_BY_MUSIC == 1
        app_var.wtone_volume = app_var.music_volume;
#endif
        app_var.wtone_volume -= value;
        if (app_var.wtone_volume < 0) {
            app_var.wtone_volume = 0;
        }
        volume = app_var.wtone_volume;
#if APP_AUDIO_STATE_WTONE_BY_MUSIC == 1
        app_var.music_volume = app_var.wtone_volume;
#endif
        break;
    default:
        return;
    }

    app_audio_set_volume(__this->state, volume, 1);
}

void app_audio_volume_set(u8 value)
{
    app_audio_set_volume(__this->state, value, 1);
}

void app_audio_state_switch(u8 state, s16 max_volume)
{
    r_printf("audio state old:%s,new:%s,vol:%d\n", audio_state[__this->state], audio_state[state], max_volume);

    __this->prev_state = __this->state;
    __this->state = state;
#if TCFG_CALL_USE_DIGITAL_VOLUME
    if (__this->state == APP_AUDIO_STATE_CALL) {
        /*调数字音量的时候，模拟音量定最大*/
        audio_dac_vol_set(TYPE_DAC_AGAIN, BIT(0) | BIT(1), max_volume, 1);
        audio_dac_vol_set(TYPE_DAC_DGAIN, BIT(0) | BIT(1), 0, 1);
    }
#endif

    /*限制最大音量*/
    __this->digital_volume = DEFAULT_DIGTAL_VOLUME;
    __this->max_volume[state] = max_volume;

    if (__this->state == APP_AUDIO_STATE_CALL) {
        __this->max_volume[state] = 15;
    }



#if (SYS_VOL_TYPE ==VOL_TYPE_DIGGROUP)
    u8 dac_connect_mode =  app_audio_output_mode_get();
    switch (dac_connect_mode) {
    case DAC_OUTPUT_MONO_L :
        audio_dac_vol_set(TYPE_DAC_AGAIN, BIT(0), 30, 1);
        audio_dac_vol_set(TYPE_DAC_AGAIN, BIT(1), 0, 1);
        audio_dac_vol_set(TYPE_DAC_DGAIN, BIT(0) | BIT(1), 16384, 1);
        break;
    case DAC_OUTPUT_MONO_R :
        audio_dac_vol_set(TYPE_DAC_AGAIN, BIT(1), 30, 1);
        audio_dac_vol_set(TYPE_DAC_AGAIN, BIT(0), 0, 1);
        audio_dac_vol_set(TYPE_DAC_DGAIN, BIT(0) | BIT(1), 16384, 1);
        break;

    default :
        audio_dac_vol_set(TYPE_DAC_AGAIN, BIT(0) | BIT(1), 30, 1);
        audio_dac_vol_set(TYPE_DAC_DGAIN, BIT(0) | BIT(1), 16384, 1);

    }
#endif


    app_audio_set_volume(__this->state, app_audio_get_volume(__this->state), 1);
}

void app_audio_state_exit(u8 state)
{
#if TCFG_CALL_USE_DIGITAL_VOLUME
    if (__this->state == APP_AUDIO_STATE_CALL) {
    }
#endif

    r_printf("audio state now:%s,prev:%s\n", audio_state[__this->state], audio_state[__this->prev_state]);
    if (state == __this->state) {
        __this->state = __this->prev_state;
        __this->prev_state = APP_AUDIO_STATE_IDLE;
    } else if (state == __this->prev_state) {
        __this->prev_state = APP_AUDIO_STATE_IDLE;
    }
    app_audio_set_volume(__this->state, app_audio_get_volume(__this->state), 1);
}
u8 app_audio_get_state(void)
{
    return __this->state;
}

s16 app_audio_get_max_volume(void)
{
    if (__this->state == APP_AUDIO_STATE_IDLE) {
        return get_max_sys_vol();
    }
    return __this->max_volume[__this->state];
}

void dac_power_on(void)
{
    log_info(">>>dac_power_on:%d", __this->ref.counter);
    if (atomic_inc_return(&__this->ref) == 1) {
        audio_dac_open(&dac_hdl);
    }

}
void dac_sniff_power_off(void)
{
    audio_dac_close(&dac_hdl);
}

void dac_power_off(void)
{
    /* log_info(">>>dac_power_off:%d", __this->ref.counter); */
    /* if (atomic_dec_return(&__this->ref)) { */
    /*     return; */
    /* } */
#if 0
    app_audio_mute(AUDIO_MUTE_DEFAULT);
    if (dac_hdl.vol_l || dac_hdl.vol_r) {
        u8 fade_time = dac_hdl.vol_l * 2 / 10 + 1;
        os_time_dly(fade_time);
        printf("fade_time:%d ms", fade_time);
    }
#endif
#if 0//(AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_DAC)
    audio_dac_close(&dac_hdl);
#endif
}
/*
 *自定义dac上电延时时间，具体延时多久应通过示波器测量
 */
#if 1
void dac_power_on_delay()
{
#if TCFG_MC_BIAS_AUTO_ADJUST
    void mic_capless_auto_adjust_init();
    mic_capless_auto_adjust_init();
#endif/*TCFG_MC_BIAS_AUTO_ADJUST*/
    os_time_dly(50);
}
#endif

#define TRIM_VALUE_LR_ERR_MAX           (600)   // 距离参考值的差值限制
#define abs(x) ((x)>0?(x):-(x))
int audio_dac_trim_value_check(struct audio_dac_trim *dac_trim)
{
    printf("audio_dac_trim_value_check %d %d\n", dac_trim->left, dac_trim->right);
    s16 reference = 0;
    if (TCFG_AUDIO_DAC_CONNECT_MODE != DAC_OUTPUT_MONO_R) {
        if (abs(dac_trim->left - reference) > TRIM_VALUE_LR_ERR_MAX) {
            return -1;
        }
    }
    if (TCFG_AUDIO_DAC_CONNECT_MODE != DAC_OUTPUT_MONO_L) {
        if (abs(dac_trim->right - reference) > TRIM_VALUE_LR_ERR_MAX) {
            return -1;
        }
    }

    return 0;
}

#if 0//TCFG_MIC_CAPLESS_ENABLE

//#define LADC_CAPLESS_INFO_DEBUG
#ifdef LADC_CAPLESS_INFO_DEBUG
/*
 * adcdso:正负1000之内
 * dacr32(正常范围:稳定在正负28000之内)
 */
void ladc_capless_info(s16 adcdso, s32 dacr32, s32 pout, s32 tmps8)
{
    printf("[%d, %d, %d, %d]\n", adcdso, dacr32, pout, tmps8);
}
#endif

static void mic_capless_feedback_toggle(u8 toggle);

#define LADC_CAPLESS_ADJUST_SAVE
#ifdef LADC_CAPLESS_ADJUST_SAVE
#define DIFF_RANGE		50
#define CFG_DIFF_RANGE	200
#define CHECK_INTERVAL  7
#define DACR32_DEFAULT	32767

#define MIC_CAPLESS_ADJUST_BUD_DEFAULT	0
#define MIC_CAPLESS_ADJUST_BUD			100
/*不支持自动校准，使用快速收敛*/
#if TCFG_MC_BIAS_AUTO_ADJUST
u8	mic_capless_adjust_bud = MIC_CAPLESS_ADJUST_BUD_DEFAULT;
#else
u8	mic_capless_adjust_bud = MIC_CAPLESS_ADJUST_BUD;
#endif

s16 read_capless_DTB(void)
{
    s16 dacr32 = 32767;
    int ret = syscfg_read(CFG_DAC_DTB, &dacr32, 2);
    printf("cfg DAC_DTB:%d,ret = %d\n", dacr32, ret);
    /*没有记忆值,使用默认值*/
    if (ret != 2) {
        /*没有收敛值的时候，使用快速收敛*/
        //printf("DAC_DTB NULL,use fast feedback");
        mic_capless_adjust_bud = MIC_CAPLESS_ADJUST_BUD;
        /*
         *未初始化状态，返回默认收敛值。可以通过修改默认收敛值，使其
         *接近最终收敛值，来加快预收敛时间。比如最终收敛值是-2500，则
         *可以把默认收敛值设置成-2000左右，因为不同的样机稍微有点小差
         *异，所以这个值不用那么精确，只要差不多就行了。
         */
        return 32767;
    }
    return dacr32;
}

s16 read_vm_capless_DTB(void)
{
    s16 vm_dacr32 = 32767;
    int ret = syscfg_read(CFG_DAC_DTB, &vm_dacr32, 2);
    printf("vm DAC_DTB:%d,ret = %d\n", vm_dacr32, ret);
    if (ret != 2) {
        return DACR32_DEFAULT;
    }
    return vm_dacr32;
}

s16 save_dacr32 = DACR32_DEFAULT;
static u8 adjust_complete = 0;
static u16 dtb_step_limit = 0; /*dtb收敛步进限制*/
void save_capless_DTB()
{
    s16 diff;
    //printf("save_capless_DTB\n");
    if ((save_dacr32 != DACR32_DEFAULT) && adjust_complete) {
        /*比较是否需要更新配置*/
        s16 cfg_dacr32 = read_vm_capless_DTB();
        adjust_complete = 0;
        diff = save_dacr32 - cfg_dacr32;
        if ((cfg_dacr32 == DACR32_DEFAULT) || ((diff < -CFG_DIFF_RANGE) || (diff > CFG_DIFF_RANGE))) {
            log_info("dacr32 write:%d\n", save_dacr32);
            syscfg_write(CFG_DAC_DTB, &save_dacr32, 2);

            /* s16 tmp_dacr32;
            syscfg_read(CFG_DAC_DTB,&tmp_dacr32,2);
            printf("dacr32 read:%d\n",tmp_dacr32); */
        } else {
            log_info("dacr32 need't update:%d,diff:%d\n", save_dacr32, diff);
        }
    } else {
        log_info("dacr32 adjust uncomplete:%d,complete:%d\n", save_dacr32, adjust_complete);
    }
}

void ladc_capless_adjust_post(s32 dacr32, u8 begin)
{
    static s32 last_dacr32 = 0;
    static u8 check_cnt = 0;

    s32 dacr32_diff;

    /*adjust_begin,clear*/
    if (begin) {
        printf("dtb_step_limit = %d\n", dtb_step_limit);
        last_dacr32 = 0;
        adjust_complete = 0;
        check_cnt = 0;
        save_dacr32 = DACR32_DEFAULT;
        return;
    }

#if TCFG_MC_CONVERGE_TRACE
    printf("<%d>", dacr32);
#endif/*MIC_CAPLESS_CONVERGE_TRACE*/
    if (adjust_complete == 0) {
        if (++check_cnt > CHECK_INTERVAL) {
            check_cnt = 0;
            dacr32_diff = dacr32 - last_dacr32;
            //printf("[capless:%d-%d-%d]",dacr32,last_dacr32,dacr32_diff);
            last_dacr32 = dacr32;
            if (adjust_complete == 0) {
                save_dacr32 = dacr32;
            }
            /*调整稳定*/
            if ((dacr32_diff > -DIFF_RANGE) && (dacr32_diff < DIFF_RANGE)) {
                log_info("adjust_OK:%d\n", dacr32);
                adjust_complete = 1;
#if TCFG_MC_BIAS_AUTO_ADJUST
                mic_capless_feedback_toggle(0);
#endif
            }
        }
    }
}
#endif

/*
 *dac快速校准
 */
//#define DAC_TRIM_FAST_EN
#ifdef DAC_TRIM_FAST_EN
u8 dac_trim_fast_en()
{
    return 1;
}
#endif


/*
 *capless模式一开始不要的数据包数量
 */
u16 get_ladc_capless_dump_num(void)
{
    return 10;
}

/*
 *mic省电容模式自动收敛
 */
u8 mic_capless_feedback_sw = 0;
static u8 audio_mc_idle_query(void)
{
    return (mic_capless_feedback_sw ? 0 : 1);
}
REGISTER_LP_TARGET(audio_mc_device_lp_target) = {
    .name = "audio_mc_device",
    .is_idle = audio_mc_idle_query,
};

/*快调慢调边界*/
u16 get_ladc_capless_bud(void)
{
    //printf("mc_bud:%d",mic_capless_adjust_bud);
    return mic_capless_adjust_bud;
}

extern int audio_adc_mic_init(u16 sr);
extern void audio_adc_mic_exit(void);
int audio_mic_capless_feedback_control(u8 en, u16 sr)
{
    int ret = 0;
    if (en) {
        ret = audio_adc_mic_init(sr);
    } else {
        audio_adc_mic_exit();
    }
    return ret;
}

OS_SEM mc_sem;
/*收敛的前提是偏置电压合法*/
static void mic_capless_feedback_toggle(u8 toggle)
{
    int ret = 0;
    log_info("mic_capless_feedback_toggle:%d-%d\n", mic_capless_feedback_sw, toggle);
    if (toggle && (mic_capless_feedback_sw == 0)) {
        mic_capless_feedback_sw = 1;
        ret = audio_mic_capless_feedback_control(1, 32000);
        if (ret == 0) {
            mic_capless_adjust_bud = MIC_CAPLESS_ADJUST_BUD;
        }
        os_sem_create(&mc_sem, 0);
    } else if (mic_capless_feedback_sw) {
        os_sem_post(&mc_sem);
        mic_capless_adjust_bud = MIC_CAPLESS_ADJUST_BUD_DEFAULT;
    } else {
        log_info("Nothing to do\n");
    }
}

extern struct adc_platform_data adc_data;

#if TCFG_MC_BIAS_AUTO_ADJUST
static const u8 mic_bias_tab[] = {0, 20, 12, 28, 4, 18, 10, 26, 2, 22, 14, 30, 17, 21, 6, 25, 29, 27, 31, 5, 3, 7};
extern void delay_2ms(int cnt);
extern void wdt_clear(void);
extern void mic_analog_init(u8 mic_ldo_vsel, u8 mic_bias);
extern void mic_analog_close(struct adc_platform_data *pd);
void mic_capless_auto_adjust_init()
{
    if (adc_data.mic_capless == 0) {
        return;
    }
    log_info("mic_capless_bias_adjust_init:%d-%d\n", adc_data.mic_ldo_vsel, adc_data.mic_bias_res);
    mic_analog_init(adc_data.mic_ldo_vsel, adc_data.mic_bias_res);
}

void mic_capless_auto_adjust_exit()
{
    if (adc_data.mic_capless == 0) {
        return;
    }
    log_info("mic_capless_bias_adjust_exit\n");
    mic_analog_close(&adc_data);
}

/*AC696x系列只支持高压模式*/
#define MIC_BIAS_HIGH_UPPER_LIMIT	200	/*高压上限：2.00v*/
#define MIC_BIAS_HIGH_LOWER_LIMIT	135	/*高压下限：1.35v*/

#define ADC_MIC_IO			IO_PORTA_01
#define ADC_MIC_CH			AD_CH_PA1
#define MIC_BIAS_RSEL(x) 	SFR(JL_ANA->ADA_CON0, 6, 5, x)
#define MIC_LDO_SEL(x)		SFR(JL_ANA->ADA_CON0, 2, 2, x)


/*
 *return -1:非省电容模式
 *return -2:校准失败
 *return  0:默认值合法，不用校准
 *return  1:默认值非法，启动校准
 */
s8 mic_capless_auto_adjust(void)
{
    u16 mic_bias_val = 0;
    u8 mic_bias_idx = adc_data.mic_bias_res;
    u8 mic_bias_compare = 0;
    u16 bias_upper_limit = MIC_BIAS_HIGH_UPPER_LIMIT;
    u16 bias_lower_limit = MIC_BIAS_HIGH_LOWER_LIMIT;
    s8 ret = 0;
    u8 err_cnt = 0;
    u8 mic_ldo_idx = 0;

    //printf("mic_capless_bias_adjust:%d\n",adc_data.mic_capless);

    if (adc_data.mic_capless == 0) {
        return -1;
    }

    log_info("mic_bias idx:%d,rsel:%d\n", mic_bias_idx, mic_bias_tab[mic_bias_idx]);

    /*采样MIC_port(PA1)的偏置电压值*/
    JL_PORTA->DIE &= ~BIT(1);
    JL_PORTA->DIR |=  BIT(1);
    JL_PORTA->PU  &= ~BIT(1);
    JL_PORTA->PD  &= ~BIT(1);
    adc_add_sample_ch(ADC_MIC_CH);

#if 0
    /*
     *调试使用
     *如果mic的偏置电压mic_bias_val稳定，则表示延时足够，否则加大延时知道电压值稳定
     */
    while (1) {
        wdt_clear();
        MIC_BIAS_RSEL(mic_bias_tab[mic_bias_idx]);
        delay_2ms(50);//延时等待偏置电压稳定
        mic_bias_val = adc_get_voltage(ADC_MIC_CH) / 10;
        log_info("mic_bias_val:%d,idx:%d,rsel:%d\n", mic_bias_val, mic_bias_idx, mic_bias_tab[mic_bias_idx]);
    }
#endif

    while (1) {
        wdt_clear();
        MIC_BIAS_RSEL(mic_bias_tab[mic_bias_idx]);
        delay_2ms(50);

        mic_bias_val = adc_get_voltage(ADC_MIC_CH) / 10;
        log_info("mic_bias_val:%d,idx:%d,rsel:%d\n", mic_bias_val, mic_bias_idx, mic_bias_tab[mic_bias_idx]);

        if (mic_bias_val < bias_lower_limit) {
            /*电压偏小，调小内部上拉偏置*/
            mic_bias_compare |= BIT(0);
            mic_bias_idx++;
            if (mic_bias_idx >= sizeof(mic_bias_tab)) {
                log_error("mic_bias_auto_adjust faild 0\n");
                /*校准失败，使用快速收敛*/
                //mic_capless_adjust_bud = MIC_CAPLESS_ADJUST_BUD;
                ret = -2;
                //break;
            }
        } else if (mic_bias_val > bias_upper_limit) {
            /*电压偏大，调大内部上拉偏置*/
            mic_bias_compare |= BIT(1);
            if (mic_bias_idx) {
                mic_bias_idx--;
            } else {
                log_error("mic_bias_auto_adjust faild 1\n");
                /*校准失败，使用快速收敛*/
                //mic_capless_adjust_bud = MIC_CAPLESS_ADJUST_BUD;
                ret = -2;
                //break;
            }
        } else {
            if (mic_bias_compare) {
                /*超出范围，调整过的值,保存*/
                adc_data.mic_bias_res = mic_bias_idx;
                log_info("mic_bias_adjust ok,idx:%d,rsel:%d\n", mic_bias_idx, mic_bias_tab[mic_bias_idx]);
                /*记住校准过的值*/
                ret = syscfg_write(CFG_MC_BIAS, &adc_data.mic_bias_res, 1);
                log_info("mic_bias_adjust save ret = %d\n", ret);
                ret = 1;
            }

            /*原本的MICLDO档位不合适，保存新的MICLDO档位*/
            if (err_cnt) {
                adc_data.mic_ldo_vsel = mic_ldo_idx;
                log_info("mic_ldo_vsel fix:%d\n", adc_data.mic_ldo_vsel);
                //log_info("mic_bias:%d,idx:%d\n",adc_data.mic_bias_res,mic_bias_idx);
                ret = syscfg_write(CFG_MIC_LDO_VSEL, &mic_ldo_idx, 1);
                log_info("mic_ldo_vsel save ret = %d\n", ret);
                ret = 1;
            }
            log_info("mic_bias valid:%d,idx:%d,res:%d\n", mic_bias_val, mic_bias_idx, mic_bias_tab[mic_bias_idx]);
            break;
        }

        /*
         *当前MICLDO分不出合适的偏置电压
         * 选择1、修改MICLDO档位，重新校准
         * 选择2、直接退出，跳出自动校准
         */
        if ((mic_bias_compare == (BIT(0) | BIT(1))) || (ret == -2)) {
            log_info("mic_bias_trim err,adjust micldo vsel\n");
            ret = 0;
#if 1	/*选择1*/
            /*从0开始遍历查询*/
            if (err_cnt) {
                mic_ldo_idx++;
            }
            err_cnt++;
            /*跳过默认的ldo电压档*/
            if (mic_ldo_idx == adc_data.mic_ldo_vsel) {
                mic_ldo_idx++;
            }
            /*遍历结束，没有合适的MICLDO电压档*/
            if (mic_ldo_idx > 3) {
                log_info("mic_bias_adjust tomeout\n");
                mic_capless_adjust_bud = MIC_CAPLESS_ADJUST_BUD;
                ret = -3;
                break;
            }
            log_info("mic_ldo_idx:%d", mic_ldo_idx);
            MIC_LDO_SEL(mic_ldo_idx);
            /*修改MICLDO电压档，等待电压稳定*/
            os_time_dly(20);
            /*复位偏置电阻档位*/
            mic_bias_idx = adc_data.mic_bias_res;
            /*复位校准标志位*/
            mic_bias_compare = 0;
#else	/*选择2*/
            log_info("mic_bias_trim err,break loop\n");
            mic_capless_adjust_bud = MIC_CAPLESS_ADJUST_BUD;
            ret = -3;
            break;
#endif
        }
    }
    mic_capless_auto_adjust_exit();
    return ret;
}
#endif

/*
 *检查mic偏置是否需要校准,以下情况需要重新校准：
 *1、power on reset
 *2、vm被擦除
 *3、每次开机都校准
 */
extern u8 power_reset_src;
u8 mc_bias_adjust_check()
{
#if(TCFG_MC_BIAS_AUTO_ADJUST == MC_BIAS_ADJUST_ALWAYS)
    return 1;
#elif (TCFG_MC_BIAS_AUTO_ADJUST == MC_BIAS_ADJUST_ONE)
    return 0;
#endif
    u8 por_flag = 0;
    int ret = syscfg_read(CFG_POR_FLAG, &por_flag, 1);
    if (ret == 1) {
        if (por_flag == 0xA5) {
            log_info("power on reset 1");
            por_flag = 0;
            ret = syscfg_write(CFG_POR_FLAG, &por_flag, 1);
            return 1;
        }
    }
    if (power_reset_src & BIT(0)) {
        log_info("power on reset 2");
        return 1;
    }
    if (read_vm_capless_DTB() == DACR32_DEFAULT) {
        log_info("vm format");
        return 1;
    }

    return 0;
}

#if TCFG_MC_DTB_STEP_LIMIT
/*获取省电容mic收敛信息配置*/
int get_mc_dtb_step_limit(void)
{
    return dtb_step_limit;
}
#endif /*TCFG_MC_DTB_STEP_LIMIT*/

/*
 *pos = 1:dac trim begin
 *pos = 2:dac trim end
 *pos = 3:dac已经trim过(开机)
 *pos = 4:dac已经读取过变量(过程)
 *pos = 5:dac已经trim过(开机,dac模块初始化)
 */
extern void audio_dac2micbias_en(struct audio_dac_hdl *dac, u8 en);
void _audio_dac_trim_hook(u8 pos)
{
#if TCFG_MC_BIAS_AUTO_ADJUST
    int ret = 0;
    log_info("dac_trim_hook:%d\n", pos);
    if ((adc_data.mic_capless == 0) || (pos == 0xFF)) {
        return;
    }

    if (pos == 1) {
        ret = mic_capless_auto_adjust();
        if (ret >= 0) {
            mic_capless_feedback_toggle(1);
        } else {
            /*校准出错的时候不做预收敛*/
            log_info("auto_adjust err:%d\n", ret);
        }
        return;
    } else if (pos == 2) {
        if (mic_capless_feedback_sw) {
            ret = os_sem_pend(&mc_sem, 250);
            audio_mic_capless_feedback_control(0, 16000);
            if (ret == OS_TIMEOUT) {
                log_info("mc_trim1 timeout!\n");
            } else {
                dtb_step_limit = TCFG_MC_DTB_STEP_LIMIT;
            }
        } else {
            log_info("auto_feedback disable");
        }
    } else if (pos == 5) {
        if (mc_bias_adjust_check()) {
            //printf("MC_BIAS_ADJUST...");
            void mic_capless_auto_adjust_init();
            mic_capless_auto_adjust_init();
            os_time_dly(25);
            ret = mic_capless_auto_adjust();
            /*
             *预收敛条件：
             *1、开机检查发现mic的偏置非法，则校准回来，同时重新收敛,比如中途更换mic头的情况
             *2、收敛值丢失（vm被擦除），重新收敛一次(前提是校准成功)
             */
            if ((ret == 1) || ((ret == 0) && (read_vm_capless_DTB() == DACR32_DEFAULT))) {
                audio_dac2micbias_en(&dac_hdl, 1);
                mic_capless_feedback_toggle(1);
                ret = os_sem_pend(&mc_sem, 250);
                audio_mic_capless_feedback_control(0, 16000);
                audio_dac2micbias_en(&dac_hdl, 0);
                if (ret == OS_TIMEOUT) {
                    log_info("mc_trim2 timeout!\n");
                } else {
                    dtb_step_limit = TCFG_MC_DTB_STEP_LIMIT;
                }
            } else {
                log_info("auto_adjust err:%d\n", ret);
                if (ret == 0) {
                    dtb_step_limit = TCFG_MC_DTB_STEP_LIMIT;
                }
            }
        } else {
            log_info("MC_BIAS_OK...\n");
            dtb_step_limit = TCFG_MC_DTB_STEP_LIMIT;
        }
    }
    mic_capless_feedback_sw = 0;
#endif/*TCFG_MC_BIAS_AUTO_ADJUST*/
}
#endif/*TCFG_MIC_CAPLESS_ENABLE*/


////////////////////////////////// audio_output_api //////////////////////////////////////////////
#if 1

void _audio_dac_irq_hook(void)
{
    /* putbyte('d'); */
    extern struct audio_stream_dac_out *dac_last;
    audio_stream_resume(&dac_last->entry);
}

void _audio_adc_irq_hook(void)
{
    /* putbyte('a'); */
    extern struct audio_adc_hdl adc_hdl;
    audio_adc_irq_handler(&adc_hdl);
}


/*******************************************************
* Function name	: app_audio_output_init
* Description	: 音频输出设备初始化
* Return        : None
********************* -HB ******************************/
void app_audio_output_init(void)
{
    audio_way_init();
}

/*******************************************************
* Function name	: app_audio_output_sync_buff_init
* Description	: 设置音频输出设备同步功能 buf
* Parameter		:
*   @sync_buff		buf 起始地址
*   @len       		buf 长度
* Return        : None
********************* -HB ******************************/
void app_audio_output_sync_buff_init(void *sync_buff, int len)
{
#if 0//(AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_DAC)
    /*音频同步DA端buffer设置*/
    /*audio_output_dac_sync_buff_init(sync_buff, len);*/
#endif
}


/*******************************************************
* Function name	: app_audio_output_samplerate_select
* Description	: 将输入采样率与输出采样率进行匹配对比
* Parameter		:
*   @sample_rate    输入采样率
*   @high:          0 - 低一级采样率，1 - 高一级采样率
* Return        : 匹配后的采样率
********************* -HB ******************************/
int app_audio_output_samplerate_select(u32 sample_rate, u8 high)
{
    int ret = audio_way_check_sample_rate(AUDIO_OUT_WAY_TYPE, sample_rate, high);
    if (ret <= 0) {
        log_e("sr check err:%d \n", ret);
        return sample_rate;
    }
    return ret;
}

/*******************************************************
* Function name	: app_audio_output_samplerate_set
* Description	: 设置音频输出设备的采样率
* Parameter		:
*   @sample_rate	采样率
* Return        : 0 success, other fail
********************* -HB ******************************/
int app_audio_output_samplerate_set(int sample_rate)
{
    int ret = audio_way_set_sample_rate(AUDIO_OUT_WAY_TYPE, sample_rate);
    if (ret <= 0) {
        log_e("sr set err:%d \n", ret);
        return -1;
    }
    return ret;
}

/*******************************************************
* Function name	: app_audio_output_samplerate_get
* Description	: 获取音频输出设备的采样率
* Return        : 音频输出设备的采样率
********************* -HB ******************************/
int app_audio_output_samplerate_get(void)
{
    int ret = audio_way_get_sample_rate(AUDIO_OUT_WAY_TYPE);
    if (ret <= 0) {
        log_e("sr get err:%d \n", ret);
        return 0;
    }
    return ret;
}

/*******************************************************
* Function name	: app_audio_output_mode_get
* Description	: 获取当前硬件输出模式
* Return        : 输出模式
********************* -HB ******************************/
int app_audio_output_mode_get(void)
{
#if 0//(AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_DAC)
    return audio_dac_get_pd_output(&dac_hdl);
#endif
    return 0;
}

/*******************************************************
* Function name	: app_audio_output_mode_set
* Description	: 设置当前硬件输出模式
* Return        : 0 success, other fail
********************* -HB ******************************/
int app_audio_output_mode_set(u8 output)
{
#if 0//(AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_DAC)
    return audio_dac_set_pd_output(&dac_hdl, output);
#endif
    return 0;
}

/*******************************************************
* Function name	: app_audio_output_channel_get
* Description	: 获取音频输出设备输出通道数
* Return        : 通道数
********************* -HB ******************************/
int app_audio_output_channel_get(void)
{
    int ret = audio_way_get_channel_num(AUDIO_OUT_WAY_TYPE);
    if (ret <= 0) {
        log_e("ch get err:%d \n", ret);
        return 0;
    }
    return ret;
}

/*******************************************************
* Function name	: app_audio_output_channel_set
* Description	: 设置音频输出设备输出通道数
* Parameter		:
*   @channel       	通道数
* Return        : 0 success, other fail
********************* -HB ******************************/
int app_audio_output_channel_set(u8 channel)
{
    int ret = audio_way_set_channel_num(AUDIO_OUT_WAY_TYPE, channel);
    if (ret <= 0) {
        log_e("ch set err:%d \n", ret);
        return 0;
    }
    return ret;
}

/*******************************************************
* Function name	: app_audio_output_write
* Description	: 向音频输出设备写入需要输出的音频数据
* Parameter		:
*   @buf			写入音频数据的起始地址
*   @len			写入音频数据的长度
* Return        : 成功写入的长度
********************* -HB ******************************/
int app_audio_output_write(void *buf, int len)
{
    return audio_way_output_write(buf, len);
}


/*******************************************************
* Function name	: app_audio_output_start
* Description	: 音频输出设备输出打开
* Return        : 0 success, other fail
********************* -HB ******************************/
int app_audio_output_start(void)
{
    audio_way_ioctrl(AUDIO_OUT_WAY_TYPE, SNDCTL_IOCTL_POWER_ON, NULL);
    /* audio_way_set_sample_rate(AUDIO_OUT_WAY_TYPE, stream->cur_sr); */
    /* audio_way_set_gain(AUDIO_OUT_WAY_TYPE, 30); */
    audio_way_start(AUDIO_OUT_WAY_TYPE);
    return 0;
}

/*******************************************************
* Function name	: app_audio_output_stop
* Description	: 音频输出设备输出停止
* Return        : 0 success, other fail
********************* -HB ******************************/
int app_audio_output_stop(void)
{
    audio_way_stop(AUDIO_OUT_WAY_TYPE);
    audio_way_ioctrl(AUDIO_OUT_WAY_TYPE, SNDCTL_IOCTL_POWER_OFF, NULL);
    return 0;
}

/*******************************************************
* Function name	: app_audio_output_reset
* Description	: 音频输出设备重启
* Parameter		:
*   @msecs       	重启时间 ms
* Return        : 0 success, other fail
********************* -HB ******************************/
int app_audio_output_reset(u32 msecs)
{
#if 0//(AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_DAC)
    return audio_dac_sound_reset(&dac_hdl, msecs);
#endif
    return 0;
}

/*******************************************************
* Function name	: app_audio_output_get_cur_buf_points
* Description	: 获取当前音频输出buf还可以输出的点数
* Parameter		:
* Return        : 还可以输出的点数
********************* -HB ******************************/
int app_audio_output_get_cur_buf_points(void)
{
    return 0;
}

int app_audio_output_ch_analog_gain_set(u8 ch, u8 again)
{
#if 0//(AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_DAC)
    return audio_dac_ch_analog_gain_set(&dac_hdl, ch, again);
#endif
    return 0;
}

int app_audio_output_ch_digital_gain_set(u8 ch, u32 dgain)
{
#if 0//(AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_DAC)
    return audio_dac_ch_digital_gain_set(&dac_hdl, ch, dgain);
#endif
    return 0;
}

int app_audio_output_state_get(void)
{
#if 0//(AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_DAC)
    return audio_dac_get_status(&dac_hdl);
#endif
    return 0;
}

void app_audio_output_ch_mute(u8 ch, u8 mute)
{
    audio_dac_ch_mute(&dac_hdl, ch, mute);
}

int audio_output_buf_time(void)
{
#if 0//(AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_DAC)
    return audio_dac_data_time(&dac_hdl);
#endif
    return 0;
}

int audio_output_dev_is_working(void)
{
#if 0//(AUDIO_OUT_WAY_TYPE & AUDIO_WAY_TYPE_DAC)
    return audio_dac_is_working(&dac_hdl);
#endif
    return 1;
}

int audio_output_sync_start(void)
{
    return 0;
}

int audio_output_sync_stop(void)
{
    return 0;
}

#endif

