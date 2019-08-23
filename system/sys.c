#include "system.h"  


uint32_t SystemCoreClock = 216000000; 


static GM_UINT32 g_fac_us=0;//us延时倍乘数			   
static GM_UINT16 g_fac_ms=0;//ms延时倍乘数,在os下,代表每个节拍的ms数


extern void xPortSysTickHandler(void);

void SYS_SystemCoreClockUpdate(void)
{
  GM_UINT32 tmp = 0, pllvco = 0, pllp = 2, pllsource = 0, pllm = 2;
  
  /* Get SYSCLK source -------------------------------------------------------*/
  tmp = RCC->CFGR & RCC_CFGR_SWS;

  switch (tmp)
  {
    case 0x00:  /* HSI used as system clock source */
      SystemCoreClock = 16000000;
      break;
    case 0x04:  /* HSE used as system clock source */
      SystemCoreClock = 25000000;
      break;
    case 0x08:  /* PLL used as system clock source */

      /* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLL_M) * PLL_N
         SYSCLK = PLL_VCO / PLL_P
         */    
      pllsource = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) >> 22;
      pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;
      
      if (pllsource != 0)
      {
        /* HSE used as PLL clock source */
        pllvco = (25000000 / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
      }
      else
      {
        /* HSI used as PLL clock source */
        pllvco = (25000000 / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);      
      }

      pllp = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >>16) + 1 ) *2;
      SystemCoreClock = pllvco/pllp;
      break;
    default:
      SystemCoreClock = 16000000;
      break;
  }
  /* Compute HCLK frequency --------------------------------------------------*/
  /* Get HCLK prescaler */
  tmp = 0;
  /* HCLK frequency */
  SystemCoreClock >>= tmp;
}


//设置向量表偏移地址
//NVIC_VectTab:基址
//Offset:偏移量

void SYS_NvicSetVectorTable(GM_UINT32 NVIC_VectTab,GM_UINT32 Offset)	 
{
	SCB->VTOR=NVIC_VectTab|(Offset&(GM_UINT32)0xFFFFFE00);//设置NVIC的向量表偏移寄存器,VTOR低9位保留,即[8:0]保留。
}
//设置NVIC分组
//NVIC_Group:NVIC分组 0~4 总共5组
void SYS_NvicPriorityGroupConfig(GM_UINT8 NVIC_Group)
{ 
	GM_UINT32 temp,temp1;	  
	temp1=(~NVIC_Group)&0x07;//取后三位
	temp1<<=8;
	temp=SCB->AIRCR;  //读取先前的设置
	temp&=0X0000F8FF; //清空先前分组
	temp|=0X05FA0000; //写入钥匙
	temp|=temp1;	   
	SCB->AIRCR=temp;  //设置分组	    	  				   
}
//设置NVIC 
//NVIC_PreemptionPriority:抢占优先级
//NVIC_SubPriority       :响应优先级
//NVIC_Channel           :中断编号
//NVIC_Group             :中断分组 0~4
//注意优先级不能超过设定的组的范围!否则会有意想不到的错误
//组划分:
//组0:0位抢占优先级,4位响应优先级
//组1:1位抢占优先级,3位响应优先级
//组2:2位抢占优先级,2位响应优先级
//组3:3位抢占优先级,1位响应优先级
//组4:4位抢占优先级,0位响应优先级
//NVIC_SubPriority和NVIC_PreemptionPriority的原则是,数值越小,越优先	   
void SYS_NvicInit(GM_UINT8 NVIC_PreemptionPriority,GM_UINT8 NVIC_SubPriority,GM_UINT8 NVIC_Channel,GM_UINT8 NVIC_Group)	 
{ 
	GM_UINT32 temp;	  
	SYS_NvicPriorityGroupConfig(NVIC_Group);//设置分组
	temp=NVIC_PreemptionPriority<<(4-NVIC_Group);	  
	temp|=NVIC_SubPriority&(0x0f>>NVIC_Group);
	temp&=0xf;								//取低四位
	NVIC->ISER[NVIC_Channel/32]|=1<<NVIC_Channel%32;//使能中断位(要清除的话,设置ICER对应位为1即可)
	NVIC->IP[NVIC_Channel]|=temp<<4;				//设置响应优先级和抢断优先级   	    	  				   
} 
//外部中断配置函数
//只针对GPIOA~I;不包括PVD,RTC,USB_OTG,USB_HS,以太网唤醒等
//参数:
//GPIOx:0~8,代表GPIOA~I
//BITx:需要使能的位;
//TRIM:触发模式,1,下升沿;2,上降沿;3，任意电平触发
//该函数一次只能配置1个IO口,多个IO口,需多次调用
//该函数会自动开启对应中断,以及屏蔽线   	    
void Ex_NVIC_Config(GM_UINT8 GPIOx,GM_UINT8 BITx,GM_UINT8 TRIM) 
{ 
	GM_UINT8 EXTOFFSET=(BITx%4)*4;  
	RCC->APB2ENR|=1<<14;  						//使能SYSCFG时钟  
	SYSCFG->EXTICR[BITx/4]&=~(0x000F<<EXTOFFSET);//清除原来设置！！！
	SYSCFG->EXTICR[BITx/4]|=GPIOx<<EXTOFFSET;	//EXTI.BITx映射到GPIOx.BITx 
	//自动设置
	EXTI->IMR|=1<<BITx;					//开启line BITx上的中断(如果要禁止中断，则反操作即可)
	if(TRIM&0x01)EXTI->FTSR|=1<<BITx;	//line BITx上事件下降沿触发
	if(TRIM&0x02)EXTI->RTSR|=1<<BITx;	//line BITx上事件上升降沿触发
} 	

