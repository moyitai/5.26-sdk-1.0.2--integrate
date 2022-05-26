#ifndef CONFIG_BOARD_701N_DEMO_CFG_H
#define CONFIG_BOARD_701N_DEMO_CFG_H

#include "board_701n_demo_global_build_cfg.h"

#ifdef CONFIG_BOARD_701N_DEMO

#define CONFIG_SDFILE_ENABLE

//*********************************************************************************//
//                                 配置开始                                        //
//*********************************************************************************//
#define ENABLE_THIS_MOUDLE					1
#define DISABLE_THIS_MOUDLE					0

#define ENABLE								1
#define DISABLE								0

#define NO_CONFIG_PORT						(-1)

//*********************************************************************************//
//                                  app 配置                                       //
//*********************************************************************************//
#define TCFG_APP_BT_EN			            1
#define TCFG_APP_MUSIC_EN			        1
#define TCFG_APP_LINEIN_EN					0
#define TCFG_APP_FM_EN					    0
#define TCFG_APP_PC_EN					    0
#define TCFG_APP_RTC_EN					    1
#define TCFG_APP_RECORD_EN				    0
#define TCFG_APP_SPDIF_EN                   0

//*********************************************************************************//
//                                 UART配置                                        //
//*********************************************************************************//
#define TCFG_UART0_ENABLE					ENABLE_THIS_MOUDLE                     //串口打印模块使能
#define TCFG_UART0_RX_PORT					NO_CONFIG_PORT                         //串口接收脚配置（用于打印可以选择NO_CONFIG_PORT）
#define TCFG_UART0_TX_PORT  				IO_PORTA_04                            //串口发送脚配置
#define TCFG_UART0_BAUDRATE  				1000000                                //串口波特率配置

//*********************************************************************************//
//                                 IIC配置                                        //
//*********************************************************************************//
/*软件IIC设置*/
#define TCFG_SW_I2C0_CLK_PORT               IO_PORTG_03                             //软件IIC  CLK脚选择
#define TCFG_SW_I2C0_DAT_PORT               IO_PORTG_04                             //软件IIC  DAT脚选择
#define TCFG_SW_I2C0_DELAY_CNT              75                                      //IIC延时参数，影响通讯时钟频率

/*硬件IIC端口选择
  SCL         SDA
  'A': IO_PORT_DP   IO_PORT_DM
  'B': IO_PORTA_09  IO_PORTA_10
  'C': IO_PORTA_07  IO_PORTA_08
  'D': IO_PORTA_05  IO_PORTA_06
 */
#define TCFG_HW_I2C0_PORTS                  'B'
#define TCFG_HW_I2C0_CLK                    100000                                  //硬件IIC波特率

//*********************************************************************************//
//                                 硬件SPI 配置                                        //
//*********************************************************************************//
#define	TCFG_HW_SPI1_ENABLE		ENABLE_THIS_MOUDLE
#define TCFG_HW_SPI1_PORT_CLK   IO_PORTA_07
#define TCFG_HW_SPI1_PORT_DO    IO_PORTA_08
#define TCFG_HW_SPI1_PORT_DI    NO_CONFIG_PORT
#define TCFG_HW_SPI1_BAUD		24000000L
#define TCFG_HW_SPI1_MODE		SPI_MODE_BIDIR_1BIT
#define TCFG_HW_SPI1_ROLE		SPI_ROLE_MASTER

#define	TCFG_HW_SPI2_ENABLE		ENABLE_THIS_MOUDLE
#define TCFG_HW_SPI2_PORT_CLK   NO_CONFIG_PORT
#define TCFG_HW_SPI2_PORT_DO    NO_CONFIG_PORT
#define TCFG_HW_SPI2_PORT_DI    NO_CONFIG_PORT
#define TCFG_HW_SPI2_BAUD		2000000L
#define TCFG_HW_SPI2_MODE		SPI_MODE_BIDIR_1BIT
#define TCFG_HW_SPI2_ROLE		SPI_ROLE_MASTER

//*********************************************************************************//
//                                 FLASH 配置                                      //
//*********************************************************************************//
#define TCFG_NOR_FS                  		ENABLE_THIS_MOUDLE
#define TCFG_NOR_FAT                  		DISABLE_THIS_MOUDLE
#define TCFG_NOR_VM                  		ENABLE_THIS_MOUDLE
#define TCFG_NOR_REC                  		DISABLE_THIS_MOUDLE
#define TCFG_NORFLASH_SFC_DEV_ENABLE 		ENABLE_THIS_MOUDLE
#define TCFG_NORFLASH_DEV_ENABLE		    DISABLE_THIS_MOUDLE
#define TCFG_NANDFLASH_DEV_ENABLE		    DISABLE_THIS_MOUDLE
#define TCFG_FLASH_DEV_SPI_HW_NUM			2// 1: SPI1    2: SPI2
#define TCFG_FLASH_DEV_SPI_CS_PORT	    	IO_PORTB_07

#define TCFG_VIRFAT_FLASH_ENABLE  			ENABLE	// FLASH虚拟文件系统
#define TCFG_VIRFAT_INSERT_FLASH_ENABLE  	DISABLE	// 内置FLASH虚拟文件系统
#define TCFG_VIRFAT_INSERT_FLASH_SIZE	  	0x300000 // 内置FLASH虚拟文件系统空间大小
#define TCFG_SFC_VM                         ENABLE	// 内置FLASH自定义存储管理
#define TCFG_SFC_VM_SIZE                    0x8000	// 内置FLASH自定义存储管理空间大小

#define TCFG_EX_FLASH_POWER_IO				IO_PORTC_08 // 外置flash电源脚
#define CONFIG_EX_FLASH_POWER_IO			PC08 // NULL // 外置flash电源脚。不接IO时填NULL或者屏蔽掉该宏定义

#define TCFG_SFC_ENCRY_ENABLE				1 // 是否加密写和解密读

//*********************************************************************************//
//                                  SD 配置                                        //
//*********************************************************************************//
#define TCFG_SD0_ENABLE						ENABLE_THIS_MOUDLE	//SD0模块使能
#define TCFG_SD0_DAT_MODE					1               //线数设置，1：一线模式  4：四线模式
#define TCFG_SD0_DET_MODE					SD_IO_DECT//SD_CLK_DECT		//SD卡检测方式
#define TCFG_SD0_DET_IO                     IO_PORTB_03      //当检测方式为IO检测可用
#define TCFG_SD0_DET_IO_LEVEL               0               //当检测方式为IO检测可用,0：低电平检测到卡。 1：高电平(外部电源)检测到卡。 2：高电平(SD卡电源)检测到卡。
#define TCFG_SD0_CLK						(3000000 * 4L)  //SD卡时钟频率设置
#define TCFG_SD0_PORT_CMD					IO_PORTG_01
#define TCFG_SD0_PORT_CLK					IO_PORTG_02
#define TCFG_SD0_PORT_DA0					IO_PORTG_00
#define TCFG_SD0_PORT_DA1					NO_CONFIG_PORT  //当选择4线模式时要用
#define TCFG_SD0_PORT_DA2					NO_CONFIG_PORT
#define TCFG_SD0_PORT_DA3					NO_CONFIG_PORT

#define TCFG_SD_ALWAY_ONLINE_ENABLE         ENABLE
#define TCFG_KEEP_CARD_AT_ACTIVE_STATUS	    DISABLE	// 保持卡活跃状态。会增加功耗


//*********************************************************************************//
//                                 key 配置                                        //
//*********************************************************************************//
//#define KEY_NUM_MAX                        	10
//#define KEY_NUM                            	3
#define KEY_IO_NUM_MAX						6
#define KEY_AD_NUM_MAX						10
#define KEY_IR_NUM_MAX						21
#define KEY_TOUCH_NUM_MAX					6
#define KEY_RDEC_NUM_MAX                    6
#define KEY_CTMU_TOUCH_NUM_MAX				6

