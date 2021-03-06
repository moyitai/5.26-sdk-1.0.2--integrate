/*
Copyright (c) 2017 Silan MEMS. All Rights Reserved.
*/

#ifndef SL_Watch_PEDO_KCAL_WRIST_SLEEP_SWAY_ALGO_DRV__H__
#define SL_Watch_PEDO_KCAL_WRIST_SLEEP_SWAY_ALGO_DRV__H__
#include <stdlib.h>
#include <typedef.h>
// unsigned char get_sleep_status(void);
// unsigned int get_step_count(void);
#define SL_Sensor_Algo_Release_Enable 0x01
//0x00: for debug
//0x01: release version

/***ʹ������ǰ�����ʵ�ʽ����������******/
/**SC7A20��SDO �Žӵأ�  0****************/
/**SC7A20��SDO �Žӵ�Դ��1****************/
#define SL_SC7A20_SDO_VDD_GND            1
/*****************************************/

/***ʹ������ǰ�����ʵ��IIC�����������***/
/**SC7A20��IIC �ӿڵ�ַ���� 7bits��  0****/
/**SC7A20��IIC �ӿڵ�ַ���� 8bits��  1****/
#define SL_SC7A20_IIC_7BITS_8BITS        1
/*****************************************/

#if SL_SC7A20_SDO_VDD_GND==0
#define SL_SC7A20_IIC_7BITS_ADDR        0x18
#define SL_SC7A20_IIC_8BITS_WRITE_ADDR  0x30
#define SL_SC7A20_IIC_8BITS_READ_ADDR   0x31
#else
#define SL_SC7A20_IIC_7BITS_ADDR        0x19
#define SL_SC7A20_IIC_8BITS_WRITE_ADDR  0x32
#define SL_SC7A20_IIC_8BITS_READ_ADDR   0x33
#endif

#if SL_SC7A20_IIC_7BITS_8BITS==0
#define SL_SC7A20_IIC_ADDRESS        SL_SC7A20_IIC_7BITS_ADDR
#else
#define SL_SC7A20_IIC_WRITE_ADDRESS  SL_SC7A20_IIC_8BITS_WRITE_ADDR
#define SL_SC7A20_IIC_READ_ADDRESS   SL_SC7A20_IIC_8BITS_READ_ADDR
#endif

#define SL_SC7A20_CHIP_ID_ADDR       (unsigned char)0x0F
#define SL_SC7A20_CHIP_ID_VALUE      (unsigned char)0x11
#define SL_SC7A20_VERSION_ADDR       (unsigned char)0x70
#define SL_SC7A20_VERSION_VALUE      (unsigned char)0x11
#define SL_SC7A20E_VERSION_VALUE     (unsigned char)0x26

//typedef enum {FALSE = 0,TRUE = !FALSE} bool;

/********�ͻ���Ҫ���е�IIC�ӿڷ������****************/
extern unsigned char SL_SC7A20_I2c_Spi_Write(bool sl_spi_iic,unsigned char reg, unsigned char dat);
extern unsigned char SL_SC7A20_I2c_Spi_Read(bool sl_spi_iic,unsigned char reg, unsigned char len, unsigned char *buf);
/**SL_SC7A20_I2c_Spi_Write �����У� sl_spi_iic:0=spi  1=i2c  Reg���Ĵ�����ַ   dat���Ĵ���������ֵ*******************/
/**SL_SC7A20_I2c_Spi_Write ���� ��һ������д�ĺ���*******************************************************************/
/***SL_SC7A20_I2c_Spi_Read �����У� sl_spi_iic:0=spi  1=i2c Reg ͬ�ϣ�len:��ȡ���ݳ��ȣ�buf:�洢�����׵�ַ��ָ�룩***/
/***SL_SC7A20_I2c_Spi_Read ���� �ǿ��Խ��е��ζ�����������ȡ�ĺ���*************************************************/

