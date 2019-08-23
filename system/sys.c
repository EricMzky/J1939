#include "system.h"  


uint32_t SystemCoreClock = 216000000; 


static GM_UINT32 g_fac_us=0;//us��ʱ������			   
static GM_UINT16 g_fac_ms=0;//ms��ʱ������,��os��,����ÿ�����ĵ�ms��


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


//����������ƫ�Ƶ�ַ
//NVIC_VectTab:��ַ
//Offset:ƫ����

void SYS_NvicSetVectorTable(GM_UINT32 NVIC_VectTab,GM_UINT32 Offset)	 
{
	SCB->VTOR=NVIC_VectTab|(Offset&(GM_UINT32)0xFFFFFE00);//����NVIC��������ƫ�ƼĴ���,VTOR��9λ����,��[8:0]������
}
//����NVIC����
//NVIC_Group:NVIC���� 0~4 �ܹ�5��
void SYS_NvicPriorityGroupConfig(GM_UINT8 NVIC_Group)
{ 
	GM_UINT32 temp,temp1;	  
	temp1=(~NVIC_Group)&0x07;//ȡ����λ
	temp1<<=8;
	temp=SCB->AIRCR;  //��ȡ��ǰ������
	temp&=0X0000F8FF; //�����ǰ����
	temp|=0X05FA0000; //д��Կ��
	temp|=temp1;	   
	SCB->AIRCR=temp;  //���÷���	    	  				   
}
//����NVIC 
//NVIC_PreemptionPriority:��ռ���ȼ�
//NVIC_SubPriority       :��Ӧ���ȼ�
//NVIC_Channel           :�жϱ��
//NVIC_Group             :�жϷ��� 0~4
//ע�����ȼ����ܳ����趨����ķ�Χ!����������벻���Ĵ���
//�黮��:
//��0:0λ��ռ���ȼ�,4λ��Ӧ���ȼ�
//��1:1λ��ռ���ȼ�,3λ��Ӧ���ȼ�
//��2:2λ��ռ���ȼ�,2λ��Ӧ���ȼ�
//��3:3λ��ռ���ȼ�,1λ��Ӧ���ȼ�
//��4:4λ��ռ���ȼ�,0λ��Ӧ���ȼ�
//NVIC_SubPriority��NVIC_PreemptionPriority��ԭ����,��ֵԽС,Խ����	   
void SYS_NvicInit(GM_UINT8 NVIC_PreemptionPriority,GM_UINT8 NVIC_SubPriority,GM_UINT8 NVIC_Channel,GM_UINT8 NVIC_Group)	 
{ 
	GM_UINT32 temp;	  
	SYS_NvicPriorityGroupConfig(NVIC_Group);//���÷���
	temp=NVIC_PreemptionPriority<<(4-NVIC_Group);	  
	temp|=NVIC_SubPriority&(0x0f>>NVIC_Group);
	temp&=0xf;								//ȡ����λ
	NVIC->ISER[NVIC_Channel/32]|=1<<NVIC_Channel%32;//ʹ���ж�λ(Ҫ����Ļ�,����ICER��ӦλΪ1����)
	NVIC->IP[NVIC_Channel]|=temp<<4;				//������Ӧ���ȼ����������ȼ�   	    	  				   
} 
//�ⲿ�ж����ú���
//ֻ���GPIOA~I;������PVD,RTC,USB_OTG,USB_HS,��̫�����ѵ�
//����:
//GPIOx:0~8,����GPIOA~I
//BITx:��Ҫʹ�ܵ�λ;
//TRIM:����ģʽ,1,������;2,�Ͻ���;3�������ƽ����
//�ú���һ��ֻ������1��IO��,���IO��,���ε���
//�ú������Զ�������Ӧ�ж�,�Լ�������   	    
void Ex_NVIC_Config(GM_UINT8 GPIOx,GM_UINT8 BITx,GM_UINT8 TRIM) 
{ 
	GM_UINT8 EXTOFFSET=(BITx%4)*4;  
	RCC->APB2ENR|=1<<14;  						//ʹ��SYSCFGʱ��  
	SYSCFG->EXTICR[BITx/4]&=~(0x000F<<EXTOFFSET);//���ԭ�����ã�����
	SYSCFG->EXTICR[BITx/4]|=GPIOx<<EXTOFFSET;	//EXTI.BITxӳ�䵽GPIOx.BITx 
	//�Զ�����
	EXTI->IMR|=1<<BITx;					//����line BITx�ϵ��ж�(���Ҫ��ֹ�жϣ��򷴲�������)
	if(TRIM&0x01)EXTI->FTSR|=1<<BITx;	//line BITx���¼��½��ش���
	if(TRIM&0x02)EXTI->RTSR|=1<<BITx;	//line BITx���¼��������ش���
} 	