#define MULT_KEY_ENABLE						DISABLE 		//是否使能组合按键消息, 使能后需要配置组合按键映射表

#define TCFG_KEY_TONE_EN					DISABLE 		// 按键提示音。

//*********************************************************************************//
//                                 iokey 配置                                      //
//*********************************************************************************//
#define TCFG_IOKEY_ENABLE					ENABLE_THIS_MOUDLE //是否使能IO按键

#define TCFG_IOKEY0_WAY             		ONE_PORT_TO_LOW    //按键一端接低电平一端接IO
#define TCFG_IOKEY0             			IO_PORTB_01        //IO按键端口

#define TCFG_IOKEY1_WAY		            	ONE_PORT_TO_LOW  //按键一端接低电平一端接IO
#define TCFG_IOKEY1	                		IO_PORTB_00

#define TCFG_IOKEY2_WAY             		ONE_PORT_TO_LOW  //按键一端接低电平一端接IO
#define TCFG_IOKEY2		                	IO_PORTC_07

#define TCFG_IOKEY3_WAY 	            	ONE_PORT_TO_LOW  //按键一端接低电平一端接IO
#define TCFG_IOKEY3			                IO_PORTC_06

//*********************************************************************************//
//                                 adkey 配置                                      //
//*********************************************************************************//
#define TCFG_ADKEY_ENABLE                   DISABLE_THIS_MOUDLE//是否使能AD按键
#define TCFG_ADKEY_PORT                     IO_PORT_DM         //AD按键端口(需要注意选择的IO口是否支持AD功能)
/*AD通道选择，需要和AD按键的端口相对应:
    AD_CH_PA1    AD_CH_PA3    AD_CH_PA4    AD_CH_PA5
    AD_CH_PA9    AD_CH_PA1    AD_CH_PB1    AD_CH_PB4
    AD_CH_PB6    AD_CH_PB7    AD_CH_DP     AD_CH_DM
    AD_CH_PB2
*/
#define TCFG_ADKEY_AD_CHANNEL               AD_CH_DM
#define TCFG_ADKEY_EXTERN_UP_ENABLE         ENABLE_THIS_MOUDLE //是否使用外部上拉

#if TCFG_ADKEY_EXTERN_UP_ENABLE
#define R_UP    220                 //22K，外部上拉阻值在此自行设置
#else
#define R_UP    100                 //10K，内部上拉默认10K
#endif

//必须从小到大填电阻，没有则同VDDIO,填0x3ffL
#define TCFG_ADKEY_AD0      (0)                                 //0R
#define TCFG_ADKEY_AD1      (0x3ffL * 30   / (30   + R_UP))     //3k

#define TCFG_ADKEY_AD2      (0x3ffL * 62   / (62   + R_UP))     //6.2k
#define TCFG_ADKEY_AD3      (0x3ffL * 91   / (91   + R_UP))     //9.1k
#define TCFG_ADKEY_AD4      (0x3ffL * 150  / (150  + R_UP))     //15k
#define TCFG_ADKEY_AD5      (0x3ffL * 240  / (240  + R_UP))     //24k
#define TCFG_ADKEY_AD6      (0x3ffL * 330  / (330  + R_UP))     //33k
#define TCFG_ADKEY_AD7      (0x3ffL * 510  / (510  + R_UP))     //51k
#define TCFG_ADKEY_AD8      (0x3ffL * 1000 / (1000 + R_UP))     //100k
#define TCFG_ADKEY_AD9      (0x3ffL * 2200 / (2200 + R_UP))     //220k
#define TCFG_ADKEY_VDDIO    (0x3ffL)

#define TCFG_ADKEY_VOLTAGE0 ((TCFG_ADKEY_AD0 + TCFG_ADKEY_AD1) / 2)
#define TCFG_ADKEY_VOLTAGE1 ((TCFG_ADKEY_AD1 + TCFG_ADKEY_AD2) / 2)
#define TCFG_ADKEY_VOLTAGE2 ((TCFG_ADKEY_AD2 + TCFG_ADKEY_AD3) / 2)
#define TCFG_ADKEY_VOLTAGE3 ((TCFG_ADKEY_AD3 + TCFG_ADKEY_AD4) / 2)
#define TCFG_ADKEY_VOLTAGE4 ((TCFG_ADKEY_AD4 + TCFG_ADKEY_AD5) / 2)
#define TCFG_ADKEY_VOLTAGE5 ((TCFG_ADKEY_AD5 + TCFG_ADKEY_AD6) / 2)
#define TCFG_ADKEY_VOLTAGE6 ((TCFG_ADKEY_AD6 + TCFG_ADKEY_AD7) / 2)
#define TCFG_ADKEY_VOLTAGE7 ((TCFG_ADKEY_AD7 + TCFG_ADKEY_AD8) / 2)
#define TCFG_ADKEY_VOLTAGE8 ((TCFG_ADKEY_AD8 + TCFG_ADKEY_AD9) / 2)
#define TCFG_ADKEY_VOLTAGE9 ((TCFG_ADKEY_AD9 + TCFG_ADKEY_VDDIO) / 2)

#define TCFG_ADKEY_VALUE0                   0
#define TCFG_ADKEY_VALUE1                   1
#define TCFG_ADKEY_VALUE2                   2
#define TCFG_ADKEY_VALUE3                   3
#define TCFG_ADKEY_VALUE4                   4
#define TCFG_ADKEY_VALUE5                   5
#define TCFG_ADKEY_VALUE6                   6
#define TCFG_ADKEY_VALUE7                   7
#define TCFG_ADKEY_VALUE8                   8
#define TCFG_ADKEY_VALUE9                   9

//*********************************************************************************//
//                                 irkey 配置                                      //
//*********************************************************************************//
#define TCFG_IRKEY_ENABLE                   DISABLE_THIS_MOUDLE//是否使能AD按键
#define TCFG_IRKEY_PORT                     IO_PORTA_08        //IR按键端口

//*********************************************************************************//
//                              tocuh key 配置                                     //
//*********************************************************************************//
#define TCFG_TOUCH_KEY_ENABLE               DISABLE_THIS_MOUDLE             //是否使能plcnt触摸按键
//key0配置
#define TCFG_TOUCH_KEY0_PRESS_DELTA	   	    100//变化阈值，当触摸产生的变化量达到该阈值，则判断被按下，每个按键可能不一样，可先在驱动里加到打印，再反估阈值
#define TCFG_TOUCH_KEY0_PORT 				IO_PORTB_06 //触摸按键key0 IO配置
#define TCFG_TOUCH_KEY0_VALUE 				0x12 		//触摸按键key0 按键值
//key1配置
#define TCFG_TOUCH_KEY1_PRESS_DELTA	   	    100//变化阈值，当触摸产生的变化量达到该阈值，则判断被按下，每个按键可能不一样，可先在驱动里加到打印，再反估阈值
#define TCFG_TOUCH_KEY1_PORT 				IO_PORTB_07 //触摸按键key1 IO配置
#define TCFG_TOUCH_KEY1_VALUE 				0x34        //触摸按键key1 按键值

//*********************************************************************************//
//                                 rdec_key 配置                                      //
//*********************************************************************************//
#define TCFG_RDEC_KEY_ENABLE					DISABLE_THIS_MOUDLE //是否使能RDEC按键
//RDEC0配置
#define TCFG_RDEC0_ECODE1_PORT					IO_PORTA_12
#define TCFG_RDEC0_ECODE2_PORT					IO_PORTA_13
#define TCFG_RDEC0_KEY0_VALUE 				 	0
#define TCFG_RDEC0_KEY1_VALUE 				 	1

//RDEC1配置
#define TCFG_RDEC1_ECODE1_PORT					NO_CONFIG_PORT
#define TCFG_RDEC1_ECODE2_PORT					NO_CONFIG_PORT
#define TCFG_RDEC1_KEY0_VALUE 				 	2
#define TCFG_RDEC1_KEY1_VALUE 				 	3