/*************������ʼ������**************/
signed char SL_SC7A20_Driver_Init(bool Sl_spi_iic_init,unsigned char Sl_pull_up_mode);
/***�������:1,Sl_spi_iic_init:0-1***2,PULL_UP_MODE:0x00 0x08 0x04 0x0c********/
/****Sl_spi_iic_init=0��SPI MODE, Sl_pull_up_mode config failed****************/
/****Sl_spi_iic_init=1��IIC MODE***********************************************/
/****Sl_pull_up_mode=0x00: SDO  I2C  pull up***********************************/
/****Sl_pull_up_mode=0x08: I2C  pull up and  SDO  open drain*******************/
/****Sl_pull_up_mode=0x04: SDO  pull up and  I2C  open drain*******************/
/****Sl_pull_up_mode=0x0C: SDO  I2C  open drain********************************/
/****SDO�Žӵأ��Ƽ��ر�SDO�ڲ���������****************************************/
/****SPIͨѶ���Ƽ��ر�SDA��SCL�ڲ��������裬SDO�ڲ�����������Զ��ر�**********/

/*************���������������*****************/
/**return : 0x11/0x26 ��ʾCHIP ID ����*********/
/**return : 0         ��ʾ��ȡ�쳣*************/
/**return :-1;        SPI ͨ������*************/
/**return :-2;        IIC ͨ������*************/
/**return :-3;        ������ȡ ͨ������********/

/***************ִ���㷨ǰ��Ҫ��ȡFIFO����*****/
/***************FIFO���ݶ�ȡ��FIFO�����*****/
/***************����FIFOֻ�ܶ�ʱ��ȡһ��*******/
unsigned char SL_SC7A20_Read_FIFO(void);
/**return : FIFO_LEN    ��ʾ���鳤��***********/


/****˵��:�ú�����Ҫ��ʱִ��,�Ӷ���֤�㷨ִ�е�λ**/
/***************ִ���㷨��ȡ�Ʋ����***************/
unsigned int SL_Watch_Kcal_Pedo_Algo(bool sl_music_motor_en);
/***sl_music_motor_en=0: �����������ֹ���δ��**/
/***sl_music_motor_en=1: �����������ֹ����Ѵ�**/
/**************��ȡ����ĵ�ǰ�Ʋ�ֵ****************/
/*************�������Ϊ���Ʋ�ֵ(��)***************/

/***************��ȡ����ԭʼ����***************/
unsigned char SL_SC7A20_GET_FIFO_Buf(signed short *sl_x_buf,signed short *sl_y_buf,signed short *sl_z_buf,bool filter_en);
/****************ִ���㷨��ִ�и�����**********/
/**x_buf  y_buf  z_buf : ����32�������׵�ַ****/
/**filter_en��0 ��ͨ�˲���ֹ 1����ͨ�˲�ʹ��***/
/****************���������������**************/
/**return : FIFO_LEN    ��ʾ���鳤��***********/


/******************��λ�Ʋ�ֵ************************/
void SL_Pedo_Kcal_ResetStepCount(void);
/**********��������Ϊ����******�������Ϊ����********/
/*ʹ�÷���: ϵͳʱ�䵽�ڶ���ʱ�����øú�������Ʋ�ֵ*/

/*******************�Ʋ�״̬��λ**********************/
void SL_Pedo_WorkMode_Reset(void);
/******************�����������***********************/
/******************���ֵ��������*********************/

/******************���������üƲ�ֵ,����,����************************/
/******************���������ڳ�ʼ�����������Ӹú���****************/
void SL_Pedo_StepCount_Set(unsigned int sl_pedo_value,unsigned int sl_dis_value,unsigned int sl_kcal_value);
/**********��������Ϊ��sl_pedo_value  �Ʋ�ֵ*****/
/**********��������Ϊ��sl_dis_value   ����ֵ*****/
/**********��������Ϊ��sl_kcal_value  ����ֵ*****/

