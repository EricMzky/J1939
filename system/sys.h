#ifndef __SYS_H
#define __SYS_H	

#include "stm32f7xx.h" 


#define  BOOT  1

#ifdef   BOOT
#define  FLASH_OFFSET  0x0
#else
#define  FLASH_OFFSET  0x10000

#endif

typedef unsigned int U32_t;
typedef unsigned short U16_t;


typedef volatile unsigned int VU32_t;
typedef volatile unsigned short VU16_t;
typedef volatile unsigned char VU8_t; 


/**/
typedef unsigned char      GM_UINT8;
typedef unsigned short     GM_UINT16;
typedef unsigned long      GM_UINT32;
typedef signed char        GM_INT8;
typedef signed short int   GM_INT16;
typedef signed long int    GM_INT32;
typedef float              GM_FLOAT;
typedef double             GM_DOUBLE;
typedef int                GM_BOOL;


/********* �ܽź� *********/
#define CFG_PIN0    0
#define CFG_PIN1    1
#define CFG_PIN2    2
#define CFG_PIN3    3
#define CFG_PIN4    4
#define CFG_PIN5    5
#define CFG_PIN6    6
#define CFG_PIN7    7
#define CFG_PIN8    8
#define CFG_PIN9    9
#define CFG_PIN10   10
#define CFG_PIN11   11
#define CFG_PIN12   12
#define CFG_PIN13   13
#define CFG_PIN14   14
#define CFG_PIN15   15

/********* GPIO����ѡ�� *********/
#define CFG_MODE_IN    	0		/*  ��ͨ����ģʽ	*/
#define CFG_MODE_OUT	1		/*  ��ͨ���ģʽ	*/
#define CFG_MODE_AF		2		/*  ���ù���ģʽ	*/
#define CFG_MODE_AIN	3		/*  ģ������ģʽ	*/
#define CFG_SPEED_2M	0		/*  ����ٶ�2Mhz(����)	*/
#define CFG_SPEED_25M	1		/*  ����ٶ�25Mhz(����)	*/
#define CFG_SPEED_50M	2		/*  ����ٶ�50Mhz(����)	*/
#define CFG_SPEED_100M	3		/*  ����ٶ�100Mhz(����)	*/
#define CFG_PUPD_NONE	0		/*  ��������	*/
#define CFG_PUPD_PU		1		/*  ���� */
#define CFG_PUPD_PD		2		/*  ���� */
#define CFG_PUPD_RES	3		/*  ���� */
#define CFG_OTYPE_PP	0		/*  �������	*/
#define CFG_OTYPE_OD	1		/*  ��©���	*/

/********* ���ù���ѡ�� *********/
#define AF0     0       		/*  MCO/SWD/SWCLK/RTC      */
#define AF1     1       		/*  TIM1~TIM2  */
#define AF2     2       		/*  TIM3~TIM5  */
#define AF3     3       		/*  TIM8~TIM11  */
#define AF4     4       		/*  I2C1~I2C4  */
#define AF5     5       		/*  SPI1~SPI6  */
#define AF6     6       		/*  SPI3/SAI1  */
#define AF7     7       		/*  SPI2/SPI23/USART1~3/USART5/SPDIFRX     */
#define AF8     8       		/*  USART4~USART8/SPDIFRX/SAI2    */
#define AF9     9       		/*  CAN1~CAN2/TIM12~TIM14/LCD/QSPI      */
#define AF10    10      		/*  USB_OTG/USB_HS/SAI2/QSPI     */
#define AF11    11      		/*  ETH  */
#define AF12    12      		/*  FMC/SDMMC/OTG/HS     */
#define AF13    13      		/*  DCIM  */
#define AF14    14      		/*  LCD  */
#define AF15    15      		/*  EVENTOUT   */


/*********GPIOxģʽ����*********/
#define Px_MODER_SET(x,pin,mode)		( x->MODER   = (x->MODER   & ~((U32_t)0x3<<pin*2) | ((U32_t)mode <<pin*2)) )

/*********GPIOx����ٶ�����*********/
#define Px_OSPEEDR_SET(x,pin,speed)	    ( x->OSPEEDR = (x->OSPEEDR & ~((U32_t)0x3<<pin*2) | ((U32_t)speed <<pin*2)) )

/*********GPIOx������������*********/
#define Px_PUPDR_SET(x,pin,pupd)		( x->PUPDR   = (x->PUPDR   & ~((U32_t)0x3<<pin*2) | ((U32_t)pupd <<pin*2)) )

/*********GPIOx�����������*********/
#define Px_OTYPER_SET(x,pin,type)		( x->OTYPER  = (x->OTYPER  & ~((U32_t)0x1<<pin*1) | ((U32_t)type <<pin*1)) )

/*********GPIOx���ù�������*********/
#define Px_AF_SET(x,pin,af)             ( x->AFR[pin>>3] = x->AFR[pin>>3] & ~((U32_t)0xF<<((pin&0x7)*4)) | (U32_t)af<<((pin&0x7)*4) ) 

/*********GPIOx��λ����*********/
#define Px_SET(x,pin) ( x->BSRR |= (1<<pin) )

/*********GPIOx��λ����*********/
#define Px_CLR(x,pin) ( x->BSRR |= (1<<(pin+16)) )

/*********GPIOxλ��ȡ����*********/
#define Px_GET(x,pin) ( (x->IDR & (1<<pin)) >> pin )


#define IO_HIGH     1
#define IO_LOW      0
#define ON		    1
#define OFF 	    0
#define NORM        1	/*����*/
#define ABNM        0	/*������*/
#define Q_BLOCK		1	/*������ѯ*/
#define Q_UN_BLOCK  0	/*��������ѯ*/



GM_UINT8 SYS_ClockSet(GM_UINT32 plln,GM_UINT32 pllm,GM_UINT32 pllp,GM_UINT32 pllq);		//ϵͳʱ������
void SYS_ClockInit(GM_UINT32 plln,GM_UINT32 pllm,GM_UINT32 pllp,GM_UINT32 pllq); //ʱ�ӳ�ʼ��  
void SYS_SoftReset(void);      							//ϵͳ��λ
void SYS_CacheEnable(void);									//ʹ��catch
void SYS_Standby(void);         							//����ģʽ 	
void SYS_NvicSetVectorTable(GM_UINT32 NVIC_VectTab, GM_UINT32 Offset);	//����ƫ�Ƶ�ַ
void SYS_NvicPriorityGroupConfig(GM_UINT8 NVIC_Group);			//����NVIC����
void SYS_NvicInit(GM_UINT8 NVIC_PreemptionPriority,GM_UINT8 NVIC_SubPriority,GM_UINT8 NVIC_Channel,GM_UINT8 NVIC_Group);//�����ж�
void Ex_NVIC_Config(GM_UINT8 GPIOx,GM_UINT8 BITx,GM_UINT8 TRIM);				//�ⲿ�ж����ú���(ֻ��GPIOA~I)

void SYS_SystemCoreClockUpdate(void);
void SysTick_Handler(void);
//����Ϊ��ຯ��
void SYS_WfiSet(void);		//ִ��WFIָ��
void SYS_IntxDisable(void);//�ر������ж�
void SYS_IntxEnable(void);	//���������ж�
void SYS_MsrMsp(GM_UINT32 addr);	//���ö�ջ��ַ 


void SYS_DelayInit(GM_UINT8 sysclk);
void SYS_DelayUs(GM_UINT32 nus);
void SYS_DelayMs(GM_UINT16 nms);
void SYS_DelayXms(GM_UINT16 nms);


#endif