//RDEC2配置
#define TCFG_RDEC2_ECODE1_PORT					NO_CONFIG_PORT
#define TCFG_RDEC2_ECODE2_PORT					NO_CONFIG_PORT
#define TCFG_RDEC2_KEY0_VALUE 				 	4
#define TCFG_RDEC2_KEY1_VALUE 				 	5

//*********************************************************************************//
//                                 Audio配置                                       //
//*********************************************************************************//
#define TCFG_AUDIO_ADC_ENABLE				ENABLE_THIS_MOUDLE
//MIC只有一个声道，固定选择右声道
#define TCFG_AUDIO_ADC_MIC_CHA				LADC_CH_MIC_R
#define TCFG_AUDIO_ADC_LINE_CHA				LADC_LINE0_MASK
/*MIC LDO电流档位设置：
    0:0.625ua    1:1.25ua    2:1.875ua    3:2.5ua*/
#define TCFG_AUDIO_ADC_LD0_SEL				2

#define TCFG_AUDIO_DAC_ENABLE				ENABLE_THIS_MOUDLE
#define TCFG_AUDIO_DAC_LDO_SEL				1
/*
DACVDD电压设置(要根据具体的硬件接法来确定):
    DACVDD_LDO_1_20V        DACVDD_LDO_1_30V        DACVDD_LDO_2_35V        DACVDD_LDO_2_50V
    DACVDD_LDO_2_65V        DACVDD_LDO_2_80V        DACVDD_LDO_2_95V        DACVDD_LDO_3_10V*/

/*
	DACVDD_LDO_2_40V        DACVDD_LDO_2_50V        DACVDD_LDO_2_60V        DACVDD_LDO_2_70V
    DACVDD_LDO_2_80V        DACVDD_LDO_2_90V        DACVDD_LDO_3_00V        DACVDD_LDO_3_10V*/
#define TCFG_AUDIO_DAC_LDO_VOLT				DACVDD_LDO_2_80V
/*PA*/
#define TCFG_AUDIO_DAC_PA_PORT				IO_PORTG_07
/*
DAC硬件上的连接方式,可选的配置：
    DAC_OUTPUT_MONO_L               左声道
    DAC_OUTPUT_MONO_R               右声道
    DAC_OUTPUT_LR                   立体声
    DAC_OUTPUT_MONO_LR_DIFF         左右差分

    TCFG_AUDIO_DAC_CONNECT_MODE 决定方案需要使用多少个DAC声道
    TCFG_AUDIO_DAC_MODE 决定DAC是差分输出模式还是单端输出模式

    注意:
    1、选择 DAC_OUTPUT_MONO_LR_DIFF DAC要选单端输出，选差分N端也会打开导致功耗大。
*/
#define TCFG_AUDIO_DAC_CONNECT_MODE         DAC_OUTPUT_MONO_L

/*
         DAC模式选择
#define DAC_MODE_L_DIFF          (0)  // 低压差分模式   , 适用于低功率差分耳机  , 输出幅度 0~2Vpp
#define DAC_MODE_H1_DIFF         (1)  // 高压1档差分模式, 适用于高功率差分耳机  , 输出幅度 0~3Vpp  , VDDIO >= 3.0V
#define DAC_MODE_H1_SINGLE       (2)  // 高压1档单端模式, 适用于高功率单端PA音箱, 输出幅度 0~1.5Vpp, VDDIO >= 3.0V
#define DAC_MODE_H2_DIFF         (3)  // 高压2档差分模式, 适用于高功率差分PA音箱, 输出幅度 0~5Vpp  , VDDIO >= 3.3V
#define DAC_MODE_H2_SINGLE       (4)  // 高压2档单端模式, 适用于高功率单端PA音箱, 输出幅度 0~2.5Vpp, VDDIO >= 3.3V
*/
#define TCFG_AUDIO_DAC_MODE     DAC_MODE_H1_DIFF

/*
解码后音频的输出方式:
    AUDIO_OUTPUT_ORIG_CH            按原始声道输出
    AUDIO_OUTPUT_STEREO             按立体声
    AUDIO_OUTPUT_L_CH               只输出原始声道的左声道
    AUDIO_OUTPUT_R_CH               只输出原始声道的右声道
    AUDIO_OUTPUT_MONO_LR_CH         输出左右合成的单声道
 */
#define AUDIO_OUTPUT_MODE                   AUDIO_OUTPUT_MONO_LR_CH

#define TCFG_AUDIO_NOISE_GATE				DISABLE_THIS_MOUDLE
/*
 *系统音量类型选择
 *软件数字音量是指纯软件对声音进行运算后得到的
 *硬件数字音量是指dac内部数字模块对声音进行运算后输出
 */
#define VOL_TYPE_DIGITAL		0	//软件数字音量
#define VOL_TYPE_ANALOG			1	//硬件模拟音量
#define VOL_TYPE_AD				2	//联合音量(模拟数字混合调节)
#define VOL_TYPE_DIGITAL_HW		3  	//硬件数字音量
#define VOL_TYPE_DIGGROUP		4
#define SYS_VOL_TYPE            VOL_TYPE_DIGITAL_HW

#define SYS_DIGVOL_GROUP_EN		DISABLE

/*
 *通话的时候使用数字音量
 *0：通话使用和SYS_VOL_TYPE一样的音量调节类型
 *1：通话使用数字音量调节，更加平滑
 */
#define TCFG_CALL_USE_DIGITAL_VOLUME		0

/*MIC模式配置:单端隔直电容模式/差分隔直电容模式/单端省电容模式*/
#define TCFG_AUDIO_MIC_MODE                 AUDIO_MIC_CAP_MODE
#define TCFG_AUDIO_MIC1_MODE                AUDIO_MIC_CAP_MODE
#define TCFG_AUDIO_MIC2_MODE                AUDIO_MIC_CAP_MODE
#define TCFG_AUDIO_MIC3_MODE                AUDIO_MIC_CAP_MODE

/*通话降噪模式配置*/
#define CVP_ANS_MODE	0	/*传统降噪*/
#define CVP_DNS_MODE	1	/*神经网络降噪*/
#define TCFG_AUDIO_CVP_NS_MODE				CVP_ANS_MODE

/*ENC(双mic降噪)使能*/
#define TCFG_AUDIO_DUAL_MIC_ENABLE			DISABLE_THIS_MOUDLE

/*通话CVP 测试使能，配合量产设备测试MIC频响和算法性能*/
#define TCFG_AUDIO_CVP_DUT_ENABLE			DISABLE_THIS_MOUDLE

//#define AUDIO_PCM_DEBUG 1

/*
 *>>MIC电源管理:根据具体方案，选择对应的mic供电方式
 *(1)如果是多种方式混合，则将对应的供电方式或起来即可，比如(MIC_PWR_FROM_GPIO | MIC_PWR_FROM_MIC_BIAS)
 *(2)如果使用固定电源供电(比如dacvdd)，则配置成DISABLE_THIS_MOUDLE
 */
#define MIC_PWR_FROM_GPIO       (1UL << 0)  //使用普通IO输出供电
#define MIC_PWR_FROM_MIC_BIAS   (1UL << 1)  //使用内部mic_ldo供电(有上拉电阻可配)
#define MIC_PWR_FROM_MIC_LDO    (1UL << 2)  //使用内部mic_ldo供电
//配置MIC电源
#define TCFG_AUDIO_MIC_PWR_CTL              MIC_PWR_FROM_MIC_LDO

