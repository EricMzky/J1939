/********************************************************************
*XXXXXX System Development XXXXXX.XXXXXXXX Computer Software
*Copyright (c) 2019-2029, XXXXX LTD., All rights reserved.
*File Name:   DeviceCAN.c
*File Description: Implementation of  DeviceCAN.c
*Modification History:
*a	 08/22/2019 15:43:08  WangLin	Created
********************************************************************/

#include "DeviceCAN.h"




static INT32 g_CanSocket[ECU_DEV_CAN_NUM] = {-1,-1};
static BOOL g_IsCanOpen[ECU_DEV_CAN_NUM] = {FALSE,FALSE};
static const UINT8  g_CanDevName[ECU_DEV_CAN_NUM][20] = {"can0","can1"};



/*********************************************************
* Function: 	ECU_CanDeviceInit
* Description:  ��ʼ��CAN�豸
* Parameter:	void
* Return:       BOOL
			If process ok, returns TRUE, otherwise,returns FALSE
* History:
*a	 08/22/2019 15:43:08  WangLin	Created
********************************************************************/
BOOL  ECU_Can1Init(void)
{
	RCC->AHB1ENR|=1<<0;				/*PORTA CLOCK*/
	
	GPIOA->MODER &= ~(0xF<<22);		/*PA11 PA12*/
	GPIOA->MODER |= 0xA<<22;

	GPIOA->OSPEEDR &= ~(0xF<<22);	/*50M*/
	GPIOA->OSPEEDR |= 0xA<<22;
	
	GPIOA->OTYPER &= ~(0x3<<11);	/*�������*/
	GPIOA->OTYPER |= 0<<11;

	GPIOA->PUPDR &=	~(0xF<<22);		/*����*/
	GPIOA->PUPDR |=	0x5<<22;
	
	GPIOA->AFR[1] &= ~(0x99<<12);	/*AF9*/
	GPIOA->AFR[1] |= 0x99<<12;

	RCC->APB1ENR |= 1<<25;			/*CAN1 CLOCK*/

	CAN1->MCR = 0x0000;				/*�Ĵ�������*/
	CAN1->MCR |= 1<<0;				/*����CAN�����ʼ��ģʽ*/
	
	while((CAN1->MSR & 1<<0) == 0);

	CAN1->MCR |= 0<<7;				/*��ʱ�䴥��ͨ��ģʽ*/
	CAN1->MCR |= 1<<6;				/*Ӳ���Զ����߹���*/
	CAN1->MCR |= 1<<5;				/*˯��ģʽӲ������*/
	CAN1->MCR |= 0<<4;				/*�������Զ��ش�*/
	CAN1->MCR |= 0<<3;				/*�������ʱԭ���ı��±��ĸ���*/
	CAN1->MCR |= 1<<2;				/*�������ȼ��ɷ��������˳����ִ��*/

	CAN1->BTR = 0x00000000;			/*�Ĵ�������*/
	CAN1->BTR |= 0<<31;				/*Norma:0   Silent:1*/
	CAN1->BTR |= 0<<30;				/*Ordinary:0 Loopback:1 */
	CAN1->BTR |= 0x1<<24;			/*sjw trjw=tq*(sjw+1)  */
	CAN1->BTR |= 0x6<<20;			/*ts2 tbs2=tq*(ts2+1) */
	CAN1->BTR |= 0x9<<16;			/*ts1 tbs1=tq*(ts1+1) */
	CAN1->BTR |= 0x2<<0;			/*brp tq=(brp+1)*tpclk */

	CAN1->MCR &= ~(1<<0);			/*ʹ��CAN1*/

	while((CAN1->MSR & 1<<0) == 1);

	CAN1->FMR |= 1<<0;				/*������������ʼ��״̬*/
	CAN1->FA1R = 0x0;				/*��ֹ��������[0][1][2][3]*/
	CAN1->FS1R = 0xF;				/*���ù�������[0][1][2][3]λ��Ϊ1��32λ�Ĵ���*/
	CAN1->FM1R = 0x0;				/*���ù�������[0][1][2][3]�����ڱ�ʶ������λģʽ*/
	CAN1->FFA1R = 0xC;				/*���ù�������[0][1]������FIFO0����������[2][3]������FIFO1*/
	
#if 0
	CAN1->sFilterRegister[0].FR1 = CAN_FILT_IDE_DST_ID(SYS_COMPONENT_ID_ECU_A);	/*������0*/
	CAN1->sFilterRegister[0].FR2 = CAN_FILT_MASK_DST_ID;	/*������0*/
	CAN1->sFilterRegister[1].FR1 = CAN_FILT_IDE_DST_ID(SYS_COMPONENT_ID_GROUP);  	/*������1*/
	CAN1->sFilterRegister[1].FR2 = CAN_FILT_MASK_DST_ID;	/*������1*/
	CAN1->sFilterRegister[2].FR1 =  CAN_FILT_IDE_DST_ID(SYS_COMPONENT_ID_ECU_MULTICAST);/*������2*/
	CAN1->sFilterRegister[2].FR2 = CAN_FILT_MASK_DST_ID;	/*������2*/	
	CAN1->sFilterRegister[3].FR1 = 0x0;	/*������3*/
	CAN1->sFilterRegister[3].FR2 = 0x0;	/*������3*/
#endif	

	CAN1->FA1R = 0xF;				/*�����������[0][1][2][3]*/
	CAN1->FMR &= ~1;				/*�������������������״̬*/

	CAN1->IER |= 0xE;		    	/*FIFO0��Ϣ�Һ�������ж�����*/	    
	
	MY_NVIC_Init(10,0,CAN1_RX0_IRQn,4);
	return TRUE;
}