/******************�Ʋ�����������********************/
void SL_PEDO_TH_SET(unsigned char sl_pedo_amp,unsigned char sl_pedo_th,unsigned char sl_pedo_weak,unsigned char sl_zcr_lel,unsigned char sl_scope_lel);
/******sl_pedo_amp>4&&sl_pedo_amp<201**************************/
/******sl_pedo_amp:ԽС������Խ�ߣ�Խ���׼Ʋ�******************/
/******sl_pedo_amp:Խ��������Խ�ߣ�Խ�ѼƲ�********************/
/******sl_pedo_amp:Ĭ��ֵ26�������þ���26**********************/

/******sl_pedo_th>5&&sl_pedo_th<50*****************************/
/******sl_pedo_th:ԽС������Խ�ߣ�Խ���׼Ʋ�*******************/
/******sl_pedo_th:Խ��������Խ�ߣ�Խ�ѼƲ�*********************/
/******sl_pedo_th:Ĭ��ֵ10�������þ���10***********************/

/******sl_pedo_weak>=0&&sl_pedo_weak<6*************************/
/******sl_pedo_th:0,������΢��·�Ʋ�����*********************/
/******sl_pedo_th:1,����΢��·�Ʋ�����***********************/
/******sl_pedo_th:2,����΢��·�Ʋ�����***********************/
/******sl_pedo_th:3,����΢��·�Ʋ�����***********************/
/******sl_pedo_th:4,����΢��·�Ʋ�����***********************/
/******sl_pedo_th:5,����΢��·�Ʋ�����***********************/
/******sl_pedo_th:Ĭ��ֵ0,���õ�ֵԽ�󣬼��Ʋ���Խ����*******/

/*****sl_zcr_lel>=0&&sl_zcr_lel<=255***************************/
/*****sl_zcr_lel:Ĭ��ֵ=20,���õ�ֵԽС��Խ��������������****/
/*****һ��ʱ���ڵĹ����ʼ���***********************************/

/*****sl_scope_lel>=0&&sl_scope_lel<=255***********************/
/*****�������ݷ�����ֵ*****************************************/


/*******�Ʋ���������****************/
void SL_PEDO_SET_AXIS(unsigned char sl_xyz);
/***sl_xyz:0  x��*******************/
/***sl_xyz:1  y��*******************/
/***sl_xyz:2  z��*******************/
/***sl_xyz:3  �㷨����**************/

/*�����˶���������ֵ��ʱ����ֵ****/
void SL_PEDO_INT_SET(unsigned char V_TH,unsigned char T_TH,bool INT_EN);
/***V_TH:0  0-127************************************/
/***T_TH:1  0-127************************************/
/***INT_EN:0 �ر��жϹ��ܣ�1�����ж�״̬��⹦��***/
/***USE AOI2 INT*************************************/

/*�����˶�״̬��ȡ******************/
bool SL_INT_STATUS_READ(void);
/*0��  û���˶�*********************/
/*1��  ���˶�***********************/
/*�ر��жϹ�������£�һֱ���1*****/

/***********�ر�IIC�豸**************/
/***********Power down ����**********/
/********��ʹ�ø��豸�����**********/
/****����������ʹ��������ʼ������****/
bool  SL_SC7A20_Power_Down(void);
/*************�������:��************/
/**********���ز������˵��**********/
/**return  1: Power Down Success*****/
/**return  0: Power Down Fail********/





/*************��ʼ�����˲���*************/
/**������ʼ���������������������********/
void SL_Pedo_Person_Inf_Init(unsigned char *Person_Inf_Init);
/*********����ָ������ֱ���:���� ���� ���� �Ա�***����:178,60,26 1*********/
/**���߷�Χ:  30cm ~ 250cm  ***********/
/**���ط�Χ:  10Kg ~ 200Kg  ***********/
/**���䷶Χ:  3��  ~ 150��  ***********/
/**�Ա�Χ:  0 ~ 1    0:Ů 1:��   ****/