//使用普通IO输出供电:不用的port配置成NO_CONFIG_PORT
#if (TCFG_AUDIO_MIC_PWR_CTL & MIC_PWR_FROM_GPIO)
#define TCFG_AUDIO_MIC_PWR_PORT             IO_PORTC_02
#define TCFG_AUDIO_MIC1_PWR_PORT            NO_CONFIG_PORT
#define TCFG_AUDIO_MIC2_PWR_PORT            NO_CONFIG_PORT
#endif/*MIC_PWR_FROM_GPIO*/

//使用内部mic_ldo供电(有上拉电阻可配)
#if (TCFG_AUDIO_MIC_PWR_CTL & MIC_PWR_FROM_MIC_BIAS)
#define TCFG_AUDIO_MIC0_BIAS_EN             ENABLE_THIS_MOUDLE/*Port:PA2*/
#define TCFG_AUDIO_MIC1_BIAS_EN             ENABLE_THIS_MOUDLE/*Port:PA4*/
#define TCFG_AUDIO_MIC2_BIAS_EN             ENABLE_THIS_MOUDLE/*Port:PG7*/
#define TCFG_AUDIO_MIC3_BIAS_EN             ENABLE_THIS_MOUDLE/*Port:PG5*/
#endif/*MIC_PWR_FROM_MIC_BIAS*/

//使用内部mic_ldo供电(Port:PA0)
#if (TCFG_AUDIO_MIC_PWR_CTL & MIC_PWR_FROM_MIC_LDO)
#define TCFG_AUDIO_MIC_LDO_EN               ENABLE_THIS_MOUDLE
#endif/*MIC_PWR_FROM_MIC_LDO*/
/*>>MIC电源管理配置结束*/

#define TCFG_SUPPORT_MIC_CAPLESS 			DISABLE_THIS_MOUDLE
#define TCFG_MIC_CAPLESS_ENABLE				DISABLE_THIS_MOUDLE
#define TCFG_MIC1_CAPLESS_ENABLE			DISABLE_THIS_MOUDLE

// AUTOMUTE
#define AUDIO_OUTPUT_AUTOMUTE   			DISABLE_THIS_MOUDLE

// OUTPUT WAY
#define AUDIO_OUT_WAY_TYPE 					(AUDIO_WAY_TYPE_DAC | AUDIO_WAY_TYPE_BT)

//*********************************************************************************//
//                                 Audio VAD                                       //
//*********************************************************************************//
#define TCFG_AUDIO_VAD_ENABLE 				DISABLE_THIS_MOUDLE   //ENABLE_THIS_MOUDLE

//*********************************************************************************//
//                                  充电舱配置                                     //
//   充电舱/蓝牙测试盒/ANC测试三者为同级关系,开启任一功能都会初始化PP0通信接口     //
//*********************************************************************************//
#define TCFG_CHARGESTORE_ENABLE				DISABLE_THIS_MOUDLE         //是否支持智能充电舱
#define TCFG_TEST_BOX_ENABLE			    DISABLE_THIS_MOUDLE         //是否支持蓝牙测试盒
#define TCFG_ANC_BOX_ENABLE			        DISABLE_THIS_MOUDLE         //是否支持ANC测试盒
#define TCFG_CHARGESTORE_PORT				IO_PORTP_00                 //耳机通讯的IO口

//*********************************************************************************//
//                                  充电参数配置                                   //
//*********************************************************************************//
//是否支持芯片内置充电
#define TCFG_CHARGE_ENABLE					DISABLE_THIS_MOUDLE
//是否支持开机充电
#define TCFG_CHARGE_POWERON_ENABLE			DISABLE
//是否支持拔出充电自动开机功能
#define TCFG_CHARGE_OFF_POWERON_NE			DISABLE
/*充电截止电压可选配置*/
#define TCFG_CHARGE_FULL_V					CHARGE_FULL_V_4199
/*充电截止电流可选配置*/
#define TCFG_CHARGE_FULL_MA					CHARGE_FULL_mA_10
/*恒流充电电流可选配置*/
#define TCFG_CHARGE_MA						CHARGE_mA_200
/*涓流充电电流配置*/
#define TCFG_CHARGE_TRICKLE_MA              CHARGE_mA_20

//*********************************************************************************//
//                                  LED 配置                                       //
//*********************************************************************************//
#define TCFG_PWMLED_ENABLE					DISABLE_THIS_MOUDLE			//是否支持PMW LED推灯模块
#define TCFG_PWMLED_IOMODE					LED_ONE_IO_MODE				//LED模式，单IO还是两个IO推灯
#define TCFG_PWMLED_PIN						IO_PORTB_06					//LED使用的IO口

//*********************************************************************************//
//                                  UI 配置                                        //
//*********************************************************************************//
#define TCFG_UI_ENABLE 						ENABLE_THIS_MOUDLE 	//UI总开关
#define CONFIG_UI_STYLE                     STYLE_JL_WTACH_NEW
// #define TCFG_UI_LED7_ENABLE 			 	DISABLE_THIS_MOUDLE//UI使用LED7显示
// #define TCFG_UI_LCD_SEG3X9_ENABLE 		DISABLE_THIS_MOUDLE//UI使用LCD段码屏显示
#define TCFG_LCD_ST7735S_ENABLE	        	DISABLE_THIS_MOUDLE
#define TCFG_LCD_ST7789V_ENABLE             DISABLE_THIS_MOUDLE
#define TCFG_LCD_SPI_ST7789V_ENABLE         DISABLE_THIS_MOUDLE
#define TCFG_LCD_SPI_ST7789_BOE1_54_ENABLE  DISABLE_THIS_MOUDLE
#define TCFG_LCD_SPI_RM69330_ENABLE         DISABLE_THIS_MOUDLE
#define TCFG_LCD_SPI_SH8601A_ENABLE         ENABLE_THIS_MOUDLE
#define TCFG_LCD_MCU_JD5858_ENABLE          DISABLE_THIS_MOUDLE
#define TCFG_LCD_MCU_JD5858_ZHAOYU_ENABLE   DISABLE_THIS_MOUDLE
#define TCFG_LCD_RGB_ST7789V_ENABLE         DISABLE_THIS_MOUDLE
#define TCFG_LCD_ST7789VW_ENABLE	        DISABLE_THIS_MOUDLE
#define TCFG_LCD_OLED_ENABLE	            DISABLE_THIS_MOUDLE
#define TCFG_LCD_GC9A01_ENABLE              DISABLE_THIS_MOUDLE
#define TCFG_LRC_LYRICS_ENABLE              DISABLE_THIS_MOUDLE  //歌词显示
#define TCFG_SPI_LCD_ENABLE                 ENABLE_THIS_MOUDLE //spi lcd开关
#define TCFG_TFT_LCD_DEV_SPI_HW_NUM			1// 0: SPI0    1: SPI1    2: SPI2 配置lcd选择的spi口
#define TCFG_TOUCH_PANEL_ENABLE             ENABLE//DISABLE
#define TCFG_TOUCH_USER_IIC_TYPE            0  //0:软件IIC  1:硬件IIC
#define TCFG_TP_BL6133_ENABLE               ENABLE_THIS_MOUDLE//注意: iic 时钟需小于200k。软件IIC，系统192Mhz，TCFG_SW_I2C0_DELAY_CNT需要大于70
#define TCFG_TP_IT7259E_ENABLE              DISABLE_THIS_MOUDLE
#define TCFG_TP_FT6336G_ENABLE				DISABLE_THIS_MOUDLE
#define TCFG_TP_CST816S_ENABLE              DISABLE_THIS_MOUDLE
#define TCFG_TP_INT_IO                      IO_PORTG_05
#define TCFG_TP_RESET_IO                    IO_PORTG_06

#define TCFG_TP_SLEEP_EN					ENABLE	// 触摸屏sleep使能

#define IMB_DEMO_ENABLE                     DISABLE_THIS_MOUDLE


#define TCFG_LUA_ENABLE						DISABLE_THIS_MOUDLE	// 使用lua脚本显示