/*********************************************************
* Function: 	ECU_ReadCan1L2FiFoCache
* Description:  CAN���ݶ�������FIFO������
* Parameter:	*pFrame
* Return:       BOOL
			If process ok, returns TRUE, otherwise,returns FALSE
* History:
*a	 08/22/2019 15:43:08  WangLin	Created
********************************************************************/
BOOL ECU_ReadCan1L2FiFoCache(UINT8 *pFrame)
{
	UINT16 size = 16;
	BOOL ret = FALSE;
	CanFiFoCache CanFiFoL2Cache;
	UINT32 id=0,bufh=0,bufl=0;
	UINT8 i,ide,rtr,dlc,buf[8];
	CanFrame_t *can=NULL;
	
	/*ȡ��������
	ret = GM_MQ_Fixed_CanRead(&g_LocalCanMQ,(UINT8*)&CanFiFoL2Cache,&size);*/
	
	if(TRUE != ret)
	{
		return FALSE;
	}

	can = (CanFrame_t*)pFrame;

	/*Ԥ����������*/
	id =  CanFiFoL2Cache.can_reg_rir >> 3;
	rtr = CanFiFoL2Cache.can_reg_rir & 0x2 >> 1;
	ide = CanFiFoL2Cache.can_reg_rir & 0x4 >> 2;
	dlc = CanFiFoL2Cache.can_reg_rdtr & 0xF;
	bufl = CanFiFoL2Cache.can_reg_rdlr;
	bufh = CanFiFoL2Cache.can_reg_rdhr;

	/*��������*/
	can->MsgHead.DataLen = dlc;
	can->MsgHead.EndFlag = (id & 0x1) >> 0;
	can->MsgHead.SubIndex = (id & 0xFE) >> 1;
	can->MsgHead.MsgIndex = (id & 0xFF00) >> 8;
	can->MsgHead.SrcID = (id & 0xF0000) >> 16;
	can->MsgHead.DstID = (id & 0xF00000) >> 20; 
	can->MsgHead.MsgType = (id & 0x1F000000) >> 24;
	can->MsgHead.ERF = 1;
	can->MsgHead.RTR = rtr;
	can->MsgHead.FF = ide;
	
	buf[0] = bufl >> 0;
	buf[1] = bufl >> 8;
	buf[2] = bufl >> 16;
	buf[3] = bufl >> 24;
	buf[4] = bufh >> 0;
	buf[5] = bufh >> 8;
	buf[6] = bufh >> 16;
	buf[7] = bufh >> 24;

	for(i=0;i<dlc;i++)
	{
		can->MsgData[i] = buf[i];
	}

	return TRUE;		
}