//THUMBָ�֧�ֻ������
//�������·���ʵ��ִ�л��ָ��WFI  
void SYS_WfiSet(void)
{
	__ASM volatile("wfi");		  
}
//�ر������ж�(���ǲ�����fault��NMI�ж�)
void SYS_IntxDisable(void)
{
	__ASM volatile("cpsid i");
}
//���������ж�
void SYS_IntxEnable(void)
{
	__ASM volatile("cpsie i");		  
}
//����ջ����ַ
//addr:ջ����ַ
__asm void SYS_MsrMsp(GM_UINT32 addr) 
{
	MSR MSP, r0 			//set Main Stack value
	BX r14
}
//�������ģʽ	  
void SYS_Standby(void)
{ 
	SCB->SCR|=1<<2;		//ʹ��SLEEPDEEPλ (SYS->CTRL)	   
	RCC->APB1ENR|=1<<28;//ʹ�ܵ�Դʱ�� 
	PWR->CSR2|=1<<8;	//����PA0����WKUP����
	PWR->CR2|=0<<8;		//����PA0Ϊ�����ػ���
	PWR->CR1|=1<<1;		//PDDS��λ  
	PWR->CR1|=1<<0;		//LPDS��λ   	
	SYS_WfiSet();			//ִ��WFIָ��,�������ģʽ		 
}	     
//ϵͳ��λ   
void SYS_SoftReset(void)
{   
	SCB->AIRCR =0X05FA0000|(GM_UINT32)0x04;	  
} 	
//ʹ��STM32F7��L1-Cache,ͬʱ����D cache��ǿ��͸д
void SYS_CacheEnable(void)
{
    SCB_EnableICache();	//ʹ��I-Cache,������core_cm7.h���涨��
    SCB_EnableDCache();	//ʹ��D-Cache,������core_cm7.h���涨�� 
	SCB->CACR|=1<<2;	//ǿ��D-Cache͸д,�粻����͸д,ʵ��ʹ���п���������������
}
//ʱ�����ú���
//Fvco=Fs*(plln/pllm);
//Fsys=Fvco/pllp=Fs*(plln/(pllm*pllp));
//Fusb=Fvco/pllq=Fs*(plln/(pllm*pllq));

//Fvco:VCOƵ��
//Fsys:ϵͳʱ��Ƶ��
//Fusb:USB,SDMMC,RNG�ȵ�ʱ��Ƶ��
//Fs:PLL����ʱ��Ƶ��,������HSI,HSE��. 
//plln:��PLL��Ƶϵ��(PLL��Ƶ),ȡֵ��Χ:50~432.
//pllm:��PLL����ƵPLL��Ƶϵ��(PLL֮ǰ�ķ�Ƶ),ȡֵ��Χ:2~63.
//pllp:ϵͳʱ�ӵ���PLL��Ƶϵ��(PLL֮��ķ�Ƶ),ȡֵ��Χ:2,4,6,8.(������4��ֵ!)
//pllq:USB/SDMMC/������������ȵ���PLL��Ƶϵ��(PLL֮��ķ�Ƶ),ȡֵ��Χ:2~15.