#define TCFG_UI_SHUT_DOWN_TIME              1
//0:不使用PWM控制背光
//1:使用PWM_LED模块控制背光(低功耗可正常输出，但不准且pwm脚需要加外部下拉，需要用示波器实际调试),使用该模块控制后不能再用该模块推灯
//2:使用MCPWM模块控制背光(低功耗不能输出)
#define TCFG_BACKLIGHT_PWM_MODE             0
#define TCFG_BACKLIGHT_PWM_IO               IO_PORTB_10
#if (TCFG_BACKLIGHT_PWM_MODE == 1)
#undef  TCFG_PWMLED_ENABLE
#define TCFG_PWMLED_ENABLE					DISABLE_THIS_MOUDLE			//是否支持PMW LED推灯模块
#endif

#define LVGL_TEST_ENABLE					0

#define TCFG_UI_ENABLE_MOTO                 1
#define TCFG_MOTO_PWM_H                     80
#define TCFG_MOTO_PWM_L                     30
#define TCFG_MOTO_PWM_NULL                  0
#define TCFG_MOTO_PWM_IO                    IO_PORTB_11
//*********************************************************************************//
//                                  时钟配置                                       //
//*********************************************************************************//
#define TCFG_CLOCK_SYS_SRC					SYS_CLOCK_INPUT_PLL_BT_OSC   //系统时钟源选择
//#define TCFG_CLOCK_SYS_HZ					12000000                     //系统时钟设置
//#define TCFG_CLOCK_SYS_HZ					16000000                     //系统时钟设置
#define TCFG_CLOCK_SYS_HZ					24000000                     //系统时钟设置
//#define TCFG_CLOCK_SYS_HZ					32000000                     //系统时钟设置
//#define TCFG_CLOCK_SYS_HZ					48000000                     //系统时钟设置
//#define TCFG_CLOCK_SYS_HZ					54000000                     //系统时钟设置
//#define TCFG_CLOCK_SYS_HZ					64000000                     //系统时钟设置
#define TCFG_CLOCK_OSC_HZ					24000000                     //外界晶振频率设置
#define TCFG_CLOCK_MODE                     CLOCK_MODE_ADAPTIVE


//*********************************************************************************//
//                                  低功耗配置                                     //
//*********************************************************************************//

#define TCFG_LOWPOWER_POWER_SEL				PWR_DCDC15          //电源模式设置，可选DCDC和LDO
#define TCFG_LOWPOWER_BTOSC_DISABLE			0                   //低功耗模式下BTOSC是否保持
#define TCFG_LOWPOWER_LOWPOWER_SEL			DEEP_SLEEP_EN       //芯片是否进入低功耗
#define TCFG_LOWPOWER_VDDIOM_LEVEL			VDDIOM_VOL_32V
#define TCFG_LOWPOWER_OSC_TYPE              OSC_TYPE_LRC
#define TCFG_RVDD2PVDD_DCDC					0	// 低功耗外接dcdc
#define TCFG_RVDD2PVDD_DCDC_EN_IO			IO_PORTC_06

#define TCFG_LOWPOWER_RAM_SIZE				3	// 低功耗掉电ram大小，单位：128K

//*********************************************************************************//
//                                  EQ配置                                         //
//*********************************************************************************//
//EQ配置，使用在线EQ时，EQ文件和EQ模式无效。有EQ文件时，使能TCFG_USE_EQ_FILE,默认不用EQ模式切换功能
#define TCFG_EQ_ENABLE                      0     //支持EQ功能,EQ总使能
// #if TCFG_EQ_ENABLE
#define TCFG_EQ_ONLINE_ENABLE               0     //支持在线EQ调试,如果使用蓝牙串口调试，需要打开宏 APP_ONLINE_DEBUG，否则，默认使用uart调试(二选一)
#define TCFG_BT_MUSIC_EQ_ENABLE             0     //支持蓝牙音乐EQ
#define TCFG_PHONE_EQ_ENABLE                0     //支持通话近端EQ
#define TCFG_AUDIO_OUT_EQ_ENABLE            0     //mix out级，增加eq高低音接口
#define TCFG_AEC_DCCS_EQ_ENABLE           	0     // AEC DCCS
#define TCFG_AEC_UL_EQ_ENABLE           	0     // AEC UL
#define TCFG_MUSIC_MODE_EQ_ENABLE           0     //支持音乐模式EQ
#define TCFG_PC_MODE_EQ_ENABLE              0     //支持pc模式EQ
#define TCFG_LINEIN_MODE_EQ_ENABLE          0     //支持linein近端EQ
#define TCFG_FM_MODE_EQ_ENABLE              0     //支持fm模式EQ
#define TCFG_SPDIF_MODE_EQ_ENABLE           0     //支持SPDIF模式EQ
#define TCFG_AUDIO_OUT_EQ_ENABLE			0 	  //mix_out后高低音EQ

#define TCFG_USE_EQ_FILE                    1    //离线eq使用配置文件还是默认系数表 1：使用文件  0 使用默认系数表
#if !TCFG_USE_EQ_FILE
#define TCFG_USER_EQ_MODE_NUM               7    //eq默认系数表的模式个数，默认是7个
#endif

#define EQ_SECTION_MAX                      10     //eq 段数，最大20
// #endif//TCFG_EQ_ENABLE

/*省电容mic通过eq模块实现去直流滤波*/
#if (TCFG_SUPPORT_MIC_CAPLESS && (TCFG_MIC_CAPLESS_ENABLE || TCFG_MIC1_CAPLESS_ENABLE))
#if ((TCFG_EQ_ENABLE == 0) || (TCFG_AEC_DCCS_EQ_ENABLE == 0))
#error "MicCapless enable,Please enable TCFG_EQ_ENABLE and TCFG_AEC_DCCS_EQ_ENABLE"
#endif
#endif

#define TCFG_DRC_ENABLE						0	  //DRC 总使能
#define TCFG_BT_MUSIC_DRC_ENABLE            0     //支持蓝牙音乐DRC
#define TCFG_MUSIC_MODE_DRC_ENABLE          0     //支持音乐模式DRC
#define TCFG_PC_MODE_DRC_ENABLE             0     //支持PC模式DRC
#define TCFG_LINEIN_MODE_DRC_ENABLE         0     //支持LINEIN模式DRC
#define TCFG_FM_MODE_DRC_ENABLE             0     //支持FM模式DRC
#define TCFG_SPDIF_MODE_DRC_ENABLE          0     //支持SPDIF模式DRC
#define TCFG_AUDIO_OUT_DRC_ENABLE			0 	  //mix_out后drc

// ONLINE CCONFIG,如通讯失败，请检查串口是否被占用(例如充电仓是否使能了？、usb是否使能了？)
#define TCFG_ONLINE_ENABLE                        (TCFG_EQ_ONLINE_ENABLE)    //是否支持EQ在线调试功能
#define TCFG_ONLINE_TX_PORT						  IO_PORT_DP                 //EQ调试TX口选择
#define TCFG_ONLINE_RX_PORT						  IO_PORT_DM                 //EQ调试RX口选择

/***********************************非用户配置区***********************************/
#if TCFG_EQ_ONLINE_ENABLE
#if (TCFG_USE_EQ_FILE == 0)
#undef TCFG_USE_EQ_FILE
#define TCFG_USE_EQ_FILE                    1    //开在线调试时，打开使用离线配置文件宏定义
#endif
#if TCFG_AUDIO_OUT_EQ_ENABLE
#undef TCFG_AUDIO_OUT_EQ_ENABLE
#define TCFG_AUDIO_OUT_EQ_ENABLE            0    //开在线调试时，关闭高低音
#endif
#endif
/**********************************************************************************/