//THUMB指令不支持汇编内联
//采用如下方法实现执行汇编指令WFI  
void SYS_WfiSet(void)
{
	__ASM volatile("wfi");		  
}
//关闭所有中断(但是不包括fault和NMI中断)
void SYS_IntxDisable(void)
{
	__ASM volatile("cpsid i");
}
//开启所有中断
void SYS_IntxEnable(void)
{
	__ASM volatile("cpsie i");		  
}
//设置栈顶地址
//addr:栈顶地址
__asm void SYS_MsrMsp(GM_UINT32 addr) 
{
	MSR MSP, r0 			//set Main Stack value
	BX r14
}
//进入待机模式	  
void SYS_Standby(void)
{ 
	SCB->SCR|=1<<2;		//使能SLEEPDEEP位 (SYS->CTRL)	   
	RCC->APB1ENR|=1<<28;//使能电源时钟 
	PWR->CSR2|=1<<8;	//设置PA0用于WKUP唤醒
	PWR->CR2|=0<<8;		//设置PA0为上升沿唤醒
	PWR->CR1|=1<<1;		//PDDS置位  
	PWR->CR1|=1<<0;		//LPDS置位   	
	SYS_WfiSet();			//执行WFI指令,进入待机模式		 
}	     
//系统软复位   
void SYS_SoftReset(void)
{   
	SCB->AIRCR =0X05FA0000|(GM_UINT32)0x04;	  
} 	
//使能STM32F7的L1-Cache,同时开启D cache的强制透写
void SYS_CacheEnable(void)
{
    SCB_EnableICache();	//使能I-Cache,函数在core_cm7.h里面定义
    SCB_EnableDCache();	//使能D-Cache,函数在core_cm7.h里面定义 
	SCB->CACR|=1<<2;	//强制D-Cache透写,如不开启透写,实际使用中可能遇到各种问题
}
//时钟设置函数
//Fvco=Fs*(plln/pllm);
//Fsys=Fvco/pllp=Fs*(plln/(pllm*pllp));
//Fusb=Fvco/pllq=Fs*(plln/(pllm*pllq));

//Fvco:VCO频率
//Fsys:系统时钟频率
//Fusb:USB,SDMMC,RNG等的时钟频率
//Fs:PLL输入时钟频率,可以是HSI,HSE等. 
//plln:主PLL倍频系数(PLL倍频),取值范围:50~432.
//pllm:主PLL和音频PLL分频系数(PLL之前的分频),取值范围:2~63.
//pllp:系统时钟的主PLL分频系数(PLL之后的分频),取值范围:2,4,6,8.(仅限这4个值!)
//pllq:USB/SDMMC/随机数产生器等的主PLL分频系数(PLL之后的分频),取值范围:2~15.