//�ⲿ����Ϊ25M��ʱ��,�Ƽ�ֵ:plln=432,pllm=25,pllp=2,pllq=9.
//�õ�:Fvco=25*(432/25)=432Mhz
//     Fsys=432/2=216Mhz
//     Fusb=432/9=48Mhz
//����ֵ:0,�ɹ�;1,ʧ�ܡ�
GM_UINT8 SYS_ClockSet(GM_UINT32 plln,GM_UINT32 pllm,GM_UINT32 pllp,GM_UINT32 pllq)
{ 
	GM_UINT16 retry=0;
	GM_UINT8 status=0;
	RCC->CR|=1<<16;				//HSE ���� 
	while(((RCC->CR&(1<<17))==0)&&(retry<0X1FFF))retry++;//�ȴ�HSE RDY
	if(retry==0X1FFF)status=1;	//HSE�޷�����
	else   
	{
		RCC->APB1ENR|=1<<28;	//��Դ�ӿ�ʱ��ʹ��
		PWR->CR1|=3<<14; 		//������ģʽ,ʱ�ӿɵ�180Mhz
		PWR->CR1|=1<<16; 		//ʹ�ܹ�����,Ƶ�ʿɵ�216Mhz
		PWR->CR1|=1<<17; 		//ʹ�ܹ������л�
		RCC->CFGR|=(0<<4)|(5<<10)|(4<<13);//HCLK ����Ƶ;APB1 4��Ƶ;APB2 2��Ƶ. 
		RCC->CR&=~(1<<24);		//�ر���PLL
		RCC->PLLCFGR=pllm|(plln<<6)|(((pllp>>1)-1)<<16)|(pllq<<24)|(1<<22);//������PLL,PLLʱ��Դ����HSE
		RCC->CR|=1<<24;			//����PLL
		while((RCC->CR&(1<<25))==0);//�ȴ�PLL׼���� 
		FLASH->ACR|=1<<8;		//ָ��Ԥȡʹ��.
		FLASH->ACR|=1<<9;		//ʹ��ART Accelerator 
		FLASH->ACR|=7<<0;		//8��CPU�ȴ�����. 
		RCC->CFGR&=~(3<<0);		//����
		RCC->CFGR|=2<<0;		//ѡ����PLL��Ϊϵͳʱ��	 
		while((RCC->CFGR&(3<<2))!=(2<<2));//�ȴ���PLL��Ϊϵͳʱ�ӳɹ�. 
	} 
	return status;
}  

