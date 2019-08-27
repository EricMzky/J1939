/********************************************************************
*XXXXXX System Development XXXXX.XXXX Software
*Copyright (c) 2019-2029, XXX LTD., All rights reserved.
*File Name:   XXXXX.h
*File Description: Implementation of  CanCommunication.h
*Modification History:
*a	 08/22/2019 15:43:08  WangLin	Created
*b	 08/26/2019 10:27:29  WangLin	增加J1939转CAN标准帧收发函数
********************************************************************/


#include "CanCommunication.h"


/********************************************************************
* Function: 	J1939_SetAddressFilter
* Description:  
* Parameter:	Ps_Address
* Note:			
				基于SAE J1939协议，我们需要CAN控制器提供至少3个滤波器给J1939协议代码。
				1. 设置滤波器0，只接受广播信息（PF = 240 -255）。 
				2. 设置设置滤波器1，2只接受全局地址（J1939_GLOBAL_ADDRESS） 
				3. 随着程序的运行，将改变滤波器2，来适应程序逻辑。
				J1939_SetAddressFilter() 是用来设置滤波器2的， 函数主要设置PS位（目标地址）,
				其目的是，让控制器只接受发送给本设备的消息。
* warning:		滤波器0，1是在CAN驱动里配置，如果对硬件滤波配置不是很熟练，可以使能软件滤波器,#define J1939SoftwareFilterEn 
				则可跳过本函数的移植和CAN硬件滤波器的配置，为了J1939协议栈性能最优化，建议只是用硬件滤波
				
* Return:    	No Return
* History:
*a	 08/22/2019 15:43:08  WangLin	Created
********************************************************************/
void J1939_SetAddressFilter(unsigned char Ps_Address)
{
	switch (Can_Node)
	{
		case Select_CAN_NODE_1:
		{
			break;
		}
		case Select_CAN_NODE_2:
		{
			break;
		}
		case Select_CAN_NODE_3:
		{
			break;
		}
		case Select_CAN_NODE_4:
		{
			break;
		}
		default  :
		{
			break;
		}
	}
}


/********************************************************************
* Function: 	J1939_CAN_Transmit
* Description:  
* Parameter:	[in] *pstMsg 协议要发送的消息，
* Note:			
				将数据 从pstMsg结构体赋值到CAN驱动自带的结构体中
				先将传入函数的pstMsg中的数据写到CAN的结构体，再调用CAN驱动的发送函数
				默认支持4路CAN硬件的收发。如少于4路，只需配置相应的Can_Node开关代码区，
				其他（Select_CAN_NODE）保持不变。就直接返回（break）。
				
* warning:		
* Return:    	TRUE->
				FALSE->
* History:
*a	 08/22/2019 15:43:08  WangLin	Created
********************************************************************/
void J1939_CAN_Transmit(J1939_MESSAGE *pstMsg)
{
	bool rcRet = false;
	
	switch (Can_Node)
	{
		case Select_CAN_NODE_1:
			{	
				rcRet = ECU_CAN_TransmitMsg(pstMsg);
			}
			break;
		
		case Select_CAN_NODE_2:
			{	
				rcRet = ECU_CAN_TransmitMsg(pstMsg);
			}
			break;

		case Select_CAN_NODE_3:
			{	
				rcRet = ECU_CAN_TransmitMsg(pstMsg);
			}
			break;


		case Select_CAN_NODE_4:
			{
				rcRet = ECU_CAN_TransmitMsg(pstMsg);
			}
			break;
		
		default  :
			{
				
			}
			break;
	}

}


/********************************************************************
* Function: 	J1939_CAN_Receive
* Description:  
* Parameter:	[in] *pstMsg 数据要存入的内存的指针
* Note:			
				读取CAN驱动的数据，如果没有数据，返回0
				将CAN中的数据取出，存入J1939_MESSAGE结构体中
				默认支持4路CAN硬件的收发。如少于4路，只需配置相应的Can_Node开关代码区，
				其他（Select_CAN_NODE）保持不变。就直接返回（return 0）

* warning:		
* Return:    	TRUE->
				FALSE->
* History:
*a	 08/22/2019 15:43:08  WangLin	Created
********************************************************************/
int J1939_CAN_Receive(J1939_MESSAGE *pstMsg)
{

	switch (Can_Node)
	{
		case Select_CAN_NODE_1:
			{
				//判断CAN硬件1是否有数据到来，信号量机制
				if(true)
				{
					//从CAN1 中将数据读取后，存入 pstMsg				
					ECU_CAN_ReceiveMsg(pstMsg);

					
					return 1;
				}
				return 0;
			}
			break;
		
		case Select_CAN_NODE_2:
			{
				if("你的代码")//判断CAN硬件2是否有数据到来
				{
					//你的代码，从CAN硬件2 中将数据读取后，存入 pstMsg
					return 1;
				}
				return 0;
			}
			break;

		case Select_CAN_NODE_3:
			{
				if("你的代码")//判断CAN硬件3是否有数据到来
				{
					//你的代码，从CAN硬件3 中将数据读取后，存入 pstMsg
					return 1;
				}
				return 0;
			}
			break;

		case Select_CAN_NODE_4:
			{
				if("你的代码")//判断CAN硬件4是否有数据到来
				{
					//你的代码，从CAN硬件4 中将数据读取后，存入 pstMsg
					return 1;
				}
				return 0;
			}
			break;

		default  :
		{
			return 0;//没有消息
			break;
		}
	}
	return 0;//没有消息
}