/*********************************************************
* Function: 	ECU_WriteCan1L2FiFoCache
* Description:  CAN����д�����FIFO������
* Parameter:	void
* Return:       NO RETURN
* History:
*a	 08/22/2019 15:43:08  WangLin	Created
********************************************************************/
void ECU_WriteCan1L2FiFoCache(void)
{
	CanFiFoCache CanFiFoL2Cache;
	BOOL ret = FALSE;

	/*�ж�FIFO��*/
	if(CAN1->RF0R & (1<<3))
  	{   
    	CAN1->RF0R &= ~(1<<3);  
  	}

	/*�ж�FIFO���*/
	if(CAN1->RF0R & (1<<4))
  	{  
    	CAN1->RF0R &= ~(1<<4);  
  	}

	/*���������������ȫ��ȡ��*/
	while (CAN1->RF0R & 0x3)
	{
		CanFiFoL2Cache.can_reg_rir  = CAN1->sFIFOMailBox[0].RIR;
		CanFiFoL2Cache.can_reg_rdtr = CAN1->sFIFOMailBox[0].RDTR;
		CanFiFoL2Cache.can_reg_rdlr = CAN1->sFIFOMailBox[0].RDLR;
		CanFiFoL2Cache.can_reg_rdhr = CAN1->sFIFOMailBox[0].RDHR;

		/*д����Ϣ����
		ret = GM_MQ_Fixed_CanWrite(&g_LocalCanMQ,(UINT8*)&CanFiFoL2Cache,16);*/

		CAN1->RF0R |= 0x1<<5;		//�ͷ�FIFO0����
	}
}



/*********************************************************
* Function: 	ECU_CanRead
* Description:  ��CAN���߶�����
* Parameter:	canIndex *pFrame
* Return:       BOOL
			If process ok, returns TRUE, otherwise,returns FALSE
* History:
*a	 08/22/2019 15:43:08  WangLin	Created
********************************************************************/
BOOL ECU_CanRead(UINT8 canIndex,UINT8 *pFrame)
{
	BOOL ret = FALSE;
	
	ret = ECU_ReadCan1L2FiFoCache(pFrame);
		
	return ret;
}





/*********************************************************
* Function: 	ECU_CanWrite
* Description:  ��CAN����д����
* Parameter:	canIndex *pFrame
* Return:       BOOL
			If process ok, returns TRUE, otherwise,returns FALSE
* History:
*a	 08/22/2019 15:43:08  WangLin	Created
********************************************************************/
BOOL ECU_CanWrite(UINT8 canIndex,UINT8 *pFrame)
{
	UINT16 cnt = 2000;
	BOOL ret = FALSE;
	UINT8 i,empty_box,ide,rtr,dlc,buf[8]={0};
	UINT32 id=0,bufh=0,bufl=0;
	CanFrame_t *can=NULL;
	can = (CanFrame_t*)pFrame;
	
	id |= can->MsgHead.EndFlag << 3;
	id |= can->MsgHead.SubIndex << 4;
	id |= can->MsgHead.MsgIndex << 11;
	id |= can->MsgHead.SrcID << 19;
	id |= can->MsgHead.DstID << 23;
	id |= can->MsgHead.MsgType << 27;	
	
	ide = can->MsgHead.FF;
	rtr = can->MsgHead.RTR;
	dlc = can->MsgHead.DataLen;
	
	for(i=0;i<dlc;i++)
	{
		buf[i] = can->MsgData[i];
	}
	
	bufh |= buf[7] << 24;
	bufh |= buf[6] << 16;
	bufh |= buf[5] << 8;
	bufh |= buf[4] << 0;
	bufl |= buf[3] << 24;
	bufl |= buf[2] << 16;
	bufl |= buf[1] << 8;
	bufl |= buf[0] << 0;

	while(1)
	{
		if(CAN1->TSR & ((1<<26)|(1<<27)|(1<<28)))
		{
			empty_box = ((CAN1->TSR>>24) & 0x00000003);
			CAN1->sTxMailBox[empty_box].TIR = 0;
			CAN1->sTxMailBox[empty_box].TIR = id|ide<<2|rtr<<1;
			CAN1->sTxMailBox[empty_box].TDTR |= dlc;
			CAN1->sTxMailBox[empty_box].TDLR = bufl;
			CAN1->sTxMailBox[empty_box].TDHR = bufh;
			CAN1->sTxMailBox[empty_box].TIR |= (1<<0);
			ret = TRUE;
			break;
		}
		else
		{
			cnt--;
			if(cnt == 0)
			{
				ret = FALSE;
				break;
			}
		}
	}

	return TRUE;
}





/*********************************************************
* Function: 	CAN1_RX0_IRQHandler
* Description:  CAN�жϷ�����
* Parameter:	canIndex *pFrame
* Return:       BOOL
			If process ok, returns TRUE, otherwise,returns FALSE
* History:
*a	 08/22/2019 15:43:08  WangLin	Created
********************************************************************/
void CAN1_RX0_IRQHandler(void)
{
	ECU_WriteCan1L2FiFoCache();
}