//*********************************************************************************//
//                                  mic effect 配置                                //
//*********************************************************************************//
#define TCFG_MIC_EFFECT_ENABLE       DISABLE
#define TCFG_MIC_EFFECT_DEBUG        0//调试打印
#define TCFG_MIC_EFFECT_ONLINE_ENABLE  0//混响音效在线调试使能
#if ((TCFG_ONLINE_ENABLE == 0) && TCFG_MIC_EFFECT_ONLINE_ENABLE)
#undef TCFG_ONLINE_ENABLE
#define TCFG_ONLINE_ENABLE 1
#endif

#define MIC_EFFECT_REVERB             0
#define MIC_EFFECT_ECHO               1
// #define TCFG_MIC_EFFECT_SEL           MIC_EFFECT_REVERB
#define TCFG_MIC_EFFECT_SEL           MIC_EFFECT_ECHO

#if TCFG_EQ_ENABLE
#define  MIC_EFFECT_EQ_EN             0//混响音效的EQ
#endif

#define TCFG_REVERB_SAMPLERATE_DEFUAL (44100)
#define MIC_EFFECT_SAMPLERATE			(44100L)

#if TCFG_MIC_EFFECT_ENABLE
#undef MIC_SamplingFrequency
#define     MIC_SamplingFrequency         1
#undef MIC_AUDIO_RATE
#define     MIC_AUDIO_RATE              MIC_EFFECT_SAMPLERATE
#endif


/*********扩音器功能使用mic_effect.c混响流程，功能选配在effect_reg.c中 ***********/
/*********配置MIC_EFFECT_CONFIG宏定义即可********************************/
#define TCFG_LOUDSPEAKER_ENABLE            DISABLE //扩音器功能使能
#define TCFG_USB_MIC_ECHO_ENABLE           DISABLE //不能与TCFG_MIC_EFFECT_ENABLE同时打开
#define TCFG_USB_MIC_DATA_FROM_MICEFFECT   DISABLE //要确保开usbmic前已经开启混响

//*********************************************************************************//
//                                 环绕音效使能
//*********************************************************************************//
#define AUDIO_SURROUND_CONFIG     0//3d环绕

#define AUDIO_VBASS_CONFIG        0//虚拟低音,虚拟低音不支持四声道

#define AEC_PITCHSHIFTER_CONFIG   0   //通话上行变声,左耳触发变大叔声，右耳触发变女声

//*********************************************************************************//
//                                  sensor debug配置                                   //
//*********************************************************************************//
#define TCFG_SENSOR_DEBUG_ENABLE  			 	0
//*********************************************************************************//


//*********************************************************************************//
//                                  bmp-sensor配置                                   //
//*********************************************************************************//
#define TCFG_BMP280_ENABLE						  0
//*********************************************************************************//
//                                  hr-sensor配置                                   //
//*********************************************************************************//
#define TCFG_HR_SENSOR_ENABLE                     0     //hrSensor使能
#define TCFG_P11HR_SENSOR_ENABLE                  0     //使用p11 sensor hub,iic 等具体参数在具体工程配置
#define TCFG_SPO2_SENSOR_ENABLE					  0   	//SPO2sensor使能
#define TCFG_HRS3300_EN                           0
#define TCFG_HRS3603_EN				 			  0
#define TCFG_HR_SENOR_USER_IIC_TYPE               0     //0:软件IIC  1:硬件IIC
#define TCFG_HR_SENOR_USER_IIC_INDEX              0     //IIC  序号
#define TCFG_HR_SENOR_NAME                        "p11hrsensor"     //传感器名称 需要和宏匹配

//*********************************************************************************//
//                                  g-sensor配置                                   //
//*********************************************************************************//
#define TCFG_GSENSOR_ENABLE                       0    //gSensor使能
#define TCFG_DA230_EN                             0
#define TCFG_SC7A20_EN                            0
#define TCFG_P11GSENSOR_EN                        0    //使用p11 sensor hub
#define TCFG_STK8321_EN                           0
#define TCFG_IRSENSOR_ENABLE                      0
#define TCFG_JSA1221_ENABLE                       0
#define TCFG_MC3433_EN							  0
#define TCFG_MPU6050_EN							  0
#define TCFG_GSENOR_USER_IIC_TYPE                 0     //0:软件IIC  1:硬件IIC
#define TCFG_GSENOR_USER_IIC_INDEX                0     //IIC 序号
#define TCFG_GSENSOR_NAME                         "p11gsensor"     //传感器名称 需要和宏开关匹配
#define TCFG_GSENSOR_DETECT_IO                    (-1) //传感器中断io

//*********************************************************************************//
//                                  系统配置                                         //
//*********************************************************************************//
#define TCFG_AUTO_SHUT_DOWN_TIME		          0   //没有蓝牙连接自动关机时间
#define TCFG_SYS_LVD_EN						      1   //电量检测使能
#define TCFG_POWER_ON_NEED_KEY				      0	  //是否需要按按键开机配置
#define TWFG_APP_POWERON_IGNORE_DEV         	  4000//上电忽略挂载设备，0时不忽略，非0则n毫秒忽略

#if TCFG_IOKEY_ENABLE
#define TCFG_CHIP_RESET_PIN						TCFG_IOKEY0 // 长按复位
#else
#define TCFG_CHIP_RESET_PIN						IO_PORTB_01 // 长按复位
#endif
#define TCFG_CHIP_RESET_LEVEL					0 // 0-低电平复位；1-高电平复位
#define TCFG_CHIP_RESET_TIME					8 // 复位时间1 2 4 8 16 单位为秒

//*********************************************************************************//
//                                  蓝牙配置                                       //
//*********************************************************************************//
#define TCFG_USER_TWS_ENABLE                0   //tws功能使能
#define TCFG_USER_BLE_ENABLE                0   //BLE功能使能
#define TCFG_USER_BT_CLASSIC_ENABLE         1   //经典蓝牙功能使能
#define TCFG_BT_SUPPORT_AAC                 0   //AAC格式支持
#define TCFG_USER_EMITTER_ENABLE            1   //emitter功能使能
#define TCFG_BT_SNIFF_ENABLE                1   //bt sniff 功能使能
#define TCFG_BREDR_CTRL_EN                  1   //经典蓝牙手动开关使能
#define TCFG_USER_BLE_CTRL_BREDR_EN			0	//BLE控制经典蓝牙开关使能

#define USER_SUPPORT_PROFILE_SPP    0
#define USER_SUPPORT_PROFILE_HFP    1
#define USER_SUPPORT_PROFILE_A2DP   1
#define USER_SUPPORT_PROFILE_AVCTP  1
#define USER_SUPPORT_PROFILE_HID    1
#define USER_SUPPORT_PROFILE_PNP    1
#define USER_SUPPORT_PROFILE_PBAP   0

#define APP_ONLINE_DEBUG            0//在线APP调试,发布默认不开

#define TCFG_VIRTUAL_FAST_CONNECT_FOR_EMITTER      0

#if TCFG_USER_TWS_ENABLE
#define TCFG_BD_NUM						    1   //连接设备个数配置
#define TCFG_AUTO_STOP_PAGE_SCAN_TIME       0   //配置一拖二第一台连接后自动关闭PAGE SCAN的时间(单位分钟)
#define TCFG_USER_ESCO_SLAVE_MUTE           1   //对箱通话slave出声音
#else
#define TCFG_BD_NUM						    2   //连接设备个数配置
#define TCFG_AUTO_STOP_PAGE_SCAN_TIME       0 //配置一拖二第一台连接后自动关闭PAGE SCAN的时间(单位分钟)
#define TCFG_USER_ESCO_SLAVE_MUTE           0   //对箱通话slave出声音
#endif

#define BT_INBAND_RINGTONE                  0   //是否播放手机自带来电铃声
#define BT_PHONE_NUMBER                     1   //是否播放来电报号
#define BT_SYNC_PHONE_RING                  0   //是否TWS同步播放来电铃声
#define BT_SUPPORT_DISPLAY_BAT              1   //是否使能电量检测
#define BT_SUPPORT_MUSIC_VOL_SYNC           1   //是否使能音量同步