/********************************************************************
* Function: 	ECU_CAN_ReceiveMsg
* Description:  
* Parameter:	[in] *pstMsg 数据要存入的内存的指针
* Note:			
				读取CAN驱动的数据，如果没有数据，返回0
				将CAN中的数据取出，存入J1939_MESSAGE结构体中
				默认支持4路CAN硬件的收发。如少于4路，只需配置相应的Can_Node开关代码区，
				其他（Select_CAN_NODE）保持不变。就直接返回（return 0）

* warning:		
* Return:    	TRUE->
				FALSE->
* History:
*a	 08/25/2019 15:43:08  WangLin	Created
********************************************************************/
void ECU_CAN_ReceiveMsg(J1939_MESSAGE *pstMsg)
{
	UINT8 idType = 0U;
	UINT8 rtrType = 0U;
	UINT8 msgLen = 0U;

	UINT32 id = 0U;
	
	//读取CAN原始数据
	CAN1_Rx_Msg(CAN_MAIL_BOX_0, &id, CAN_IDE_ETD, &rtrType, pstMsg->Mxe.DataLength, pstMsg->Mxe.Data);

	//CAN原始ID数据转换成J1939消息帧
	pstMsg->Mxe.Priority = (UINT8)((id >> 26) & 0x07);
	pstMsg->Mxe.PDUFormat = (UINT8)((id >> 16) & 0xFF);
	pstMsg->Mxe.PDUSpecific = (UINT8)((id >> 8) & 0xFF);
	pstMsg->Mxe.SourceAddress = (UINT8)(id);
	
}

/********************************************************************
* Function: 	ECU_CAN_TransmitMsg
* Description:  
* Parameter:	[in] *pstMsg 数据要存入的内存的指针
* Note:			
				读取CAN驱动的数据，如果没有数据，返回0
				将CAN中的数据取出，存入J1939_MESSAGE结构体中
				默认支持4路CAN硬件的收发。如少于4路，只需配置相应的Can_Node开关代码区，
				其他（Select_CAN_NODE）保持不变。就直接返回（return 0）

* warning:		
* Return:    	TRUE->
				FALSE->
* History:
*a	 08/26/2019 15:43:08  WangLin	Created
********************************************************************/
bool ECU_CAN_TransmitMsg(J1939_MESSAGE *pstMsg)
{	
	UINT8 box = 0U;
	UINT8 index = 0;
	UINT16 i = 0;
	UINT32 id = 0U;
	UINT8 txMsg[8] = {0U};

	//加载29Bit ID
	id =  ((UINT32)pstMsg->Mxe.Priority   << 26)
		+ ((UINT32)pstMsg->Mxe.PDUFormat  << 16)
		+ ((UINT32)pstMsg->Mxe.PDUSpecific << 8)
		+  (UINT32)pstMsg->Mxe.SourceAddress;
	
	//CAN硬件开始发送数据	
	box = CAN1_Tx_Msg(id, CAN_IDE_ETD, pstMsg->Mxe.RTR, pstMsg->Mxe.DataLength, pstMsg->Mxe.Data);
	
	//等待发送结束
	while((0x07 != CAN1_Tx_Staus(box)) && (i < 0XFFF))
	{
		//等待发送超时机制，日后替换计时器
		i++;
	}
	if(i >= 0xFFF)
	{
		return false;	//发送失败?
	}
	else
	{
		return true;	//发送成功;
	}
}


/*不使用中断模式，不对下面的函数进行移植*/
#if J1939_POLL_ECAN == J1939_FALSE
/*
*输入：
*输出：
*说明：使能接受中断
*/
	void J1939_RXinterruptEnable()
	{
		;
	}
/*
*输入：
*输出：
*说明：失能接受中断
*/
	void J1939_RXinterruptDisable()
	{
		;
	}
/*
*输入：
*输出：
*说明：使能发送中断
*/
	void J1939_TXinterruptEnable()
	{
		;
	}
/*
*输入：
*输出：
*说明：失能发送中断
*/
	void J1939_TXinterruptDisable()
	{
		;
	}
/*
*输入：
*输出：
*说明：触发发送中断标致位，当协议栈在中断模式下，要发送消息，将调用此函数
	CAN驱动函数，就将直接把消息发送出去，不需要协议在调用任何can驱动函数
*/
	void J1939_TXinterruptOk()
	{
		;
	}
/*
*输入：
*输出：
*说明：清除CAN驱动相关的中断产生标识位，包括（发送中断标志位，接受中断标
	志位，can总线错误标识位）
*/
	void CAN_identifier_clc()
	{
		;
	}
#endif