/*********************��ȡ�˶�״ֵ̬**********************/
unsigned char SL_Pedo_GetMotion_Status(void);
/**********��������Ϊ����*********************************/
/**********�������Ϊ��0 ~ 3 *****************************/
/**�������Ϊ��0   *��ֹ����**********/
/**�������Ϊ��1   *���߻�ɢ��**********/
/**�������Ϊ��2   *������·************/
/**�������Ϊ��3   *�ܲ�������˶�******/

/**************��ȡ��ֹ��ǰ��������߾���*****************/
unsigned int SL_Pedo_Step_Get_Distance(void);
/*******************��������Ϊ����************************/
/*******************�������Ϊ���������߾��� *************/
/*******************��λ:       ����(dm)******************/

/**************��ȡ��ֹ��ǰ�������������*****************/
unsigned int SL_Pedo_Step_Get_KCal(void);
/*******************��������Ϊ����************************/
/*******************�������Ϊ��������������ֵ ***********/
/*********��λ: ��     1����λ=0.1��******************/

/**************��ȡ���һ��ʱ�����·ƽ������*************/
unsigned short SL_Pedo_Step_Get_Avg_Amp(void);
/********************��������Ϊ����***********************/
/***********************1LSB��XXXmg************************/

/***********************��ȡ��ǰ��·ƽ����Ƶ**************/
unsigned char SL_Pedo_Step_Get_Step_Per_Min(void);
/********************��������Ϊ����***********************/
/********************�������Ϊ��XXX��/���� **************/

/***********************��ȡ��ǰ���˶��ȼ�****************/
unsigned char SL_Pedo_Step_Get_Motion_Degree(void);
/********************��������Ϊ����***********************/
/********************�������Ϊ��0-25 ********************/


/**********̧�������㷨**********/

/*****************̧�������㷨��ʼ������****************/
void SL_Turn_Wrist_Init(unsigned char *SL_Turn_Wrist_Para);
/************���������*************/
/******SL_Turn_Wrist_Para[0]:���ٶȼ���Ƭλ������  0--7********/
/******SL_Turn_Wrist_Para[1]:̧����������������    1--5********/
/******SL_Turn_Wrist_Para[2]:ˮƽ̧������ʹ�ܿ���λ0--1********/

/*************������ٶȼ���Ƭλ������*************************/
/***SL_Turn_Wrist_Para[0]���趨ֵ�ķ�ΧΪ: 0 ~ 7 **************/
/***��ο��ĵ�:Silan_MEMS_�ֻ��㷨˵����_V1.1.pdf**************/

/***SL_Turn_Wrist_Para[1]:�趨ֵ�ķ�ΧΪ: 1 ~ 5 **/
/*********Ĭ��ֵΪ��3   �е�������****************/
/*********�趨ֵΪ��1   ���������****************/
/*********Ĭ��ֵΪ��5   ���������****************/
/*********�趨ֵΪ��1(��ٶ�)~5(������)***********/

/******SL_Turn_Wrist_Para[2]:ˮƽ̧������ʹ�ܿ���λ0--1*******/
/******0����ֹˮƽ̧�ֹ���********/
/******1��ʹ��ˮƽ̧�ֹ���********/



/*****************̧������״̬��ȡ����****************/
signed char SL_Watch_Wrist_Algo(void);
/***********�����������***********/
/***********���ز������˵��*******/
/**Return:  2     ��ע˵��:��Ļ��������Ҫ�ر�*********/
/**Return:  1     ��ע˵��:��Ļ��Ҫ����***************/
/**Return:  0     ��ע˵��:��Ļ����Ҫ����*************/
/**Return: -1     ��ע˵��:δ��ʼ�����ʼ��ʧ��*******/

/*****************̧������״̬��λ********************/
//void SL_Turn_Wrist_WorkMode_Reset(void);
/******************�����������***********************/
/******************���ֵ��������*********************/


//3-12
//1-10
//0-10
/******************˯�߲�������********************/
void SL_Sleep_Para(unsigned char adom_time_th,unsigned char sleep_vpp_th,unsigned char sleep_time_th);
/**************************�������ʱ��***********************/
/******adom_time_th:0-255 min***********************************/
/******adom_time_th:δ���ʱ�䣬����������ֵ������Ϊδ���****/
/******adom_time_th:����ֵԽС��Խ���׳���δ������************/