//外部晶振为25M的时候,推荐值:plln=432,pllm=25,pllp=2,pllq=9.
//得到:Fvco=25*(432/25)=432Mhz
//     Fsys=432/2=216Mhz
//     Fusb=432/9=48Mhz
//返回值:0,成功;1,失败。
GM_UINT8 SYS_ClockSet(GM_UINT32 plln,GM_UINT32 pllm,GM_UINT32 pllp,GM_UINT32 pllq)
{ 
	GM_UINT16 retry=0;
	GM_UINT8 status=0;
	RCC->CR|=1<<16;				//HSE 开启 
	while(((RCC->CR&(1<<17))==0)&&(retry<0X1FFF))retry++;//等待HSE RDY
	if(retry==0X1FFF)status=1;	//HSE无法就绪
	else   
	{
		RCC->APB1ENR|=1<<28;	//电源接口时钟使能
		PWR->CR1|=3<<14; 		//高性能模式,时钟可到180Mhz
		PWR->CR1|=1<<16; 		//使能过驱动,频率可到216Mhz
		PWR->CR1|=1<<17; 		//使能过驱动切换
		RCC->CFGR|=(0<<4)|(5<<10)|(4<<13);//HCLK 不分频;APB1 4分频;APB2 2分频. 
		RCC->CR&=~(1<<24);		//关闭主PLL
		RCC->PLLCFGR=pllm|(plln<<6)|(((pllp>>1)-1)<<16)|(pllq<<24)|(1<<22);//配置主PLL,PLL时钟源来自HSE
		RCC->CR|=1<<24;			//打开主PLL
		while((RCC->CR&(1<<25))==0);//等待PLL准备好 
		FLASH->ACR|=1<<8;		//指令预取使能.
		FLASH->ACR|=1<<9;		//使能ART Accelerator 
		FLASH->ACR|=7<<0;		//8个CPU等待周期. 
		RCC->CFGR&=~(3<<0);		//清零
		RCC->CFGR|=2<<0;		//选择主PLL作为系统时钟	 
		while((RCC->CFGR&(3<<2))!=(2<<2));//等待主PLL作为系统时钟成功. 
	} 
	return status;
}  

//系统时钟初始化函数
//plln:主PLL倍频系数(PLL倍频),取值范围:50~432.
//pllm:主PLL和音频PLL分频系数(PLL之前的分频),取值范围:2~63.
//pllp:系统时钟的主PLL分频系数(PLL之后的分频),取值范围:2,4,6,8.(仅限这4个值!)
//pllq:USB/SDMMC/随机数产生器等的主PLL分频系数(PLL之后的分频),取值范围:2~15.
void SYS_ClockInit(GM_UINT32 plln,GM_UINT32 pllm,GM_UINT32 pllp,GM_UINT32 pllq)
{  
	RCC->CR|=0x00000001;		//设置HISON,开启内部高速RC振荡
	RCC->CFGR=0x00000000;		//CFGR清零 
	RCC->CR&=0xFEF6FFFF;		//HSEON,CSSON,PLLON清零 
	RCC->PLLCFGR=0x24003010;	//PLLCFGR恢复复位值 
	RCC->CR&=~(1<<18);			//HSEBYP清零,外部晶振不旁路
	RCC->CIR=0x00000000;		//禁止RCC时钟中断 
	SYS_CacheEnable();				//使能L1 Cache
	SYS_ClockSet(plln,pllm,pllp,pllq);//设置时钟 
	//配置向量表				  
#ifdef  VECT_TAB_RAM
	SYS_NvicSetVectorTable(SRAM1_BASE,0x0);
#else   
	SYS_NvicSetVectorTable(FLASH_BASE,0x20000);
#endif 
}		    


/*********************************************************
* Function: 	void SYS_SysTickHandler(void)
* Description:	FreeRTOS????,????
* Parameter:	void
* Return:       void
* History:
*a   16-10?-2017 11:15:52   dengminjun     Created
**********************************************************/	
void SysTick_Handler(void)
{   
	if( xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED )
	{
		xPortSysTickHandler();
	}
}