#define TCFG_BLUETOOTH_BACK_MODE			1	//后台模式

#if (TCFG_USER_TWS_ENABLE && TCFG_BLUETOOTH_BACK_MODE) && (TCFG_BT_SNIFF_ENABLE==0) && defined(CONFIG_LOCAL_TWS_ENABLE)
#define TCFG_DEC2TWS_ENABLE					0
#define TCFG_PCM_ENC2TWS_ENABLE				0
#define TCFG_TONE2TWS_ENABLE				0
#else
#define TCFG_DEC2TWS_ENABLE					0
#define TCFG_PCM_ENC2TWS_ENABLE				0
#define TCFG_TONE2TWS_ENABLE				0
#endif

//#define TWS_PHONE_LONG_TIME_DISCONNECTED

#if (APP_ONLINE_DEBUG && !USER_SUPPORT_PROFILE_SPP)
#error "NEED ENABLE USER_SUPPORT_PROFILE_SPP!!!"
#endif


//*********************************************************************************//
//                                  AI配置                                       //
//*********************************************************************************//
#define CONFIG_APP_BT_ENABLE // AI功能、流程总开关

#ifdef CONFIG_APP_BT_ENABLE
#define    TRANS_DATA_EN             0
#define    SMART_BOX_EN 			 1
#else
#define    TRANS_DATA_EN             0
#define    SMART_BOX_EN 			 0
#endif

#define ANCS_CLIENT_EN				0
#define DUEROS_DMA_EN 			 	0

#define JL_SMART_BOX_SENSORS_DATA_OPT		1

#define JL_SMART_BOX_NFC_DATA_OPT			0


#if (SMART_BOX_EN)                       //rcsp需要打开ble

#undef TCFG_USER_BLE_ENABLE
#define TCFG_USER_BLE_ENABLE         1   //双模工程，默认打开BLE功能使能

#undef CONFIG_DOUBLE_BANK_ENABLE
#define CONFIG_DOUBLE_BANK_ENABLE    1

#define RCSP_UPDATE_EN		         1     //是否支持rcsp升级
#define OTA_TWS_SAME_TIME_ENABLE     0     //是否支持TWS同步升级
#define UPDATE_MD5_ENABLE            0     //升级是否支持MD5校验
#define RCSP_FILE_OPT				 1
#define JL_EARPHONE_APP_EN			 1
#define TCFG_BS_DEV_PATH_EN			 1
#define WATCH_FILE_TO_FLASH          1
#define UPDATE_EX_FALSH_USE_4K_BUF   1
#else
#define OTA_TWS_SAME_TIME_ENABLE     0
#define RCSP_UPDATE_EN               0
#define UPDATE_MD5_ENABLE            0     //升级是否支持MD5校验
#define RCSP_FILE_OPT				 0
#define JL_EARPHONE_APP_EN			 0
#define TCFG_BS_DEV_PATH_EN			 0
#define WATCH_FILE_TO_FLASH          0
#define UPDATE_EX_FALSH_USE_4K_BUF   0
#endif


#if (SMART_BOX_EN)
#undef     JL_SMART_BOX_CUSTOM_APP_EN
#define    JL_SMART_BOX_CUSTOM_APP_EN
#define		JL_SMART_BOX_SIMPLE_TRANSFER 1
#endif

#define JL_SMART_BOX_EXTRA_FLASH_OPT		1

#if TCFG_APP_RTC_EN
// 0 - 旧闹钟，只支持提示音闹铃
// 1 - 新闹钟，可选择提示音或者设备音乐作为闹铃
#define CUR_RTC_ALARM_MODE				1
#endif

#ifdef JL_SMART_BOX_CUSTOM_APP_EN

#define SMARTBOX_SPP_INTERACTIVE_SUPPORT    1
#define SMARTBOX_BLE_INTERACTIVE_SUPPORT    1

#define RCSP_USE_BLE      0
#define RCSP_USE_SPP      1
#if TCFG_USER_BLE_CTRL_BREDR_EN
#define RCSP_CHANNEL_SEL  RCSP_USE_BLE
#else
#define RCSP_CHANNEL_SEL  RCSP_USE_SPP
#endif

#define RCSP_SMARTBOX_ADV_EN			0

// 关闭smartbox的音乐模式和rtc模式
#define SMARTBOX_APP_MUSIC_EN			(0)
#define SMARTBOX_APP_RTC_EN				(0)

// 屏蔽音效
#define RCSP_SOUND_EFFECT_FUNC_DISABLE	1

#if  RCSP_SMARTBOX_ADV_EN
#define RCSP_ADV_NAME_SET_ENABLE        1
#define RCSP_ADV_KEY_SET_ENABLE         0
#define RCSP_ADV_LED_SET_ENABLE         1
#define RCSP_ADV_MIC_SET_ENABLE         0
#define RCSP_ADV_WORK_SET_ENABLE        0
#else
#define RCSP_ADV_NAME_SET_ENABLE        0
#define RCSP_ADV_KEY_SET_ENABLE         0
#define RCSP_ADV_LED_SET_ENABLE         0
#define RCSP_ADV_MIC_SET_ENABLE         0
#define RCSP_ADV_WORK_SET_ENABLE        0
#endif

#define RCSP_ADV_MUSIC_INFO_ENABLE      0
#define RCSP_ADV_PRODUCT_MSG_ENABLE     1
#define RCSP_ADV_COLOR_LED_SET_ENABLE   0
#define RCSP_ADV_KARAOKE_SET_ENABLE		0
#define RCSP_ADV_KARAOKE_EQ_SET_ENABLE	0
#define RCSP_ADV_HIGH_LOW_SET			0
#define RCSP_ADV_EQ_SET_ENABLE          0
#define RCSP_ADV_FIND_DEVICE_ENABLE     1

#endif



//*********************************************************************************//
//                           编解码格式配置(CodecFormat)                           //
//*********************************************************************************//
#define TCFG_DEC_MP3_ENABLE                 ENABLE
#define TCFG_DEC_WTGV2_ENABLE				DISABLE
#define TCFG_DEC_G729_ENABLE                ENABLE
#define TCFG_DEC_WMA_ENABLE					DISABLE
#define TCFG_DEC_WAV_ENABLE					DISABLE
#define TCFG_DEC_FLAC_ENABLE				DISABLE
#define TCFG_DEC_APE_ENABLE					DISABLE
#define TCFG_DEC_M4A_ENABLE					DISABLE
#define TCFG_DEC_ALAC_ENABLE				DISABLE
#define TCFG_DEC_AMR_ENABLE					DISABLE
#define TCFG_DEC_DTS_ENABLE					DISABLE
#define TCFG_DEC_G726_ENABLE			    DISABLE
#define TCFG_DEC_MIDI_ENABLE			    DISABLE
#define TCFG_DEC_MTY_ENABLE					DISABLE
#define TCFG_DEC_SBC_ENABLE					ENABLE
#define TCFG_DEC_PCM_ENABLE					DISABLE
#define TCFG_DEC_CVSD_ENABLE				DISABLE
#define TCFG_DEC_LC3_ENABLE                 DISABLE

#define TCFG_WAV_TONE_MIX_ENABLE			DISABLE	//wav提示音叠加播放

#define TCFG_APP_FM_EMITTER_EN              DISABLE_THIS_MOUDLE

#define TCFG_AUDIO_INPUT_IIS                DISABLE_THIS_MOUDLE
#define TCFG_AUDIO_OUTPUT_IIS               DISABLE_THIS_MOUDLE


//*********************************************************************************//
//                                  REC 配置                                       //
//*********************************************************************************//
#define RECORDER_MIX_EN						DISABLE//混合录音使能, 需要录制例如蓝牙、FM、 LINEIN才开
#define TCFG_RECORD_FOLDER_DEV_ENABLE       DISABLE//ENABLE//音乐播放录音区分使能
#define RECORDER_MIX_BT_PHONE_EN			ENABLE//电话录音使能


