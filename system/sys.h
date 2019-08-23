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


/********* 管脚号 *********/
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

/********* GPIO配置选项 *********/
#define CFG_MODE_IN    	0		/*  普通输入模式	*/
#define CFG_MODE_OUT	1		/*  普通输出模式	*/
#define CFG_MODE_AF		2		/*  复用功能模式	*/
#define CFG_MODE_AIN	3		/*  模拟输入模式	*/
#define CFG_SPEED_2M	0		/*  最大速度2Mhz(低速)	*/
#define CFG_SPEED_25M	1		/*  最大速度25Mhz(中速)	*/
#define CFG_SPEED_50M	2		/*  最大速度50Mhz(快速)	*/
#define CFG_SPEED_100M	3		/*  最大速度100Mhz(高速)	*/
#define CFG_PUPD_NONE	0		/*  无上下拉	*/
#define CFG_PUPD_PU		1		/*  上拉 */
#define CFG_PUPD_PD		2		/*  下拉 */
#define CFG_PUPD_RES	3		/*  保留 */
#define CFG_OTYPE_PP	0		/*  推挽输出	*/
#define CFG_OTYPE_OD	1		/*  开漏输出	*/

/********* 复用功能选项 *********/
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


/*********GPIOx模式配置*********/
#define Px_MODER_SET(x,pin,mode)		( x->MODER   = (x->MODER   & ~((U32_t)0x3<<pin*2) | ((U32_t)mode <<pin*2)) )

/*********GPIOx输出速度配置*********/
#define Px_OSPEEDR_SET(x,pin,speed)	    ( x->OSPEEDR = (x->OSPEEDR & ~((U32_t)0x3<<pin*2) | ((U32_t)speed <<pin*2)) )

/*********GPIOx上拉下拉配置*********/
#define Px_PUPDR_SET(x,pin,pupd)		( x->PUPDR   = (x->PUPDR   & ~((U32_t)0x3<<pin*2) | ((U32_t)pupd <<pin*2)) )

/*********GPIOx输出类型配置*********/
#define Px_OTYPER_SET(x,pin,type)		( x->OTYPER  = (x->OTYPER  & ~((U32_t)0x1<<pin*1) | ((U32_t)type <<pin*1)) )

/*********GPIOx复用功能配置*********/
#define Px_AF_SET(x,pin,af)             ( x->AFR[pin>>3] = x->AFR[pin>>3] & ~((U32_t)0xF<<((pin&0x7)*4)) | (U32_t)af<<((pin&0x7)*4) ) 

/*********GPIOx置位操作*********/
#define Px_SET(x,pin) ( x->BSRR |= (1<<pin) )

/*********GPIOx复位操作*********/
#define Px_CLR(x,pin) ( x->BSRR |= (1<<(pin+16)) )

/*********GPIOx位读取操作*********/
#define Px_GET(x,pin) ( (x->IDR & (1<<pin)) >> pin )


#define IO_HIGH     1
#define IO_LOW      0
#define ON		    1
#define OFF 	    0
#define NORM        1	/*正常*/
#define ABNM        0	/*非正常*/
#define Q_BLOCK		1	/*阻塞查询*/
#define Q_UN_BLOCK  0	/*非阻塞查询*/



GM_UINT8 SYS_ClockSet(GM_UINT32 plln,GM_UINT32 pllm,GM_UINT32 pllp,GM_UINT32 pllq);		//系统时钟设置
void SYS_ClockInit(GM_UINT32 plln,GM_UINT32 pllm,GM_UINT32 pllp,GM_UINT32 pllq); //时钟初始化  
void SYS_SoftReset(void);      							//系统软复位
void SYS_CacheEnable(void);									//使能catch
void SYS_Standby(void);         							//待机模式 	
void SYS_NvicSetVectorTable(GM_UINT32 NVIC_VectTab, GM_UINT32 Offset);	//设置偏移地址
void SYS_NvicPriorityGroupConfig(GM_UINT8 NVIC_Group);			//设置NVIC分组
void SYS_NvicInit(GM_UINT8 NVIC_PreemptionPriority,GM_UINT8 NVIC_SubPriority,GM_UINT8 NVIC_Channel,GM_UINT8 NVIC_Group);//设置中断
void Ex_NVIC_Config(GM_UINT8 GPIOx,GM_UINT8 BITx,GM_UINT8 TRIM);				//外部中断配置函数(只对GPIOA~I)

void SYS_SystemCoreClockUpdate(void);
void SysTick_Handler(void);
//以下为汇编函数
void SYS_WfiSet(void);		//执行WFI指令
void SYS_IntxDisable(void);//关闭所有中断
void SYS_IntxEnable(void);	//开启所有中断
void SYS_MsrMsp(GM_UINT32 addr);	//设置堆栈地址 


void SYS_DelayInit(GM_UINT8 sysclk);
void SYS_DelayUs(GM_UINT32 nus);
void SYS_DelayMs(GM_UINT16 nms);
void SYS_DelayXms(GM_UINT16 nms);


#endif