/*********************************************************
* Function: 	void SYS_DelayInit(GM_UINT8 sysclk)
* Description:	初始化延时函数
				当使用FreeRTOS的时候,此函数会初始化FreeRTOS的时钟节拍
				SYSTICK的时钟固定为AHB时钟
				SYSCLK:系统时钟频率
* Parameter:	GM_UINT8 sysclk
* Return:       void
* History:
*a   16-10月-2017 11:15:52   dengminjun     Created
**********************************************************/
void SYS_DelayInit(GM_UINT8 sysclk)
{
	GM_UINT32 reload;//SysTick频率为HCLK
	SysTick->CTRL &= ~( 1 << 2 );					//SYSTICK使用外部时钟源	 
	g_fac_us = sysclk / 8;//不论是否使用OS,fac_us都需要使用
	reload = sysclk / 8;//每秒钟的计数次数单位为K
	reload *= 1000000 / configTICK_RATE_HZ;
	//根据configTICK_RATE_HZ设定溢出时间
	//reload为24位寄存器,最大值:16777216,//在180M下,约合0.745s左右
	g_fac_ms = 1000 / configTICK_RATE_HZ;//代表OS可以延时的最少单位
	SysTick->CTRL |= 1 << 1;   					//开启SYSTICK中断
	SysTick->LOAD = reload; //每1/configTICK_RATE_HZ断一次
	SysTick->CTRL |= 1 << 0; //开启SYSTIC
}							


/*********************************************************
* Function: 	void SYS_DelayUs(GM_UINT32 nus)
* Description:	延时nus，不调度
				nus:要延时的us数.	
				nus:0~204522252(最大值即2^32/fac_us@fac_us=21)	
* Parameter:	GM_UINT32 nus
* Return:       void
* History:
*a   16-10月-2017 11:15:52   dengminjun     Created
**********************************************************/    								   
void SYS_DelayUs(GM_UINT32 nus)
{		
	GM_UINT32 ticks;
	GM_UINT32 told,tnow,tcnt=0;
	GM_UINT32 reload = SysTick->LOAD;		//LOAD的值	    	 
	ticks = nus * g_fac_us; 				//需要的节拍数 
	told = SysTick->VAL;        			//刚进入时的计数器值
	while( 1 )
	{
		tnow = SysTick->VAL;	
		if( tnow != told )
		{
			if( tnow < told )
			{
				tcnt += told-tnow ;//这里注意一下SYSTICK是一个递减的计数器就可以了.
			}
			else 
			{
				tcnt += reload - tnow + told;
			}	    
			told = tnow;
			if( tcnt >= ticks )
			{
				break;			//时间超过/等于要延迟的时间,则退出
			}
		}  
	}								    
} 
/*********************************************************
* Function: 	void SYS_DelayMs(GM_UINT16 nms)
* Description:	延时nms
				nms:要延时的ms数
				nms:0~65535	
* Parameter:	GM_UINT32 nus
* Return:       void
* History:
*a   16-10月-2017 11:15:52   dengminjun     Created
**********************************************************/   
void SYS_DelayMs(GM_UINT16 nms)
{	
	if( xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED )//如果OS已经在跑了,并且不是在中断里面(中断里面不能任务调度)	    
	{		 
		if( nms >= g_fac_ms)						//延时的时间大于OS的最少时间周期 
		{ 
   			vTaskDelay( nms / g_fac_ms );	//OS延时
		}
		nms %= g_fac_ms;						//OS已经无法提供这么小的延时了,采用普通方式延时    
	}
	SYS_DelayUs( (GM_UINT32) ( nms * 1000) );				//普通方式延时
}

/*********************************************************
* Function: 	void SYS_DelayXms(GM_UINT16 nms)
* Description:	延时nms
				nms:要延时的ms数
				nms:0~65535	
* Parameter:	GM_UINT32 nus
* Return:       void
* History:
*a   16-10月-2017 11:15:52   dengminjun     Created
**********************************************************/  
void SYS_DelayXms(GM_UINT16 nms)
{	 		  	  
	GM_UINT32 i;
	for( i = 0 ; i < nms ; i++ )
	{
		SYS_DelayUs(1000);	  		//清空计数器	 
	} 	    
} 