//*********************************************************************************//
//                                  linein配置                                     //
//*********************************************************************************//
#define TCFG_LINEIN_ENABLE					TCFG_APP_LINEIN_EN	// linein使能
// #define TCFG_LINEIN_LADC_IDX				0					// linein使用的ladc通道，对应ladc_list
#define TCFG_LINEIN_LR_CH					LADC_LINE0_MASK
#define TCFG_LINEIN_CHECK_PORT				IO_PORTB_01			// linein检测IO
#define TCFG_LINEIN_PORT_UP_ENABLE        	1					// 检测IO上拉使能
#define TCFG_LINEIN_PORT_DOWN_ENABLE       	0					// 检测IO下拉使能
#define TCFG_LINEIN_AD_CHANNEL             	NO_CONFIG_PORT		// 检测IO是否使用AD检测
#define TCFG_LINEIN_VOLTAGE                	0					// AD检测时的阀值
#if(TCFG_MIC_EFFECT_ENABLE)
#define TCFG_LINEIN_INPUT_WAY               LINEIN_INPUT_WAY_ANALOG
#else
#if (RECORDER_MIX_EN)
#define TCFG_LINEIN_INPUT_WAY               LINEIN_INPUT_WAY_ADC//LINEIN_INPUT_WAY_ANALOG
#else
#define TCFG_LINEIN_INPUT_WAY               LINEIN_INPUT_WAY_ANALOG
#endif/*RECORDER_MIX_EN*/
#endif
#define TCFG_LINEIN_MULTIPLEX_WITH_FM		DISABLE 				// linein 脚与 FM 脚复用
#define TCFG_LINEIN_MULTIPLEX_WITH_SD		DISABLE 				// linein 检测与 SD cmd 复用
#define TCFG_LINEIN_SD_PORT		            0// 0:sd0 1:sd1     //选择复用的sd

//*********************************************************************************//
//                                 USB 配置                                        //
//*********************************************************************************//
#define TCFG_PC_ENABLE						TCFG_APP_PC_EN//PC模块使能
#define TCFG_UDISK_ENABLE					0//ENABLE_THIS_MOUDLE//U盘模块使能
#define TCFG_HOST_AUDIO_ENABLE				DISABLE_THIS_MOUDLE
#define TCFG_HID_HOST_ENABLE				DISABLE_THIS_MOUDLE
#define TCFG_OTG_USB_DEV_EN                 BIT(0)//USB0 = BIT(0)  USB1 = BIT(1)

#include "usb_std_class_def.h"


#define TCFG_USB_PORT_CHARGE            DISABLE

#undef USB_DEVICE_CLASS_CONFIG
#define     USB_DEVICE_CLASS_CONFIG 		(MASSSTORAGE_CLASS|SPEAKER_CLASS|MIC_CLASS|HID_CLASS)


//*********************************************************************************//
//                                  rtc 配置                                     //
//*********************************************************************************//
#define TCFG_RTC_ENABLE						TCFG_APP_RTC_EN

#define TCFG_RTC_USE_LRC					DISABLE // 使用lrc作为rtc时钟（省32k晶振）


//*********************************************************************************//
//                                  fat 文件系统配置                                       //
//*********************************************************************************//
#define CONFIG_FATFS_ENABLE					ENABLE
#define TCFG_LFN_EN                         1

//*********************************************************************************//
//                                 电源切换配置                                    //
//*********************************************************************************//

#define CONFIG_PHONE_CALL_USE_LDO15	    0


//*********************************************************************************//
//                                  UI APP                                         //
//*********************************************************************************//

#define TCFG_UI_ENABLE_STANDBY_DIAL			ENABLE // 表盘浏览
#define TCFG_UI_ENABLE_PULLDOWN_MENU		ENABLE // 下拉菜单
#define TCFG_UI_ENABLE_LEFT_MENU            DISABLE // 左滑侧边栏,ui_page_manager.h把SIDEBAR_LEFT_MENU_ENABLE置1关闭卡片管理表盘右滑功能
#define TCFG_UI_ENABLE_SYS_SET              ENABLE

// 需要使能TCFG_APP_RTC_EN
#define TCFG_UI_ENABLE_ALARM				ENABLE // 闹钟功能
#define TCFG_UI_ENABLE_STOPWATCH			ENABLE // 计时器
#define TCFG_UI_ENABLE_TIMER_ACTION			ENABLE // 倒计时

// 需要使能TCFG_APP_PC_EN
#define TCFG_UI_ENABLE_PC					ENABLE // PC模式

// 需要使能TCFG_APP_MUSIC_EN
#define TCFG_UI_ENABLE_MUSIC				ENABLE // 音乐
#define TCFG_UI_ENABLE_MUSIC_MENU			ENABLE // 音乐菜单
#define TCFG_UI_ENABLE_FILE					ENABLE // 文件浏览

// 需要使能TCFG_USER_EMITTER_ENABLE
#define TCFG_UI_ENABLE_BT_PAGE_ACTION		ENABLE // 蓝牙耳机列表
#define TCFG_UI_ENABLE_BT_SCAN				ENABLE // 搜索蓝牙耳机
#define TCFG_UI_ENABLE_PAGE_TOUCH			ENABLE // 已保存蓝牙耳机列表
#define TCFG_UI_ENABLE_SCAN_TOUCH			ENABLE // 搜索蓝牙耳机

#define TCFG_UI_ENABLE_PHONE_ACTION			ENABLE // 蓝牙通话
#define TCFG_UI_ENABLE_PHONEBOOK			ENABLE // 电话本
#define TCFG_UI_ENABLE_FINDPHONE			ENABLE // 找手机
#define TCFG_UI_ENABLE_NOTICE				ENABLE // 手机消息通知
#define TCFG_UI_ENABLE_WEATHER				ENABLE // 天气

#define TCFG_UI_ENABLE_FLASHLIGHT			ENABLE // 手电筒
#define TCFG_UI_ENABLE_QR_CODE				ENABLE // 二维码

// 需要使能TCFG_BMP280_ENABLE
#define TCFG_UI_ENABLE_ALTIMETER			ENABLE // 海拔高度
#define TCFG_UI_ENABLE_PRESSURE				ENABLE // 海拔气压

// 需要使能TCFG_GSENSOR_ENABLE、TCFG_HR_SENSOR_ENABLE
#define TCFG_UI_ENABLE_SPORT_INFO			ENABLE // 运动状态
#define TCFG_UI_ENABLE_SPORTRECORD			ENABLE // 运动记录
#define TCFG_UI_ENABLE_TRAIN				ENABLE // 锻炼
#define TCFG_UI_ENABLE_SPORTRECORD_BROWSE	ENABLE // 运动记录浏览
#define TCFG_UI_ENABLE_OXYGEN				ENABLE // 血氧饱和度
#define TCFG_UI_ENABLE_HEART				ENABLE // 心率
#define TCFG_UI_ENABLE_BREATH_TRAIN			ENABLE // 呼吸训练
#define TCFG_UI_ENABLE_HEAT					DISABLE // 压力
#define TCFG_UI_ENABLE_SLEEP				ENABLE // 睡眠

#define TCFG_UI_ENABLE_DEMO                 DISABLE //示例
//*********************************************************************************//
//                                 配置结束                                        //
//*********************************************************************************//


#if TCFG_MIC_EFFECT_ENABLE
#ifdef TCFG_AEC_ENABLE
#undef TCFG_AEC_ENABLE
#define TCFG_AEC_ENABLE 0
#endif//TCFG_AEC_ENABLE
#endif

#endif //CONFIG_BOARD_701N_DEMO
#endif //CONFIG_BOARD_701N_DEMO_CFG_H