//ϵͳʱ�ӳ�ʼ������
//plln:��PLL��Ƶϵ��(PLL��Ƶ),ȡֵ��Χ:50~432.
//pllm:��PLL����ƵPLL��Ƶϵ��(PLL֮ǰ�ķ�Ƶ),ȡֵ��Χ:2~63.
//pllp:ϵͳʱ�ӵ���PLL��Ƶϵ��(PLL֮��ķ�Ƶ),ȡֵ��Χ:2,4,6,8.(������4��ֵ!)
//pllq:USB/SDMMC/������������ȵ���PLL��Ƶϵ��(PLL֮��ķ�Ƶ),ȡֵ��Χ:2~15.
void SYS_ClockInit(GM_UINT32 plln,GM_UINT32 pllm,GM_UINT32 pllp,GM_UINT32 pllq)
{  
	RCC->CR|=0x00000001;		//����HISON,�����ڲ�����RC��
	RCC->CFGR=0x00000000;		//CFGR���� 
	RCC->CR&=0xFEF6FFFF;		//HSEON,CSSON,PLLON���� 
	RCC->PLLCFGR=0x24003010;	//PLLCFGR�ָ���λֵ 
	RCC->CR&=~(1<<18);			//HSEBYP����,�ⲿ������·
	RCC->CIR=0x00000000;		//��ֹRCCʱ���ж� 
	SYS_CacheEnable();				//ʹ��L1 Cache
	SYS_ClockSet(plln,pllm,pllp,pllq);//����ʱ�� 
	//����������				  
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
* Description:	��ʼ����ʱ����
				��ʹ��FreeRTOS��ʱ��,�˺������ʼ��FreeRTOS��ʱ�ӽ���
				SYSTICK��ʱ�ӹ̶�ΪAHBʱ��
				SYSCLK:ϵͳʱ��Ƶ��
* Parameter:	GM_UINT8 sysclk
* Return:       void
* History:
*a   16-10��-2017 11:15:52   dengminjun     Created
**********************************************************/
void SYS_DelayInit(GM_UINT8 sysclk)
{
	GM_UINT32 reload;//SysTickƵ��ΪHCLK
	SysTick->CTRL &= ~( 1 << 2 );					//SYSTICKʹ���ⲿʱ��Դ	 
	g_fac_us = sysclk / 8;//�����Ƿ�ʹ��OS,fac_us����Ҫʹ��
	reload = sysclk / 8;//ÿ���ӵļ���������λΪK
	reload *= 1000000 / configTICK_RATE_HZ;
	//����configTICK_RATE_HZ�趨���ʱ��
	//reloadΪ24λ�Ĵ���,���ֵ:16777216,//��180M��,Լ��0.745s����
	g_fac_ms = 1000 / configTICK_RATE_HZ;//����OS������ʱ�����ٵ�λ
	SysTick->CTRL |= 1 << 1;   					//����SYSTICK�ж�
	SysTick->LOAD = reload; //ÿ1/configTICK_RATE_HZ��һ��
	SysTick->CTRL |= 1 << 0; //����SYSTIC
}							


/*********************************************************
* Function: 	void SYS_DelayUs(GM_UINT32 nus)
* Description:	��ʱnus��������
				nus:Ҫ��ʱ��us��.	
				nus:0~204522252(���ֵ��2^32/fac_us@fac_us=21)	
* Parameter:	GM_UINT32 nus
* Return:       void
* History:
*a   16-10��-2017 11:15:52   dengminjun     Created
**********************************************************/    								   
void SYS_DelayUs(GM_UINT32 nus)
{		
	GM_UINT32 ticks;
	GM_UINT32 told,tnow,tcnt=0;
	GM_UINT32 reload = SysTick->LOAD;		//LOAD��ֵ	    	 
	ticks = nus * g_fac_us; 				//��Ҫ�Ľ����� 
	told = SysTick->VAL;        			//�ս���ʱ�ļ�����ֵ
	while( 1 )
	{
		tnow = SysTick->VAL;	
		if( tnow != told )
		{
			if( tnow < told )
			{
				tcnt += told-tnow ;//����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
			}
			else 
			{
				tcnt += reload - tnow + told;
			}	    
			told = tnow;
			if( tcnt >= ticks )
			{
				break;			//ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�
			}
		}  
	}								    
} 
/*********************************************************
* Function: 	void SYS_DelayMs(GM_UINT16 nms)
* Description:	��ʱnms
				nms:Ҫ��ʱ��ms��
				nms:0~65535	
* Parameter:	GM_UINT32 nus
* Return:       void
* History:
*a   16-10��-2017 11:15:52   dengminjun     Created
**********************************************************/   
void SYS_DelayMs(GM_UINT16 nms)
{	
	if( xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED )//���OS�Ѿ�������,���Ҳ������ж�����(�ж����治���������)	    
	{		 
		if( nms >= g_fac_ms)						//��ʱ��ʱ�����OS������ʱ������ 
		{ 
   			vTaskDelay( nms / g_fac_ms );	//OS��ʱ
		}
		nms %= g_fac_ms;						//OS�Ѿ��޷��ṩ��ôС����ʱ��,������ͨ��ʽ��ʱ    
	}
	SYS_DelayUs( (GM_UINT32) ( nms * 1000) );				//��ͨ��ʽ��ʱ
}

/*********************************************************
* Function: 	void SYS_DelayXms(GM_UINT16 nms)
* Description:	��ʱnms
				nms:Ҫ��ʱ��ms��
				nms:0~65535	
* Parameter:	GM_UINT32 nus
* Return:       void
* History:
*a   16-10��-2017 11:15:52   dengminjun     Created
**********************************************************/  
void SYS_DelayXms(GM_UINT16 nms)
{	 		  	  
	GM_UINT32 i;
	for( i = 0 ; i < nms ; i++ )
	{
		SYS_DelayUs(1000);	  		//��ռ�����	 
	} 	    
} 























