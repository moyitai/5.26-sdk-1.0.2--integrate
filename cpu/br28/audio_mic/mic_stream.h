#ifndef __MIC_STREAM_H__
#define __MIC_STREAM_H__

#include "system/includes.h"
#include "media/includes.h"
enum {
    MIC_EFFECT_CONFIG_REVERB = 0x0, //声卡混响
    MIC_EFFECT_CONFIG_ECHO,         //K歌混响
    MIC_EFFECT_CONFIG_PITCH,        //变声
    MIC_EFFECT_CONFIG_NOISEGATE,    //噪声压制，主要压制无人声时底噪
    MIC_EFFECT_CONFIG_DODGE,        //闪避
    MIC_EFFECT_CONFIG_DVOL,         //数字音量
    MIC_EFFECT_CONFIG_HOWLING,      //移频啸叫抑制
    MIC_EFFECT_CONFIG_FILT,         //高低音处理,新版本 与EQ合并
    MIC_EFFECT_CONFIG_EQ,           //MIC EQ
    MIC_EFFECT_CONFIG_ENERGY_DETECT,//能量检测
    MIC_EFFECT_CONFIG_SOFT_SRC,     //变采样
    MIC_EFFECT_CONFIG_LINEIN,       //linein 叠加，仅限声卡使用
    MIC_EFFECT_CONFIG_DIGIT_ENHANCE,//数字放大，提升mic灵敏度
};


struct __mic_stream_io {
    void *priv;
    u32(*func)(void *priv, void *in, void *out, u32 inlen, u32 outlen);
};

struct __mic_stream_parm {
    u16 sample_rate;
    u16 point_unit;
    u16	dac_delay;
    u16 mic_gain;
};

struct __mic_stream {
    struct adc_mic_ch 				mic_ch;
    struct audio_adc_output_hdl 	adc_output;
    struct __mic_stream_parm		*parm;
    struct __mic_stream_io 			out;
    OS_SEM 							sem;
    volatile u8						busy:		1;
        volatile u8						release:	1;
        volatile u8						revert:		6;
        s16 *adc_buf;
        int adc_buf_len;
    };

    typedef struct __mic_stream mic_stream;


    /*----------------------------------------------------------------------------*/
    /**@brief    创建mic数据流
       @param    采样率、增益、ADC中断输出点数、dac延时
       @return   数据流句柄
       @note
    */
    /*----------------------------------------------------------------------------*/
    mic_stream *mic_stream_creat(struct __mic_stream_parm *parm);


    /*----------------------------------------------------------------------------*/
    /**@brief    释放mic数据流
       @param    数据流句柄
       @return
       @note
    */
    /*----------------------------------------------------------------------------*/
    void mic_stream_destroy(mic_stream **hdl);


    /*----------------------------------------------------------------------------*/
    /**@brief    启动mic
       @param    struct __mic_stream *
       @return   true - 成功, false - 失败.
       @note
    */
    /*----------------------------------------------------------------------------*/
    bool mic_stream_start(mic_stream  *stream);


    /*----------------------------------------------------------------------------*/
    /**@brief    设置mic数据流回调函数
       @param
       @return
       @note
    */
    /*----------------------------------------------------------------------------*/
    void mic_stream_set_output(struct __mic_stream  *stream, void *priv, u32(*func)(void *priv, void *in, void *out, u32 inlen, u32 outlen));

#endif// __MIC_STREAM_H__