/**************************˯���м����ֵ***********************/
/******sleep_vpp_th:ԽС������Խ�ߣ�Խ����˯��״̬�л�**********/
/******sleep_vpp_th:Խ��������Խ�ͣ�Խ��˯��״̬�л�************/
/******sleep_vpp_th:Ĭ��ֵ10�������þ���10**********************/

/**************************״̬�л���Сʱ��*********************/
/******sleep_time_th:��ֵԽС��˯��״̬�л�������ʱ���Խ��*****/
/******sleep_time_th:��ֵԽ��˯��״̬�л�������ʱ���Խ��*****/
/******sleep_time_th:Ĭ��ֵ1�������þ���1***********************/


/***************��ȡ��ǰ��˯��״̬*****************/
unsigned char SL_Sleep_GetStatus(unsigned char SL_Sys_Time);
/*******SL_Sys_Time����ǰ��ʱ�䣬��СʱΪ��λ******/
/*******ȫ�������ʱ�䷶ΧΪ��0-23 ****************/
/***************������ݷ�Χ��0-7******************/
/***************0������״̬************************/
/***************7�����˯��************************/

/***************��ȡ��ǰ�Ļ�ȼ�*****************/
unsigned char SL_Sleep_Active_Degree(unsigned char mode);
/***************mode��0 ��ʱ��0.5s�е���***********/
/***************mode��1 ��ʱ��1min�е���***********/
/***************������ݷ�Χ��0-255****************/
/***************0����ֹ****************************/
/*************255���˶�****************************/

unsigned char SL_Sleep_Get_Active_Degree(void);
/***************mode��0 ��ʱ��0.5s�е���***********/
/***************mode��1 ��ʱ��1min�е���***********/
/***************������ݷ�Χ��0-255****************/
/***************0����ֹ****************************/
/*************255���˶�****************************/

/***************��ȡ��ǰ����������***************/
unsigned char SL_Adom_GetStatus(void);
/***************0��δ���**************************/
/***************1�������**************************/

/***************��ȡ��ǰ����������***************/
unsigned char SL_In_Sleep_Status(void);
/***************0��out sleep**********************/
/***************1��in sleep***********************/

/**************************��ת����************************/
bool SL_Get_Clock_Status(bool open_close);
/***********���������sensor_pos***********/
/****sensor_pos:1 open  ����Ƿ�Ҫ�ر�ʱ���� 1*************/
/****sensor_pos:0 close �ر�����ʱʱ    ���� 0*************/
/*********************���ز������˵��*********************/
/***********************��ת���ܼ��***********************/
/**Return:  1     ��ע˵��:��Ļ�ѷ�ת���ر�����************/
/**Return:  0     ��ע˵��:��Ļδ��ת�����ر�����**********/


/**************************ҡ�ι���************************/
bool SL_Get_Phone_Answer_Status(unsigned char Sway_Degree,unsigned char Sway_Num);
/***********���������ҡ�εȼ�0--10  ҡ������0--10***********************/
/***ҡ�εȼ�������ֵԽС����Ҫҡ�εķ���ԽС��Խ���״�����������*********/
/***ҡ������������ֵԽС����Ҫҡ�εĴ���Խ�٣�Խ���״�����������*********/
/***********���ز������˵��*******************************/
/**Return:  1     ��ע˵��:ҡ�ζ��������������绰**********/
/**Return:  0     ��ע˵��:ҡ�ζ������������������绰******/


/************�Ӳ���Ŀ�ͻ��˲���***********/
/***��ʼ����ʱ����������ж�FIFO�Ƿ�����**/
bool SL_SC7A20_FIFO_TEST(void);
/*************�������:��*****************/
/*************���������������************/
/**return :1  FIFO �쳣*******************/
/**return :0  FIFO ����*******************/
#endif/****SL_Watch_ALGO_DRV__H__*********/



