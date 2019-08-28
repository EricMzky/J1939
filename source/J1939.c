/********************************************************************
*XXXXXX System Development XXXXX.XXXX Software
*Copyright (c) 2019-2029, XXX LTD., All rights reserved.
*File Name:   DeviceCAN.h
*File Description: Implementation of  J1939.c
*Modification History:
*v1.0.0: 
	*a	2017/06/04	XeiTongXueFlyMe		首个版本
*v1.0.1:  
	*a	2017/08/04	XeiTongXueFlyMe	   	增加对TP的支持
*v2.0.1:
	*a	2017/11/24	XeiTongXueFlyMe	   	增加对多路CAN硬件的收发，和报文处理
	*b	2017/11/29	XeiTongXueFlyMe	   	增加请求和响应API
	*c  2017/12/07	XeiTongXueFlyMe	   	重做TP接受API函数
	*d  2017/12/08	XeiTongXueFlyMe	   	增加软件滤波器
*v2.0.2:
	*a	2018/01/03	XeiTongXueFlyMe	   	重做接受发送API，简化协议栈初始化调用逻辑
*v2.1.0:
	*a	2019/08/19  15:43:08 WangLin	Created New File
	*b	2019/08/25  10:13:29 WangLin	修改J1939_TP_Poll J1939_Poll函数;重定义各个传参命名
********************************************************************/
#include <assert.h>


#include "J1939.h"   
#include "CanCommunication.h"


#define J1939_TRUE         1	/**< 代表函数正确返回*/
#define J1939_FALSE        0	/**< 代表函数错误返回*/
#define ADDRESS_CLAIM_TX   1	/**< 进入地址竞争发送处理模式*/
#define ADDRESS_CLAIM_RX   2	/**< 进入地址竞争接受处理模式*/

//全局变量。
/** 设备的标称符
 *	
 *	我们需要在"J1939_config.H"中配置
 *	@note 在初始化中赋值，赋值参考参考1939-81文档
 */
UINT8                   CA_Name[J1939_DATA_LENGTH];
UINT8                   CommandedAddress; 
 
UINT8                   J1939_Address;   
J1939_FLAG              J1939_Flags;   
J1939_MESSAGE           OneMessage;   
CAN_NODE                Can_Node;
//节点地址
UINT8					NodeAddress_1;
UINT8					NodeAddress_2;
UINT8					NodeAddress_3;
UINT8					NodeAddress_4;

//接受列队全局变量(CAN_NODE_1) 
UINT8                   RXHead_1;
UINT8                   RXTail_1;
UINT8                   RXQueueCount_1;
J1939_MESSAGE           RXQueue_1[J1939_RX_QUEUE_SIZE];
//发送列队全局变量 (CAN_NODE_1)
UINT8                   TXHead_1;
UINT8                   TXTail_1;
UINT8                   TXQueueCount_1;
J1939_MESSAGE           TXQueue_1[J1939_TX_QUEUE_SIZE];
//接受列队全局变量(CAN_NODE_2)
UINT8                   RXHead_2;
UINT8                   RXTail_2;
UINT8                   RXQueueCount_2;
J1939_MESSAGE           RXQueue_2[J1939_RX_QUEUE_SIZE];
//发送列队全局变量 (CAN_NODE_2)
UINT8                   TXHead_2;
UINT8                   TXTail_2;
UINT8                   TXQueueCount_2;
J1939_MESSAGE           TXQueue_2[J1939_TX_QUEUE_SIZE];
//接受列队全局变量(CAN_NODE_3)
UINT8                   RXHead_3;
UINT8                   RXTail_3;
UINT8                   RXQueueCount_3;
J1939_MESSAGE           RXQueue_3[J1939_RX_QUEUE_SIZE];
//发送列队全局变量 (CAN_NODE_3)
UINT8                   TXHead_3;
UINT8                   TXTail_3;
UINT8                   TXQueueCount_3;
J1939_MESSAGE           TXQueue_3[J1939_TX_QUEUE_SIZE];
//接受列队全局变量(CAN_NODE_4)
UINT8                   RXHead_4;
UINT8                   RXTail_4;
UINT8                   RXQueueCount_4;
J1939_MESSAGE           RXQueue_4[J1939_RX_QUEUE_SIZE];
//发送列队全局变量 (CAN_NODE_4)
UINT8                   TXHead_4;
UINT8                   TXTail_4;
UINT8                   TXQueueCount_4;
J1939_MESSAGE           TXQueue_4[J1939_TX_QUEUE_SIZE];


struct Request_List REQUEST_LIST;

#if J1939_TP_RX_TX
//TP协议全局变量  
J1939_TP_Flags                  J1939_TP_Flags_t;   
J1939_TRANSPORT_RX_INFO         TP_RX_MSG;    
J1939_TRANSPORT_TX_INFO         TP_TX_MSG;
#endif //J1939_TP_RX_TX





/***************************define fountion*************************/
void J1939_Response(const UINT32 uwPGN);
void J1939_CM_Start(void);
void J1939_TP_TX_Abort(void);
void J1939_TP_RX_Abort(void);
void J1939_TP_DT_Packet_send(void);
void J1939_read_DT_Packet(void);
void J1939_SetSoftWareFilter(UINT8 Address);

bool SendOneMessage(J1939_MESSAGE *pstMsg);


UINT8 J1939_ReceiveMessages(void);
UINT8 J1939_TransmitMessages(void);
UINT8 J1939_TP_RX_RefreshCMTimer(UINT16 usMS);
UINT8 J1939_TP_TX_RefreshCMTimer(UINT16 usMS);
UINT8 J1939_Messages_Filter(J1939_MESSAGE *pstMsg);
UINT8 J1939_EnqueueMessage(J1939_MESSAGE *pstMsg, CAN_NODE canNode);
UINT8 J1939_DequeueMessage(J1939_MESSAGE *pstMsg, CAN_NODE canNode);



/********************************************************************
* Function: 	SetAddressFilter
* Description:  
* Parameter:	Address
* Note:			硬件滤波器2 或 软件滤波器  滤波配置(设置PS段)
* Return:    	NO RETURN
* History:
*a	 08/19/2019 15:43:08  WangLin	Created
********************************************************************/
void J1939_SetSoftWareFilter(UINT8 Address)
{   
	/*软件滤波*/
	#if J1939SoftwareFilterEn == J1939_TRUE
		switch (Can_Node)
		{
			case Select_CAN_NODE_1:
				{
					NodeAddress_1 = Address;
				}
				break;
			
			case Select_CAN_NODE_2:
				{
					NodeAddress_2 = Address;
				}
				break;
			
			case Select_CAN_NODE_3:
				{
					NodeAddress_3 = Address;	
				}
				break;
			
			case Select_CAN_NODE_4:
				{
					NodeAddress_4 = Address;
				}
				break;
			
			default :
				{
				
				}
				break;
		}
	#endif//J1939SoftwareFilterEn

	/*在DeviceCAN文件中配置CAN硬件过滤
	Port_SetHardWareFilter(Address);*/
}   


/********************************************************************
* Function: 	SendOneMessage
* Description:	
* Parameter:	pstMsg

* note 			发送*pstMsg的信息，所有的数据字段
				(比如数据长度、优先级、和源地址)必须已经设置。
* Return:		NO RETURN
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
bool SendOneMessage(J1939_MESSAGE *pstMsg)
{
	bool rcRet = false;
	
	assert(NULL != pstMsg);
		
    //设置消息的最后部分,确保DataLength规范(参考CAN B2.0)
	pstMsg->Mxe.Res = 0;//参考J1939的数据链路层(SAE J1939-21)
	pstMsg->Mxe.RTR = 0;
	
    if (pstMsg->Mxe.DataLength > 8)
    {
    	pstMsg->Mxe.DataLength = 8;
    }
    	
	//将J1939_MESSAGE中的所有消息转化CAN标准帧格式并发送
	rcRet = Port_CAN_Transmit(pstMsg);

	return rcRet;
}   


/********************************************************************
* Function: 	J1939_DequeueMessage
* Description:	
* Parameter:	pstMsg->用户要出队的消息, canNode        ->要出队的CAN硬件编号
				
* note 			从接受队列中读取一个信息到*pstMsg。如果我们用的是中断，
				需要将中断失能，在获取接受队列数据时
* Return:		RC_SUCCESS, RC_QUEUEEMPTY
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
UINT8 J1939_DequeueMessage(J1939_MESSAGE *pstMsg, CAN_NODE canNode)
{   
    UINT8 _rc = RC_SUCCESS;

	/**************************关接收中断********************************/  
	#if J1939_POLL_ECAN == J1939_FALSE
		Port_RXinterruptDisable();
	#endif
	
    switch (canNode)
	{
		case Select_CAN_NODE_1:
			{
			    if (RXQueueCount_1 == 0)
			    {
					_rc = RC_QUEUEEMPTY;
			    }
			    else
			    {
			        *pstMsg = RXQueue_1[RXHead_1];
			        RXHead_1 ++;
					
			        if (RXHead_1 >= J1939_RX_QUEUE_SIZE)
			        {
			        	RXHead_1 = 0;
			        }
			        RXQueueCount_1 --;
			    }

			}
			break;
		
		case Select_CAN_NODE_2:
			{
			    if (RXQueueCount_2 == 0)
			    {
			    	_rc = RC_QUEUEEMPTY;
			    }
			    else
			    {
			        *pstMsg = RXQueue_2[RXHead_2];
			        RXHead_2 ++;
			        if (RXHead_2 >= J1939_RX_QUEUE_SIZE)
			        {
			        	RXHead_2 = 0;
			        }
			        RXQueueCount_2 --;
			    }
			}
			break;
		
		case Select_CAN_NODE_3:
			{
			    if (RXQueueCount_3 == 0)
			    {
			    	_rc = RC_QUEUEEMPTY;
			    }
			    else
			    {
			        *pstMsg = RXQueue_3[RXHead_3];
			        RXHead_3 ++;
			        if (RXHead_3 >= J1939_RX_QUEUE_SIZE)
			        {
			        	RXHead_3 = 0;
			        }
			        RXQueueCount_3 --;
			    }
			}
			break;

		case Select_CAN_NODE_4:
			{
				if (RXQueueCount_4 == 0)
				{
					_rc = RC_QUEUEEMPTY;
				}
				else
				{
					*pstMsg = RXQueue_4[RXHead_4];
					RXHead_4 ++;
					if (RXHead_4 >= J1939_RX_QUEUE_SIZE)
					{
						RXHead_4 = 0;
					}
					RXQueueCount_4 --;
				}
			}
			break;
		
		default  :
			{
				_rc = RC_CANNOTRECEIVE;
			}
			break;
	}
	
  	/**************************开接受中断********************************/
	#if J1939_POLL_ECAN == J1939_FALSE
   		Port_RXinterruptEnable();
	#endif

	return _rc;
}

/********************************************************************
* Function: 	J1939_EnqueueMessage
* Description:	
* Parameter:	pstMsg->用户要入队的消息, canNode        ->要入队的CAN硬件编号（要选择的使用的CAN硬件编号）
* note 			这段程序，将*pstMsg放入发送消息列队中,如果信息不能入队或者发送，将有一个相应的返回提示,
				如果发送中断被设置（可用），当消息列队后，发送中断被使能
* Return:		RC_SUCCESS          消息入队成功 
				RC_QUEUEFULL        发送列队满，消息入队失败 
				RC_CANNOTTRANSMIT   系统目前不能发送消息  
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
UINT8 J1939_EnqueueMessage(J1939_MESSAGE *pstMsg, CAN_NODE canNode)
{   
    UINT8 _rc = RC_SUCCESS;

	#if J1939_POLL_ECAN == J1939_FALSE  
    	Port_TXinterruptDisable();
	#endif 
   
    if (0)
    {
    	_rc = RC_CANNOTTRANSMIT;
    }    
    else   
    {   
    	switch (canNode)
		{
			case  Select_CAN_NODE_1:
				{
					if ((J1939_OVERWRITE_TX_QUEUE == J1939_TRUE) || (TXQueueCount_1 < J1939_TX_QUEUE_SIZE))
					{
						if (TXQueueCount_1 < J1939_TX_QUEUE_SIZE)
						{
							TXQueueCount_1 ++;
							TXTail_1 ++;
							if (TXTail_1 >= J1939_TX_QUEUE_SIZE)
							{
								TXTail_1 = 0;
							}
							
						}
						else
						{
							J1939_Flags.TransmitMessagesdCover = 1;//发送数据被覆盖，上一帧数据被覆盖
						}
						
						TXQueue_1[TXTail_1] = *pstMsg;
					}
					else
					{
						_rc = RC_QUEUEFULL;
					}
				}
				break;
			
			case Select_CAN_NODE_2:
				{
					if ((J1939_OVERWRITE_TX_QUEUE == J1939_TRUE) || (TXQueueCount_2 < J1939_TX_QUEUE_SIZE))
					{
						if (TXQueueCount_2 < J1939_TX_QUEUE_SIZE)
						{
							TXQueueCount_2 ++;
							TXTail_2 ++;
							if (TXTail_2 >= J1939_TX_QUEUE_SIZE)
							{
								TXTail_2 = 0;
							}
						}
						else
						{
							J1939_Flags.TransmitMessagesdCover = 1;//发送数据被覆盖，上一帧数据被覆盖
						}
						
						TXQueue_2[TXTail_2] = *pstMsg;
					}
					else
					{
						_rc = RC_QUEUEFULL;
					}	
				}
				break;
			
			case Select_CAN_NODE_3:
				{
					if ((J1939_OVERWRITE_TX_QUEUE == J1939_TRUE) || (TXQueueCount_3 < J1939_TX_QUEUE_SIZE))
					{
						if (TXQueueCount_3 < J1939_TX_QUEUE_SIZE)
						{
							TXQueueCount_3 ++;
							TXTail_3 ++;
							if (TXTail_3 >= J1939_TX_QUEUE_SIZE)
							{
								TXTail_3 = 0;
							}
						}
						else
						{
							J1939_Flags.TransmitMessagesdCover = 1;//发送数据被覆盖，上一帧数据被覆盖
						}
						
						TXQueue_3[TXTail_3] = *pstMsg;
					}
					else
					{
						_rc = RC_QUEUEFULL;
					}	
				}
				break;
			
			case  Select_CAN_NODE_4:
				{
					if ((J1939_OVERWRITE_TX_QUEUE == J1939_TRUE) || (TXQueueCount_4 < J1939_TX_QUEUE_SIZE))
					{
						if (TXQueueCount_4 < J1939_TX_QUEUE_SIZE)
						{
							TXQueueCount_4 ++;
							TXTail_4 ++;
							if (TXTail_4 >= J1939_TX_QUEUE_SIZE)
							{
								TXTail_4 = 0;
							}	
						}
						else
						{
							J1939_Flags.TransmitMessagesdCover = 1;//发送数据被覆盖，上一帧数据被覆盖
						}
						
						TXQueue_4[TXTail_4] = *pstMsg;
					}
					else
					{
						_rc = RC_QUEUEFULL;
					}
				}
				break;
			
			default :
				{
					
				}
				break;
		}
    }   

	#if J1939_POLL_ECAN == J1939_FALSE   
    	Port_TXinterruptEnable();
    	//触发发送中断
    	Port_TXinterruptOk();
	#endif
	
    return _rc;
}  


/********************************************************************
* Function: 	J1939_Read_Message
* Description:	
* Parameter:	pstMsg->存储读取消息的缓存, canNode        ->读取消息的CAN硬件编号（从哪一路CAN读取数据）
* note 			从接受队列中读取一个信息到*pstMsg
* Return:		RC_SUCCESS, RC_QUEUEEMPTY
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
UINT8 J1939_Read_Message(J1939_MESSAGE *pstMsg, CAN_NODE canNode)
{
	return J1939_DequeueMessage(pstMsg, canNode);
}


/********************************************************************
* Function: 	J1939_Send_Message
* Description:	
* Parameter:	pstMsg->存储发送消息的缓存, canNode        ->发送消息的CAN硬件编号（从哪一路CAN发送数据）
* note 			这段程序，将*pstMsg放入发送消息列队中,如果信息不能入队或者发送，将有一个相应的返回提示,
				如果发送中断被设置（可用），当消息列队后，发送中断被使能
* Return:		RC_SUCCESS          消息入队成功 
				RC_QUEUEFULL        发送列队满，消息入队失败 
				RC_CANNOTTRANSMIT   系统目前不能发送消息  
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
UINT8 J1939_Send_Message(J1939_MESSAGE *pstMsg, CAN_NODE canNode)
{
	return J1939_EnqueueMessage(pstMsg,canNode);
}


/********************************************************************
* Function: 	J1939_Initialization
* Description:	
* Parameter:	void
* note 			这段代码在系统初始化中被调用,(放在CAN设备初始化之后)初始化J1939全局变量
* Return:		No Return
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
void J1939_Initialization(void)
{
    /*初始化全局变量*/   
    J1939_Flags.FlagVal = 0; //没有声明地址，其他的标识位将被设置为0（复位）

    /*初始化接受和发送列队*/
    TXHead_1 = 0;
    TXHead_2 = 0;
    TXHead_3 = 0;
    TXHead_4 = 0;
    TXTail_1 = 0xFF;
    TXTail_2 = 0xFF;
    TXTail_3 = 0xFF;
    TXTail_4 = 0xFF;
    RXHead_1 = 0;
    RXHead_2 = 0;
    RXHead_3 = 0;
    RXHead_4 = 0;
    RXTail_1 = 0xFF;
    RXTail_2 = 0xFF;
    RXTail_3 = 0xFF;
    RXTail_4 = 0xFF;
	TXQueueCount_1 = 0;
	TXQueueCount_2 = 0;
	TXQueueCount_3 = 0;
	TXQueueCount_4 = 0;
    RXQueueCount_1 = 0;
    RXQueueCount_2 = 0;
    RXQueueCount_3 = 0;
    RXQueueCount_4 = 0;
	
	/*初始化节点地址*/
	NodeAddress_1 = J1939_STARTING_ADDRESS_1;
	NodeAddress_2 = J1939_STARTING_ADDRESS_2;
	NodeAddress_3 = J1939_STARTING_ADDRESS_3;
	NodeAddress_4 = J1939_STARTING_ADDRESS_4;
	
    /*初始化CAN节点的选择*/
    Can_Node = Select_CAN_NODE_1;
	
    /*初始化请求链表*/
    REQUEST_LIST.PGN = 0;
    REQUEST_LIST.data = NULL;
	REQUEST_LIST.update = NULL;
	REQUEST_LIST.lenght = 0;
	REQUEST_LIST.Can_Node = Select_CAN_NODE_Null;
	REQUEST_LIST.next = NULL;

	/*测试*/
	Can_Node = Select_CAN_NODE_1;
    J1939_Address = NodeAddress_1;

	
    /*将TP协议置为空闲*/
	#if J1939_TP_RX_TX
	    J1939_TP_Flags_t.state = J1939_TP_NULL;
	    J1939_TP_Flags_t.TP_RX_CAN_NODE = Select_CAN_NODE_Null;
	    J1939_TP_Flags_t.TP_TX_CAN_NODE = Select_CAN_NODE_Null;

	    TP_TX_MSG.packets_request_num = 0;
	    TP_TX_MSG.packets_total = 0;
		TP_TX_MSG.packet_offset_p = 0;
		TP_TX_MSG.time = 0;
		TP_TX_MSG.state = J1939_TP_TX_WAIT;

	    TP_RX_MSG.packets_ok_num = 0;
		TP_RX_MSG.packets_total = 0;
		TP_RX_MSG.time = 0;
		TP_RX_MSG.state = J1939_TP_RX_WAIT;
	#endif
}   



/********************************************************************
* Function: 	J1939_ISR
* Description:	
* Parameter:	void
* note 			这个函数被调用，当设备产生CAN中断(可能是接受中断，也可能是发送中断),
				首先我们要清除中断标识位        然后调用接受或者发送函数。
* Return:		No Return
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
#if J1939_POLL_ECAN == J1939_FALSE   
void J1939_ISR(void)
{   
    //判断相关标识位,是接受还是发送
    //清除标识位
    Port_CAN_identifier_clc();
	
    //调用相关的处理函数    
    J1939_ReceiveMessages();
    J1939_TransmitMessages();
	
    #if J1939_TP_RX_TX
        J1939_TP_Poll();
    #endif //J1939_TP_RX_TX
	
    //可能存在因为错误产生中断，直接清除相关的标识位 
}   
#endif   


/********************************************************************
* Function: 	J1939_Poll
* Description:	
* Parameter:	void
* note:
				如果我们采用轮询的方式获取信息，这个函数每几个毫秒将被调用一次
				不断的接受消息和发送消息从消息队列中
				此外，如果我们正在等待一个地址竞争反应
				如果超时，我们只接收特定的消息（目标地址 = J1939_Address）
				如果设备使用中断，此函数被调用，在调用J1939_Initialization（）函数后，因为
				J1939_Initialization（）可能初始化WaitingForAddressClaimContention标识位为1.
				如果接受到命令地址消息，这个函数也必须被调用，以防万一总线要求我们改变地址
				如果使用中断模式，本程序将不会处理接受和发送消息，只处理地址竞争超时
* Return:		No Return
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
void J1939_Poll(void)
{
    //我们必须调用J1939_ReceiveMessages接受函数，在时间被重置为0之前。
#if J1939_POLL_ECAN == J1939_TRUE
    	Can_Node = Select_CAN_NODE_1;
    	J1939_Address = NodeAddress_1;
        J1939_ReceiveMessages();
        J1939_TransmitMessages();
		
	#if 0		
    	Can_Node = Select_CAN_NODE_2;
    	J1939_Address = NodeAddress_2;
        J1939_ReceiveMessages();
        J1939_TransmitMessages();
		
    	Can_Node = Select_CAN_NODE_3;
    	J1939_Address = NodeAddress_3;
        J1939_ReceiveMessages();
        J1939_TransmitMessages();
		
    	Can_Node = Select_CAN_NODE_4;
        J1939_Address = NodeAddress_4;
        J1939_ReceiveMessages();
        J1939_TransmitMessages();
	#endif

	/**/
	#if J1939_TP_RX_TX
	       J1939_TP_Poll();
	#endif //J1939_TP_RX_TX
	
#endif //J1939_POLL_ECAN == J1939_TRUE 
}   


/********************************************************************
* Function: 	J1939_Messages_Filter
* Description:	
* Parameter:	void
* note:
				软件滤波器
				基于SAE J1939协议，我们需要CAN控制器提供至少3个滤波器给J1939协议代码。三个滤波器分别配置如下：
		        1. 设置滤波器0，只接受广播信息（PF = 240 -255）。
		        2. 设置设置滤波器1，2只接受全局地址（J1939_GLOBAL_ADDRESS）
		        3. 随着程序的运行，将改变滤波器2，来适应程序逻辑。
* Return:		RC_SUCCESS         消息是可以接受
				RC_CANNOTTRANSMIT  消息是不可以接受
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
#if J1939SoftwareFilterEn == J1939_TRUE
UINT8 J1939_Messages_Filter(J1939_MESSAGE *pstMsg)
{
    /*滤波器0*/
    if((pstMsg->Mxe.PDUFormat) >= 240)
    {
        return RC_SUCCESS;
    }
    /*滤波器1*/
    if(((pstMsg->Mxe.PDUFormat) < 240) && (pstMsg->Mxe.PDUSpecific == J1939_GLOBAL_ADDRESS))
    {
        return RC_SUCCESS;
    }
	/*滤波器2*/
	switch (Can_Node)
	{
		case  Select_CAN_NODE_1:
			{
				if(((pstMsg->Mxe.PDUFormat) < 240) && (pstMsg->Mxe.PDUSpecific == NodeAddress_1))
				{
					return RC_SUCCESS;
				}
			}
			break;
		
		case  Select_CAN_NODE_2:
			{
				if(((pstMsg->Mxe.PDUFormat) < 240) && (pstMsg->Mxe.PDUSpecific == NodeAddress_2))
				{
					return RC_SUCCESS;
				}
			}
			break;
		
		case  Select_CAN_NODE_3:
			{
				if(((pstMsg->Mxe.PDUFormat) < 240) && (pstMsg->Mxe.PDUSpecific == NodeAddress_3))
				{
					return RC_SUCCESS;
				}
			}
			break;
		
		case  Select_CAN_NODE_4:
			{
				if(((pstMsg->Mxe.PDUFormat) < 240) && (pstMsg->Mxe.PDUSpecific == NodeAddress_4))
				{
					return RC_SUCCESS;
				}
			}
			break;
		
		default  :
			{
			
			}
			break;
	}
	
    return RC_CANNOTTRANSMIT;
}
#endif //J1939SoftwareFilterEn


/********************************************************************
* Function: 	J1939_ReceiveMessages
* Description:	
* Parameter:	void
* note:
				这段程序被调用，当CAN收发器接受数据（中断 或者 轮询）
				如果一个信息被接受, 它将被调用
				如果信息是一个网络管理信息或长帧传输（TP），接受的信息将被加工处理，在这个函数中
				否则, 信息将放置在用户的接收队列
				注意：在这段程序运行期间中断是失能的
				
* Return:		RC_SUCCESS         消息是可以接受
				RC_CANNOTTRANSMIT  消息是不可以接受
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
UINT8 J1939_ReceiveMessages(void) 
{
	int ret = 0;
	
	#if J1939_TP_RX_TX
		UINT32 _pgn = 0;
	#endif //J1939_TP_RX_TX

    /*从接收缓存中读取信息到OneMessage中，OneMessage是一个全局变量*/	
    /*读取CAN邮箱内的原始数据, Port_CAN_Receive函数读取到数据返回1，没有数据则返回0*/
	ret = Port_CAN_Receive(&OneMessage);
	
    if(ret)
    {
    	/*软件滤波*/
		#if J1939SoftwareFilterEn == J1939_TRUE
		    if(J1939_Messages_Filter(&OneMessage) != RC_SUCCESS)
		    {
		        return RC_CANNOTRECEIVE;
		    }
		#endif //J1939SoftwareFilterEn
		
        switch(OneMessage.Mxe.PDUFormat)
        { 
			#if J1939_TP_RX_TX
				case J1939_PF_TP_CM:       //参考J1939-21 TP多帧传输协议
					{
						_pgn = (UINT32)((OneMessage.Mxe.Data[7] << 16) & 0xFF0000)
	                						+(UINT32)((OneMessage.Mxe.Data[6] << 8) & 0xFF00)
	                						+(UINT32)((OneMessage.Mxe.Data[5]) & 0xFF);
						if((J1939_TP_Flags_t.state == J1939_TP_NULL) && (TP_RX_MSG.state == J1939_TP_RX_WAIT))
						{
							if(OneMessage.Mxe.Data[0] == 16)
							{
								J1939_TP_Flags_t.state = J1939_TP_RX;
								J1939_TP_Flags_t.TP_RX_CAN_NODE = Can_Node;

								TP_RX_MSG.tp_rx_msg.SA = OneMessage.Mxe.SourceAddress;
								TP_RX_MSG.tp_rx_msg.PGN = (UINT32)((OneMessage.Mxe.Data[7] << 16) & 0xFF0000)
															+(UINT32)((OneMessage.Mxe.Data[6] << 8) & 0xFF00)
															+(UINT32)((OneMessage.Mxe.Data[5]) & 0xFF);
								
								/*如果系统繁忙*/
								if(TP_RX_MSG.osbusy)
								{
									TP_RX_MSG.state = J1939_TP_RX_ERROR;
									return RC_CANNOTRECEIVE;
								}
								
								/*判断是否有足够的内存接收数据，如果没有直接，断开连接*/
								if(((UINT32)((OneMessage.Mxe.Data[2] << 8) & 0xFF00) + (UINT32)((OneMessage.Mxe.Data[1]) & 0xFF)) > J1939_TP_MAX_MESSAGE_LENGTH)
								{
									TP_RX_MSG.state = J1939_TP_RX_ERROR;
									return RC_QUEUEFULL;
								}
								
								TP_RX_MSG.tp_rx_msg.byte_count = ((UINT32)((OneMessage.Mxe.Data[2] << 8) & 0xFF00) + (UINT32)((OneMessage.Mxe.Data[1]) & 0xFF));
								TP_RX_MSG.packets_total = OneMessage.Mxe.Data[3];
								TP_RX_MSG.time = J1939_TP_T2;
								TP_RX_MSG.state = J1939_TP_RX_READ_DATA;
								
								break;
							}
							goto PutInReceiveQueue;
						}
						
						if(J1939_TP_Flags_t.state == J1939_TP_TX)
						{
							/*校验PGN*/
							if (_pgn == TP_TX_MSG.tp_tx_msg.PGN)
							{
								switch(OneMessage.Mxe.Data[0])
								{
									case J1939_RTS_CONTROL_BYTE:
										{
										
										}
										/* 程序运行到这里，说明已经与网络中设备1建立虚拟链接（作为发送端），但是收到设备2的链接请求，并且同一个PGN消息请求*/
										/* 根据J1939-21数据链路层的规定，我们要保持原有的链接，不做任何事，设备2会应为超时自动放弃链接*/
										break;
									
									case J1939_CTS_CONTROL_BYTE:
										{
											if((J1939_TP_TX_CM_WAIT == TP_TX_MSG.state) || (J1939_TP_WAIT_ACK == TP_TX_MSG.state))
											{
												/* 发送等待保持 */
												if(0x00u == OneMessage.Mxe.Data[1])
												{
													/* 刷新等待计数器 */
													TP_TX_MSG.time = J1939_TP_T4;
												}
												else
												{
													if((OneMessage.Mxe.Data[2] + OneMessage.Mxe.Data[1]) > (TP_TX_MSG.packets_total + 1))
													{
														/*请求超出数据包范围*/
														TP_TX_MSG.state = J1939_TP_TX_ERROR;
													}
													else
													{ /* response parameter OK */
														TP_TX_MSG.packets_request_num = OneMessage.Mxe.Data[1];
														TP_TX_MSG.packet_offset_p = (UINT8)(OneMessage.Mxe.Data[2] - 1);
														TP_TX_MSG.state = J1939_TP_TX_DT;
													}
												}
											}	
										}
										break;
									
									case J1939_EOMACK_CONTROL_BYTE:
										{
											if(J1939_TP_WAIT_ACK == TP_TX_MSG.state)
											{
												TP_TX_MSG.state = J1939_TX_DONE;
											}
										}
										//这里可以增加一个对数据的校验
										break;
									
									case J1939_CONNABORT_CONTROL_BYTE:
										{
											//收到一个放弃连接，什么都不做，协议会在一段延时时间后主动放弃链接
									
										}
										break;
									
									default:
										{
											
										}
										break;
								}
							}
						}
						goto PutInReceiveQueue;
					}
			#endif//J1939_TP_RX_TX

			#if J1939_TP_RX_TX
	            case J1939_PF_DT:  
	            	{
	            		if((TP_RX_MSG.state == J1939_TP_RX_DATA_WAIT) && (TP_RX_MSG.tp_rx_msg.SA == OneMessage.Mxe.SourceAddress))
		                {
		                	TP_RX_MSG.tp_rx_msg.data[(OneMessage.Mxe.Data[0] - 1) *7u + 0] = OneMessage.Mxe.Data[1];
		                	TP_RX_MSG.tp_rx_msg.data[(OneMessage.Mxe.Data[0] - 1) *7u + 1] = OneMessage.Mxe.Data[2];
		                	TP_RX_MSG.tp_rx_msg.data[(OneMessage.Mxe.Data[0] - 1) *7u + 2] = OneMessage.Mxe.Data[3];
		                	TP_RX_MSG.tp_rx_msg.data[(OneMessage.Mxe.Data[0] - 1) *7u + 3] = OneMessage.Mxe.Data[4];
		                	TP_RX_MSG.tp_rx_msg.data[(OneMessage.Mxe.Data[0] - 1) *7u + 4] = OneMessage.Mxe.Data[5];
		                	TP_RX_MSG.tp_rx_msg.data[(OneMessage.Mxe.Data[0] - 1) *7u + 5] = OneMessage.Mxe.Data[6];
		                	TP_RX_MSG.tp_rx_msg.data[(OneMessage.Mxe.Data[0] - 1) *7u + 6] = OneMessage.Mxe.Data[7];
							
							/*特殊处理重新接受已接受过的数据包*/
		                	if((OneMessage.Mxe.Data[0]) > TP_RX_MSG.packets_ok_num)
							{
		                		TP_RX_MSG.packets_ok_num++;
							}
							TP_RX_MSG.time = J1939_TP_T1;
							
							/*判断是否收到偶数个数据包或者读取到最后一个数据包*/
							if((0 == (TP_RX_MSG.packets_ok_num % 2)) ||(TP_RX_MSG.packets_ok_num == TP_RX_MSG.packets_total))
							{
								TP_RX_MSG.state = J1939_TP_RX_READ_DATA;
								break;
							}
		                }
	            	}
	                break;
				
	                /*程序不可能运行到这，但是我们不能放弃接受的数据包
	                goto PutInReceiveQueue;*/
				
			#endif//J1939_TP_RX_TX
			
            	case J1939_PF_REQUEST: 
	            	{
	            		/*用OneMessage.Mxe.PGN 来存下被请求的PGN*/
						if(OneMessage.Mxe.Data[1] < 240)
						{
							OneMessage.Mxe.PGN = (UINT32)((OneMessage.Mxe.Data[2] << 16) & 0x030000)
												+(UINT32)((OneMessage.Mxe.Data[1] << 8) & 0xFF00)
												+0x00;
						}
						else
						{
							OneMessage.Mxe.PGN = (UINT32)((OneMessage.Mxe.Data[2] << 16) & 0x030000)
												+(UINT32)((OneMessage.Mxe.Data[1] << 8) & 0xFF00)
												+(UINT32)((OneMessage.Mxe.Data[0]) & 0xFF);
						}
						J1939_Response(OneMessage.Mxe.PGN);
	            	}	
	                break;
			
            	default:
            		{
	            		PutInReceiveQueue:
						/*
						if(OneMessage.Mxe.PDUFormat < 240){
							OneMessage.Mxe.PGN = (UINT32)((OneMessage.Array[0]<<16)&0x030000)
												+(UINT32)((OneMessage.Array[1]<<8)&0xFF00)
												+0x00;
						}else{
							OneMessage.Mxe.PGN = (UINT32)((OneMessage.Array[0]<<16)&0x030000)
												+(UINT32)((OneMessage.Array[1]<<8)&0xFF00)
												+(UINT32)((OneMessage.Array[2])&0xFF);
						}
						*/
		            	if(OneMessage.Mxe.PDUFormat < 240)
						{
		            		OneMessage.Mxe.PGN = (OneMessage.Mxe.Res << 17)
		            							+(OneMessage.Mxe.DataPage << 16)
												+(OneMessage.Mxe.PDUFormat << 8);
		            	}
						else
						{
		            		OneMessage.Mxe.PGN = (OneMessage.Mxe.Res << 17)
		            							+(OneMessage.Mxe.DataPage << 16)
												+(OneMessage.Mxe.PDUFormat << 8)
												+ OneMessage.Mxe.PDUSpecific;
		            	}

		            	switch (Can_Node)
						{
							case Select_CAN_NODE_1:
								{
									if ( (J1939_OVERWRITE_RX_QUEUE == J1939_TRUE) || (RXQueueCount_1 < J1939_RX_QUEUE_SIZE))
									{
										if (RXQueueCount_1 < J1939_RX_QUEUE_SIZE)
										{
											RXQueueCount_1++;
											RXTail_1++;
											if (RXTail_1 >= J1939_RX_QUEUE_SIZE)
											{
												RXTail_1 = 0;
											}
										}
										else
										{
											J1939_Flags.ReceivedMessagesdCover = 1; //产生数据覆盖
											J1939_Flags.ReceivedMessagesdCoverOrDroppedNode = Select_CAN_NODE_1;
										}
										RXQueue_1[RXTail_1] = OneMessage;
									}
									else
									{
										J1939_Flags.ReceivedMessagesDropped = 1; //产生数据溢出
									}	
									
								}
								break;
							
							case Select_CAN_NODE_2:
								{
									if ( (J1939_OVERWRITE_RX_QUEUE == J1939_TRUE) || (RXQueueCount_2 < J1939_RX_QUEUE_SIZE))
									{
										if (RXQueueCount_2 < J1939_RX_QUEUE_SIZE)
										{
											RXQueueCount_2++;
											RXTail_2++;
											if (RXTail_2 >= J1939_RX_QUEUE_SIZE)
											{
												RXTail_2 = 0;
											}
												
										}
										else
										{
											J1939_Flags.ReceivedMessagesdCover = 1; //产生数据覆盖
											J1939_Flags.ReceivedMessagesdCoverOrDroppedNode = Select_CAN_NODE_2;
										}
										RXQueue_2[RXTail_2] = OneMessage;
									}
									else
									{
										J1939_Flags.ReceivedMessagesDropped = 1;
									}
								}
								break;
							
							case Select_CAN_NODE_3:
								{
									if ( (J1939_OVERWRITE_RX_QUEUE == J1939_TRUE) || (RXQueueCount_3 < J1939_RX_QUEUE_SIZE))
									{
										if (RXQueueCount_3 < J1939_RX_QUEUE_SIZE)
										{
											RXQueueCount_3++;
											RXTail_3++;
											if (RXTail_3 >= J1939_RX_QUEUE_SIZE)
											{
												RXTail_3 = 0;
											}
										}
										else
										{
											J1939_Flags.ReceivedMessagesdCover = 1; //产生数据覆盖
											J1939_Flags.ReceivedMessagesdCoverOrDroppedNode = Select_CAN_NODE_3;
										}
										RXQueue_3[RXTail_3] = OneMessage;
									}
									else
									{
										J1939_Flags.ReceivedMessagesDropped = 1;
									}
								}
								break;
							
							case Select_CAN_NODE_4:
								{
									if ( (J1939_OVERWRITE_RX_QUEUE == J1939_TRUE) || (RXQueueCount_4 < J1939_RX_QUEUE_SIZE))
									{
										if (RXQueueCount_4 < J1939_RX_QUEUE_SIZE)
										{
											RXQueueCount_4++;
											RXTail_4++;
											if (RXTail_4 >= J1939_RX_QUEUE_SIZE)
											{
												RXTail_4 = 0;
											}
										}
										else
										{
											J1939_Flags.ReceivedMessagesdCover = 1; //产生数据覆盖
											J1939_Flags.ReceivedMessagesdCoverOrDroppedNode = Select_CAN_NODE_4;
										}
										RXQueue_4[RXTail_4] = OneMessage;
									}
									else
									{
										J1939_Flags.ReceivedMessagesDropped = 1;
									}
								}
								break;
							
							default :	
								{
									
								}
								break;
					}
				}	
       }
	}

	return ret;
}   



/********************************************************************
* Function: 	J1939_TransmitMessages
* Description:	
* Parameter:	void
* note:
				调用这个函数后，如果发送消息列队中有消息就位，则会发送消息 ，如果不能发送消息，相关的错误代码将返回。\n
             	程序运行期间，中断是被失能的。
				
* Return:		RC_SUCCESS         信息发送成功
				RC_CANNOTTRANSMIT  系统没有发送消息,没有要发送的消息,或错误的CAN设备
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
UINT8 J1939_TransmitMessages(void)
{

	switch (Can_Node)
	{
		case  Select_CAN_NODE_1:
			{
			    if (TXQueueCount_1 == 0)
			    {
			        //如果没有要发送的消息从发送消息列队中，恢复中断(清空发送标识位)
					#if J1939_POLL_ECAN == J1939_FALSE
			        	Port_TXinterruptEnable();
					#endif
					
			        return RC_CANNOTTRANSMIT;
			    }
			    else
			    {
			        while(TXQueueCount_1 > 0)
			        {
			            /*确保上次数据发送成功*/
			            /**************可增加一个判断函数**************************/
			        	TXQueue_1[TXHead_1].Mxe.SourceAddress = NodeAddress_1;

			            SendOneMessage((J1939_MESSAGE *) &(TXQueue_1[TXHead_1]));
			            TXHead_1++;
						
			            if (TXHead_1 >= J1939_TX_QUEUE_SIZE)
			            {
			            	TXHead_1 = 0;
			            }
			            
			            TXQueueCount_1--;
			        }

			       	/*配置了一些标识位，使能中断*/
					#if J1939_POLL_ECAN == J1939_FALSE
			        	Port_TXinterruptEnable();
					#endif
			    }	
			}
			break;
		
		case  Select_CAN_NODE_2:
			{
				if (TXQueueCount_2 == 0)
				{
					//如果没有要发送的消息从发送消息列队中，恢复中断(清空发送标识位)
					#if J1939_POLL_ECAN == J1939_FALSE
						Port_TXinterruptEnable();
					#endif
					
					return RC_CANNOTTRANSMIT;
				}
				else
				{

					while(TXQueueCount_2 > 0)
					{
						/*确保上次数据发送成功*/
						/**************可增加一个判断函数**************************/

						TXQueue_2[TXHead_2].Mxe.SourceAddress = NodeAddress_2;

						SendOneMessage( (J1939_MESSAGE *) &(TXQueue_2[TXHead_2]) );
						TXHead_2++;
						if (TXHead_2 >= J1939_TX_QUEUE_SIZE)
						{
							TXHead_2 = 0;
						}					
						TXQueueCount_2--;
					}

				   	/*配置了一些标识位，使能中断*/
					#if J1939_POLL_ECAN == J1939_FALSE
						Port_TXinterruptEnable();
					#endif
				}
			}
			break;
		
		case  Select_CAN_NODE_3:
			{
				if (TXQueueCount_3 == 0)
				{
					//如果没有要发送的消息从发送消息列队中，恢复中断(清空发送标识位)
					#if J1939_POLL_ECAN == J1939_FALSE
						Port_TXinterruptEnable();
					#endif
					
					return RC_CANNOTTRANSMIT;
				}
				else
				{
					while(TXQueueCount_3 > 0)
					{
						/*确保上次数据发送成功*/
						/**************可增加一个判断函数**************************/

						TXQueue_3[TXHead_3].Mxe.SourceAddress = NodeAddress_3;

						SendOneMessage( (J1939_MESSAGE *) &(TXQueue_3[TXHead_3]) );
						TXHead_3++;
						if (TXHead_3 >= J1939_TX_QUEUE_SIZE)
						{
							TXHead_3 = 0;
						}
						TXQueueCount_3--;
					}

				   	/*配置了一些标识位，使能中断*/
					#if J1939_POLL_ECAN == J1939_FALSE
						Port_TXinterruptEnable();
					#endif
				}
			}
			break;
		
		case  Select_CAN_NODE_4:
			{
				if (TXQueueCount_4 == 0)
				{
					//如果没有要发送的消息从发送消息列队中，恢复中断(清空发送标识位)
					#if J1939_POLL_ECAN == J1939_FALSE
						Port_TXinterruptEnable();
					#endif
					
					return RC_CANNOTTRANSMIT;
				}
				else
				{

					while(TXQueueCount_4 > 0)
					{
						/*确保上次数据发送成功*/
						/**************可增加一个判断函数**************************/

						TXQueue_4[TXHead_4].Mxe.SourceAddress = NodeAddress_4;

						SendOneMessage( (J1939_MESSAGE *) &(TXQueue_4[TXHead_4]) );
						TXHead_4++;
						if (TXHead_4 >= J1939_TX_QUEUE_SIZE)
						{
							TXHead_4 = 0;
						}
						TXQueueCount_4--;
					}

				   /*配置了一些标识位，使能中断*/
					#if J1939_POLL_ECAN == J1939_FALSE
						Port_TXinterruptEnable();
					#endif
				}	
			}
			break;
		
		default  :
		{
			return RC_CANNOTTRANSMIT;	
		}

	}

    return RC_SUCCESS;   
}


/********************************************************************
* Function: 	J1939_TP_DT_Packet_send
* Description:	
* Parameter:	void
* note:			发送TP.DT，参考J1939-21
				
* Return:		No Return
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
#if J1939_TP_RX_TX
void J1939_TP_DT_Packet_send(void)
{
	J1939_MESSAGE _msg;
	UINT16 _packet_offset_p;
	INT32 _i = 0;
	
	_msg.Mxe.Priority = J1939_TP_DT_PRIORITY;
	_msg.Mxe.DataPage = 0;
	_msg.Mxe.PDUFormat = J1939_PF_DT;
	_msg.Mxe.DestinationAddress = TP_TX_MSG.tp_tx_msg.SA;
	_msg.Mxe.DataLength = 8;


    /*获取请求发送的数据包数量*/
    if(TP_TX_MSG.packets_request_num > 0)
    {
    	TP_TX_MSG.packets_request_num--;
		
    	/*获取数据偏移指针*/
    	_packet_offset_p = (UINT16)(TP_TX_MSG.packet_offset_p * 7u);
		
    	/*加载数据包编号*/
    	_msg.Mxe.Data[0] = (UINT8)(1u + TP_TX_MSG.packet_offset_p);

		/*数据放入data字段*/
        for(_i = 0; _i <7; _i++)
        {
        	_msg.Mxe.Data[_i+1] = TP_TX_MSG.tp_tx_msg.data[_packet_offset_p + _i];
        }
		
        /*是否是最后一包数据消息*/
        if(TP_TX_MSG.packet_offset_p == (TP_TX_MSG.packets_total - 1u))
        {
        	/*参数群是否能被填满，是否需要填充，*/
            if ( _packet_offset_p > TP_TX_MSG.tp_tx_msg.byte_count - 7 )
            {
            	/*计算需要填充的数据数*/
            	_i = (TP_TX_MSG.tp_tx_msg.byte_count - _packet_offset_p - 7);

                for (    ; _i < 0; _i++ )
                {
                	/*我们默认J1939的参数群大小为8*/
                	_msg.Mxe.Data[_i+8] = J1939_RESERVED_BYTE;
                }
            }

            TP_TX_MSG.packets_request_num = 0;
            TP_TX_MSG.packet_offset_p = 0;
            TP_TX_MSG.time = J1939_TP_T3;
			
            /* 跳转步骤，等待结束确认或则重新发送数据请求*/
            TP_TX_MSG.state = J1939_TP_WAIT_ACK;
        }
        else
        {
        	/*为下一个数据发送做准备*/
        	TP_TX_MSG.packet_offset_p++;
        	TP_TX_MSG.state = J1939_TP_TX_DT;
        }

        /*可能队列已满，发不出去，但是这里不能靠返回值进行无限的死等*/
        J1939_EnqueueMessage(&_msg, Can_Node);
    }
    else
    {
    	TP_TX_MSG.packets_request_num = 0;
    	TP_TX_MSG.packet_offset_p = 0;
    	TP_TX_MSG.time = J1939_TP_T3;
    	TP_TX_MSG.state = J1939_TP_WAIT_ACK;
    }
}


/********************************************************************
* Function: 	J1939_CM_Start
* Description:	
* Parameter:	void
* note:			发送TP。CM-RTS,16,23,4,255,PGN消息，参考J1939-21，
				
* Return:		No Return
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
void J1939_CM_Start(void)
{
	UINT32 pgn_num;
	J1939_MESSAGE _msg;

    pgn_num = TP_TX_MSG.tp_tx_msg.PGN;

    _msg.Mxe.Priority = J1939_TP_CM_PRIORITY;
    _msg.Mxe.DataPage =0;
    _msg.Mxe.PDUFormat = J1939_PF_TP_CM;
    _msg.Mxe.DestinationAddress = TP_TX_MSG.tp_tx_msg.SA;
    _msg.Mxe.DataLength = 8;
    _msg.Mxe.Data[0] = J1939_RTS_CONTROL_BYTE;
    _msg.Mxe.Data[1] = (UINT8) TP_TX_MSG.tp_tx_msg.byte_count ;
    _msg.Mxe.Data[2] = (UINT8) ((TP_TX_MSG.tp_tx_msg.byte_count)>>8);
    _msg.Mxe.Data[3] = TP_TX_MSG.packets_total;
    _msg.Mxe.Data[4] = J1939_RESERVED_BYTE;
    _msg.Mxe.Data[7] = (UINT8)((pgn_num >> 16) & 0xff);
    _msg.Mxe.Data[6] = (UINT8)((pgn_num >> 8) & 0xff);
    _msg.Mxe.Data[5] = (UINT8)(pgn_num & 0xff);

    /*可能队列已满，发不出去，但是这里不能靠返回值进行无限的死等*/
    J1939_EnqueueMessage(&_msg, Can_Node);

	/*刷新等待时间，触发下一个步骤（）*/
    TP_TX_MSG.time = J1939_TP_T3;
    TP_TX_MSG.state = J1939_TP_TX_CM_WAIT;

}

/********************************************************************
* Function: 	J1939_TP_TX_Abort
* Description:	
* Parameter:	void
* note:			中断TP链接
				
* Return:		No Return
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
void J1939_TP_TX_Abort(void)
{
	J1939_MESSAGE _msg;
	UINT32 pgn_num;

	pgn_num = TP_TX_MSG.tp_tx_msg.PGN;

	_msg.Mxe.Priority = J1939_TP_CM_PRIORITY;
	_msg.Mxe.DataPage = 0;
	_msg.Mxe.PDUFormat = J1939_PF_TP_CM;
	_msg.Mxe.DestinationAddress = TP_TX_MSG.tp_tx_msg.SA;
	_msg.Mxe.DataLength = 8;
	_msg.Mxe.Data[0] = J1939_CONNABORT_CONTROL_BYTE;
	_msg.Mxe.Data[1] = J1939_RESERVED_BYTE;
	_msg.Mxe.Data[2] = J1939_RESERVED_BYTE;
	_msg.Mxe.Data[3] = J1939_RESERVED_BYTE;
	_msg.Mxe.Data[4] = J1939_RESERVED_BYTE;
	_msg.Mxe.Data[7] = (UINT8)((pgn_num >> 16) & 0xff);
	_msg.Mxe.Data[6] = (UINT8)((pgn_num >> 8) & 0xff);
	_msg.Mxe.Data[5] = (UINT8)(pgn_num & 0xff);

    /*可能队列已满，发不出去，但是这里不能靠返回值进行无限的死等*/
    J1939_EnqueueMessage(&_msg, Can_Node);
	
	/*结束发送*/
    TP_TX_MSG.state = J1939_TX_DONE;

}

/********************************************************************
* Function: 	J1939_TP_RX_Abort
* Description:	
* Parameter:	void
* note: 		中断TP链接
				
* Return:		No Return
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
void J1939_TP_RX_Abort(void)
{
	J1939_MESSAGE _msg;
	UINT32 pgn_num;

	pgn_num = TP_RX_MSG.tp_rx_msg.PGN;

	_msg.Mxe.Priority = J1939_TP_CM_PRIORITY;
	_msg.Mxe.DataPage =0;
	_msg.Mxe.PDUFormat = J1939_PF_TP_CM;
	_msg.Mxe.DestinationAddress = TP_RX_MSG.tp_rx_msg.SA;
	_msg.Mxe.DataLength = 8;
	_msg.Mxe.Data[0] = J1939_CONNABORT_CONTROL_BYTE;
	_msg.Mxe.Data[1] = J1939_RESERVED_BYTE;
	_msg.Mxe.Data[2] = J1939_RESERVED_BYTE;
	_msg.Mxe.Data[3] = J1939_RESERVED_BYTE;
	_msg.Mxe.Data[4] = J1939_RESERVED_BYTE;
	_msg.Mxe.Data[7] = (UINT8)((pgn_num >> 16) & 0xff);
	_msg.Mxe.Data[6] = (UINT8)((pgn_num >> 8) & 0xff);
	_msg.Mxe.Data[5] = (UINT8)(pgn_num & 0xff);

    /*可能队列已满，发不出去，但是这里不能靠返回值进行无限的死等*/
    J1939_EnqueueMessage(&_msg, Can_Node);
	
	/*结束发送*/
    TP_RX_MSG.state = J1939_RX_DONE;

}


/********************************************************************
* Function: 	J1939_TP_TX_RefreshCMTimer
* Description:	
* Parameter:	usMS
* note: 		TP的计时器
				
* Return:		UINT8
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
UINT8 J1939_TP_TX_RefreshCMTimer(UINT16 usMS)
{
	if((J1939_TP_TX_CM_WAIT == TP_TX_MSG.state)||(J1939_TP_WAIT_ACK == TP_TX_MSG.state))
	{
		if(TP_TX_MSG.time > usMS)
		{
			TP_TX_MSG.time = (TP_TX_MSG.time - usMS);
			
			return J1939_TP_TIMEOUT_NORMAL;
		}
		else
		{
			/*超时 */
			TP_TX_MSG.time = 0u;
			
			return  J1939_TP_TIMEOUT_ABNORMAL;
		}
	}
	else
	{
		return  J1939_TP_TIMEOUT_NORMAL;
	}
}


/********************************************************************
* Function: 	J1939_TP_RX_RefreshCMTimer
* Description:	
* Parameter:	usMS
* note: 		TP的计时器
				
* Return:		UINT8
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
UINT8 J1939_TP_RX_RefreshCMTimer(UINT16 usMS)
{
	if((J1939_TP_RX_DATA_WAIT == TP_RX_MSG.state))
	{
		if(TP_RX_MSG.time > usMS)
		{
			TP_RX_MSG.time = TP_RX_MSG.time - usMS;
			
			return J1939_TP_TIMEOUT_NORMAL;
		}
		else
		{
			/*超时 */
			TP_RX_MSG.time = 0u;
			
			return  J1939_TP_TIMEOUT_ABNORMAL;
		}

	}
	else
	{
		return  J1939_TP_TIMEOUT_NORMAL;
	}
}


/********************************************************************
* Function: 	J1939_read_DT_Packet
* Description:	
* Parameter:	void
* note: 		发送读取数据 TP.CM_CTS 和 EndofMsgAck消息。
				
* Return:		void
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
void J1939_read_DT_Packet(void)
{
	J1939_MESSAGE _msg;
	UINT32 pgn_num;
	pgn_num = TP_RX_MSG.tp_rx_msg.PGN;

	_msg.Mxe.Priority = J1939_TP_CM_PRIORITY;
	_msg.Mxe.DataPage =0;
	_msg.Mxe.PDUFormat = J1939_PF_TP_CM;
	_msg.Mxe.DestinationAddress = TP_RX_MSG.tp_rx_msg.SA;
	_msg.Mxe.DataLength = 8;

	/*如果系统繁忙,保持链接但是不传送消息*/
	if(TP_RX_MSG.osbusy)
	{
		_msg.Mxe.Data[0] = J1939_CTS_CONTROL_BYTE;
		_msg.Mxe.Data[1] = 0;
		_msg.Mxe.Data[2] = J1939_RESERVED_BYTE;
		_msg.Mxe.Data[3] = J1939_RESERVED_BYTE;
		_msg.Mxe.Data[4] = J1939_RESERVED_BYTE;
		_msg.Mxe.Data[7] = (UINT8)((pgn_num >> 16) & 0xff);
		_msg.Mxe.Data[6] = (UINT8)((pgn_num >> 8) & 0xff);
		_msg.Mxe.Data[5] = (UINT8)(pgn_num & 0xff);
		
        /*可能队列已满，发不出去，但是这里不能靠返回值进行无限的死等*/
        J1939_EnqueueMessage(&_msg, Can_Node);
		
		return ;
	}
	
	if(TP_RX_MSG.packets_total > TP_RX_MSG.packets_ok_num)
	{
		/*最后一次响应，如果不足2包数据*/
		if((TP_RX_MSG.packets_total - TP_RX_MSG.packets_ok_num) == 1)
		{
			_msg.Mxe.Data[0] = J1939_CTS_CONTROL_BYTE;
			_msg.Mxe.Data[1] = 1;
			_msg.Mxe.Data[2] = TP_RX_MSG.packets_total;
			_msg.Mxe.Data[3] = J1939_RESERVED_BYTE;
			_msg.Mxe.Data[4] = J1939_RESERVED_BYTE;
			_msg.Mxe.Data[7] = (UINT8)((pgn_num >> 16) & 0xff);
			_msg.Mxe.Data[6] = (UINT8)((pgn_num >> 8) & 0xff);
			_msg.Mxe.Data[5] = (UINT8)(pgn_num & 0xff);
			
            /*可能队列已满，发不出去，但是这里不能靠返回值进行无限的死等*/
            J1939_EnqueueMessage(&_msg, Can_Node);
			TP_RX_MSG.state = J1939_TP_RX_DATA_WAIT;
			
			return ;
		}
		_msg.Mxe.Data[0] = J1939_CTS_CONTROL_BYTE;
		_msg.Mxe.Data[1] = 2;
		_msg.Mxe.Data[2] = (TP_RX_MSG.packets_ok_num + 1);
		_msg.Mxe.Data[3] = J1939_RESERVED_BYTE;
		_msg.Mxe.Data[4] = J1939_RESERVED_BYTE;
		_msg.Mxe.Data[7] = (UINT8)((pgn_num >> 16) & 0xff);
		_msg.Mxe.Data[6] = (UINT8)((pgn_num >> 8) & 0xff);
		_msg.Mxe.Data[5] = (UINT8)(pgn_num & 0xff);

        /*可能队列已满，发不出去，但是这里不能靠返回值进行无限的死等*/
        J1939_EnqueueMessage(&_msg, Can_Node);
		TP_RX_MSG.state = J1939_TP_RX_DATA_WAIT;
		
		return ;
	}
	else
	{
		/*发送传输正常结束消息，EndofMsgAck*/
		_msg.Mxe.Data[0] = J1939_EOMACK_CONTROL_BYTE;
		_msg.Mxe.Data[1] = (TP_RX_MSG.tp_rx_msg.byte_count & 0x00ff);
		_msg.Mxe.Data[2] = ((TP_RX_MSG.tp_rx_msg.byte_count >> 8) & 0x00ff);
		_msg.Mxe.Data[3] = TP_RX_MSG.packets_total;
		_msg.Mxe.Data[4] = J1939_RESERVED_BYTE;
		_msg.Mxe.Data[7] = (UINT8)((pgn_num >> 16) & 0xff);
		_msg.Mxe.Data[6] = (UINT8)((pgn_num >> 8) & 0xff);
		_msg.Mxe.Data[5] = (UINT8)(pgn_num & 0xff);
		
        /*可能队列已满，发不出去，但是这里不能靠返回值进行无限的死等*/
        J1939_EnqueueMessage(&_msg, Can_Node);
		TP_RX_MSG.state = J1939_RX_DONE;
		
		return ;
	}
}


/********************************************************************
* Function: 	J1939_TP_Poll
* Description:	
* Parameter:	void
* note: 		TP协议的心跳，为了满足在总线的计时准确，10ms轮询一次   J1939_TP_TX_RefreshCMTimer(10)
				如果想要更高的分辨率，1ms轮询一次，但是要改下面计时函数  J1939_TP_TX_RefreshCMTimer(1)
* Return:		void
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
void J1939_TP_Poll(void)
{
	if(J1939_TP_Flags_t.state == J1939_TP_NULL || J1939_TP_Flags_t.state == J1939_TP_OSBUSY)
	{
		return ;
	}
	
	if(J1939_TP_Flags_t.state == J1939_TP_RX)
	{
		Can_Node = J1939_TP_Flags_t.TP_RX_CAN_NODE;
		
		switch(TP_RX_MSG.state)
		{
			case J1939_TP_RX_WAIT:
				{
				
				}
	    		break;
			
			case J1939_TP_RX_READ_DATA:
				{
					/*发送读取数据 TP.CM_CTS 和 EndofMsgAck消息*/
					J1939_read_DT_Packet();
				}	
				break;
			
			case J1939_TP_RX_DATA_WAIT:
				{
					/*等待TP.DT帧传输的消息*/
					if(J1939_TP_TIMEOUT_ABNORMAL == J1939_TP_RX_RefreshCMTimer(10))
					{
						/* 等待超时，发生连接异常，跳转到异常步骤 */
						TP_RX_MSG.state = J1939_TP_RX_ERROR;
					}
				}
				
				break;
			case J1939_TP_RX_ERROR:
				{
					J1939_TP_RX_Abort();
					J1939_TP_Flags_t.TP_RX_CAN_NODE = Select_CAN_NODE_Null;
				}
				break;
			
			case J1939_RX_DONE:
				{
					TP_RX_MSG.packets_ok_num = 0;
					TP_RX_MSG.packets_total = 0;
					TP_RX_MSG.time = J1939_TP_T3;
					TP_RX_MSG.state = J1939_TP_RX_WAIT;
					J1939_TP_Flags_t.state = J1939_TP_NULL;
				}
				break;
			
	        default:
	        	{
	        	
	        	}
	            break;
		}
			
		return ;
	}
	
	if(J1939_TP_Flags_t.state == J1939_TP_TX)
	{
		Can_Node = J1939_TP_Flags_t.TP_TX_CAN_NODE;
		switch (TP_TX_MSG.state)
		{
			case J1939_TP_TX_WAIT:
				{
					/*没有要发送的数据*/
				}
	    		break;
			
			case J1939_TP_TX_CM_START:
				{
					/*发送TP.CM_RTS帧传输的消息(参考j1939-21)*/
					J1939_CM_Start();		
				}
				break;
			
			case J1939_TP_TX_CM_WAIT:
				{
					/*等待TP.CM_CTS帧传输的消息*/
					if(J1939_TP_TIMEOUT_ABNORMAL == J1939_TP_TX_RefreshCMTimer(10))
					{
						/* 等待超时，发生连接异常，跳转到异常步骤 */
						TP_TX_MSG.state = J1939_TP_TX_ERROR;
					}
				}
				break;
			
			case J1939_TP_TX_DT:
				{
					/*拼包拆包发送*/
					J1939_TP_DT_Packet_send();
				}
	    		break;
			
	        case J1939_TP_WAIT_ACK:
	        	{
	        		/*等待TP.EndofMsgACK帧传输的消息*/
					if(J1939_TP_TIMEOUT_ABNORMAL == J1939_TP_TX_RefreshCMTimer(10))
					{
						/* 等待超时，发生连接异常，跳转到异常步骤 */
						TP_TX_MSG.state = J1939_TP_TX_ERROR;
					}
	        	}
	            break;
			
			case J1939_TP_TX_ERROR:
				{
					J1939_TP_TX_Abort();
				}		
	    		break;
			
			case J1939_TX_DONE:
				{
					TP_TX_MSG.packets_request_num = 0;
					TP_TX_MSG.packet_offset_p = 0;
					TP_TX_MSG.time = J1939_TP_T3;
					TP_TX_MSG.state = J1939_TP_TX_WAIT;
					J1939_TP_Flags_t.state = J1939_TP_NULL;
				}
	    		break;
			
	        default:
	        	{
	        		//程序不会运行到这里来，可以增加一个调试输出
	        	}
	            break;
			
		}
		
		return ;
	}
}


/********************************************************************
* Function: 	J1939_TP_TX_Message
* Description:	TP协议的发送函数,非阻塞io接口
* Parameter:	
				[in] uwPGN	TP会话的参数群编号
				[in] SA		TP会话的目标地址
				[in] *data	TP会话的数据缓存地址
				[in] data_num TP会话的数据大小
				[in] canNode  要入队的CAN硬件编号（要选择的使用的CAN硬件编号）
				
* note: 		TP协议的心跳，为了满足在总线的计时准确，10ms轮询一次   J1939_TP_TX_RefreshCMTimer(10)\n
				如果想要更高的分辨率，1ms轮询一次，但是要改下面计时函数  J1939_TP_TX_RefreshCMTimer(1)
* Return:		
				RC_SUCCESS        成功打开TP链接，开始进入发送流程
				RC_CANNOTTRANSMIT 不能发送，因为TP协议已经建立虚拟链接，并且未断开

* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
INT8 J1939_TP_TX_Message(UINT32 uwPGN, UINT8 ucDesAddr, UINT8 *pData, UINT16 usDataNum, CAN_NODE canNode)
{
	UINT16 _byte_count = 0;
	
	/*取得发送权限*/
	if(J1939_TP_Flags_t.state == J1939_TP_NULL)
	{
		J1939_TP_Flags_t.state = J1939_TP_TX;
		J1939_TP_Flags_t.TP_TX_CAN_NODE = canNode;
	}
	else
	{
		return RC_CANNOTTRANSMIT;//不能发送，因为TP协议已经建立虚拟链接，并且未断开
	}

	TP_TX_MSG.tp_tx_msg.PGN = uwPGN;
	TP_TX_MSG.tp_tx_msg.SA = ucDesAddr;
	TP_TX_MSG.tp_tx_msg.byte_count = usDataNum;
	
	for(_byte_count = 0; _byte_count < usDataNum; _byte_count++)
	{
		TP_TX_MSG.tp_tx_msg.data[_byte_count] = pData[_byte_count];
	}
	
	TP_TX_MSG.packet_offset_p = 0;
	TP_TX_MSG.packets_request_num = 0;
	TP_TX_MSG.packets_total = (usDataNum / 7);
	
	if((usDataNum % 7) != 0)
	{
		TP_TX_MSG.packets_total ++;
	}
	
	TP_TX_MSG.time = J1939_TP_T3;
	
	//触发开始CM_START
	TP_TX_MSG.state = J1939_TP_TX_CM_START;

	return RC_SUCCESS;
}



/********************************************************************
* Function: 	J1939_TP_RX_Message
* Description:	TP协议的发送函数,非阻塞io接口
* Parameter:	
				[in]  msg.data	     读取数据的缓存
				[in]  msg.data_num   读取数据的缓存大小
				[in]  canNode      要入队的CAN硬件编号（要选择的使用的CAN硬件编号）
				[out] msg.SA         数据源地址
				[out] msg.byte_count 数据大小
				[out] msg.PGN        数据参数群编号
			
* note: 		TP的接受函数 , 接受缓存的大小必须大于接受数据的大小，建议初始化缓存大小用  J1939_TP_MAX_MESSAGE_LENGTH\n
				请正确带入 缓存区的大小，参数错误程序运行有风险
* Return:		
				RC_CANNOTRECEIVE 不能接受，TP协议正在接受数据中
				RC_SUCCESS		读取数据成功
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
INT8 J1939_TP_RX_Message(TP_RX_MESSAGE *pstMsg , CAN_NODE canNode)
{
	UINT16 _a = 0;
	
	/*判断是否能读取数据*/
	if(J1939_TP_Flags_t.state == J1939_TP_NULL && TP_RX_MSG.tp_rx_msg.PGN != 0)
	{
		J1939_TP_Flags_t.state = J1939_TP_OSBUSY;
	}
	else
	{
		return RC_CANNOTRECEIVE;//不能接受，TP协议正在接受数据中,或没有数据
	}
	
	//判断是不是要读取那一路CAN数据
	if(canNode != J1939_TP_Flags_t.TP_RX_CAN_NODE)
	{
        /*释放TP接管权限*/
        if(J1939_TP_Flags_t.state == J1939_TP_OSBUSY)
        {
            J1939_TP_Flags_t.state = J1939_TP_NULL;
        }
		
		return RC_CANNOTRECEIVE;
	}
	
    //判断数据缓存够不够
    if((pstMsg->data_num) < TP_RX_MSG.tp_rx_msg.byte_count)
	{
		return RC_CANNOTRECEIVE;//不能接受，缓存区太小
	}

    /*获取数据*/
    for(_a = 0; _a < (pstMsg->data_num); _a++)
	{
        pstMsg->data[_a] = TP_RX_MSG.tp_rx_msg.data[_a];
	}
	
    /*获取数据 源地址*/
    pstMsg->SA  =  TP_RX_MSG.tp_rx_msg.SA;
	
    /*获取数据的大小*/
    pstMsg->byte_count  =  TP_RX_MSG.tp_rx_msg.byte_count;
	
    /*获取数据PGN*/
    pstMsg->PGN  =  TP_RX_MSG.tp_rx_msg.PGN;
	

    /*丢弃读取过的数据*/
	TP_RX_MSG.tp_rx_msg.byte_count= 0u;
	TP_RX_MSG.tp_rx_msg.PGN = 0;

	/*释放TP接管权限*/
	if(J1939_TP_Flags_t.state == J1939_TP_OSBUSY)
	{
		J1939_TP_Flags_t.state = J1939_TP_NULL;
	}

	return RC_SUCCESS;
}


/********************************************************************
* Function: 	J1939_Request_PGN
* Description:	TP协议的发送函数,非阻塞io接口
* Parameter:	
				[in] uwPGN  被请求的参数群
				[in] DA   目标地址（DestinationAddress） 当DA = 0xff表示是全局请求			
				[in] canNode  要入队的CAN硬件编号（要选择的使用的CAN硬件编号）
				
* note: 		请求（从全局范围或则特定目的地的）参数群，请求规则J1939-21的16-17页，有明确的说明
* Return:		
				No Return
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
void J1939_Request_PGN(UINT32 uwPGN ,UINT8 ucDesAddr, CAN_NODE canNode)
{
	J1939_MESSAGE _msg;

	_msg.Mxe.DataPage                = 0;
	_msg.Mxe.Priority                = J1939_REQUEST_PRIORITY;
	_msg.Mxe.DestinationAddress      = ucDesAddr;
	_msg.Mxe.DataLength              = 3;
	_msg.Mxe.PDUFormat               = J1939_PF_REQUEST;
	_msg.Mxe.Data[0]        		 = (UINT8)(uwPGN & 0x000000FF);
	_msg.Mxe.Data[1]        		 = (UINT8)((uwPGN & 0x0000FF00) >> 8);
	_msg.Mxe.Data[2]				 = (UINT8)((uwPGN & 0x00FF0000) >> 16);

	while (J1939_EnqueueMessage( &_msg, canNode) != RC_SUCCESS);
}


/********************************************************************
* Function: 	J1939_Create_Response
* Description:	
* Parameter:	
				[in]  data	      需要发送数据的缓存
				[in]  dataLenght  发送数据的缓存大小
				[in]  uwPGN         需要发送数据的PGN(参数群编号)
				[in]  void (*dataUPFun)()  用于更新缓存data 的函数地址指针
				[in]  canNode      要入队的CAN硬件编号（要选择的使用的CAN硬件编号）
				
* note: 		创建一个PGN 的 请求 对应的 响应\n 如果收到改请求则先运行 REQUEST_LIST.dataUPFun(),在将数据REQUEST_LIST.data发送出去
* warning  		本函数只能被串行调用，（多线程）并行调用请在函数外加互斥操作
* Return:		
				No Return
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
void J1939_Create_Response(UINT8 ucData[], UINT16 usDataLen, UINT32 uwPGN, void (*dataUPFun)(), CAN_NODE canNode)
{
	/*查找可用的链表项*/
	struct Request_List * _requestList = &REQUEST_LIST;
	while(NULL != _requestList->next)
	{
		_requestList = _requestList->next;
	}
	_requestList->next = (struct Request_List *)malloc(sizeof(struct Request_List));
	_requestList = _requestList->next;

	/*对新的链表项赋值*/
	_requestList->data = ucData;
	_requestList->lenght = usDataLen;
	_requestList->PGN = uwPGN;
	_requestList->update = dataUPFun;
    _requestList->Can_Node = canNode;
	_requestList->next = NULL;

}


/********************************************************************
* Function: 	J1939_Response
* Description:	
* Parameter:	uwPGN	
* note: 		收到一个PGN请求后，如果有REQUEST_LIST中有相应的PGN，则会自动发送REQUEST_LIST中的PGN。\n
  				如果没有则会发送一个NACK; 本函数的响应逻辑，参考J1939-21 17页表4
* warning  		
* Return:		
				No Return
* History:
*a	 08/21/2019 09:44:08	WangLin  Created
********************************************************************/
void J1939_Response(const UINT32 uwPGN)
{
	UINT8 _i = 0;
	J1939_MESSAGE _msg;

	/*查找可用的链表项*/
	struct Request_List *_requestList = &REQUEST_LIST;
	
	while((uwPGN != _requestList->PGN) || (Can_Node != _requestList->Can_Node))
	{
		if(_requestList->next == NULL)
		{
			/*原文档规定 全局请求不被支持时不能响应 NACK*/
			if(OneMessage.Mxe.PDUSpecific == J1939_GLOBAL_ADDRESS)
			{
				return;
			}
			
			if((uwPGN & 0xFF00) >= 0xF000)
			{
				return;
			}

			/*没有相应的PGN响应被创建，向总线发送一个NACK*/
			_msg.Mxe.Priority            = J1939_ACK_PRIORITY;
			_msg.Mxe.DataPage            = 0;
			_msg.Mxe.PDUFormat           = J1939_PF_ACKNOWLEDGMENT;
			_msg.Mxe.DestinationAddress  = OneMessage.Mxe.SourceAddress;
			_msg.Mxe.DataLength          = 8;
			_msg.Mxe.SourceAddress		 = J1939_Address;
			_msg.Mxe.Data[0]         = J1939_NACK_CONTROL_BYTE;
			_msg.Mxe.Data[1]         = 0xFF;
			_msg.Mxe.Data[2]         = 0xFF;
			_msg.Mxe.Data[3]         = 0xFF;
			_msg.Mxe.Data[4]         = 0xFF;
			_msg.Mxe.Data[5]         = (uwPGN & 0x0000FF);
			_msg.Mxe.Data[6]         = ((uwPGN >> 8) & 0x0000FF);
			_msg.Mxe.Data[7]         = ((uwPGN >> 16) & 0x0000FF);

			SendOneMessage( (J1939_MESSAGE *) &_msg);
			
			return ;
		}
		else
		{
			_requestList = _requestList->next;
		}
	}

	/*调用dataUPFun（）函数，主要用于参数群数据更新*/
	if(NULL != _requestList->update)
	{
		_requestList->update();
	}

	/*响应请求*/
	if(_requestList->lenght > 8)
	{
		/*回一个确认响应多帧(非广播多帧)*/
		if(RC_SUCCESS != J1939_TP_TX_Message(_requestList->PGN, OneMessage.Mxe.SourceAddress, _requestList->data, _requestList->lenght, Can_Node))
		{
			/*原文档规定 全局请求不被支持时不能响应 NACK*/
			if(OneMessage.Mxe.PDUSpecific == J1939_GLOBAL_ADDRESS)
			{
				return;
			}

			/*如果长帧发送不成功*/
			_msg.Mxe.Priority            = J1939_ACK_PRIORITY;
			_msg.Mxe.DataPage            = 0;
			_msg.Mxe.PDUFormat           = J1939_PF_ACKNOWLEDGMENT;
			_msg.Mxe.DestinationAddress  = OneMessage.Mxe.SourceAddress;
			_msg.Mxe.DataLength          = 8;
			_msg.Mxe.SourceAddress		 = J1939_Address;
			_msg.Mxe.Data[0]         = J1939_ACCESS_DENIED_CONTROL_BYTE;
			_msg.Mxe.Data[1]         = 0xFF;
			_msg.Mxe.Data[2]         = 0xFF;
			_msg.Mxe.Data[3]         = 0xFF;
			_msg.Mxe.Data[4]         = 0xFF;
			_msg.Mxe.Data[5]         = (uwPGN & 0x0000FF);
			_msg.Mxe.Data[6]         = ((uwPGN >> 8) & 0x0000FF);
			_msg.Mxe.Data[7]         = ((uwPGN >> 16) & 0x0000FF);

			SendOneMessage( (J1939_MESSAGE *) &_msg);
			
			return ;
		}

		/*回一个确认响应*/
		_msg.Mxe.Priority            = J1939_ACK_PRIORITY;
		_msg.Mxe.DataPage            = 0;
		_msg.Mxe.PDUFormat           = J1939_PF_ACKNOWLEDGMENT;
		/*原文档规定 全局请求响应到全局*/
		if(OneMessage.Mxe.PDUSpecific == J1939_GLOBAL_ADDRESS)
		{
			_msg.Mxe.DestinationAddress  = 0xFF;
		}
		else
		{
			_msg.Mxe.DestinationAddress  = OneMessage.Mxe.SourceAddress;
		}
		
		_msg.Mxe.DataLength          = 8;
		_msg.Mxe.SourceAddress		 = J1939_Address;
		_msg.Mxe.Data[0]         = J1939_ACK_CONTROL_BYTE;
		_msg.Mxe.Data[1]         = 0xFF;
		_msg.Mxe.Data[2]         = 0xFF;
		_msg.Mxe.Data[3]         = 0xFF;
		_msg.Mxe.Data[4]         = 0xFF;
		_msg.Mxe.Data[5]         = (uwPGN & 0x0000FF);
		_msg.Mxe.Data[6]         = ((uwPGN >> 8) & 0x0000FF);
		_msg.Mxe.Data[7]         = ((uwPGN >> 16) & 0x0000FF);
		
		SendOneMessage( (J1939_MESSAGE *) &_msg);
	}
	else
	{

		/*回一个确认响应*/
		_msg.Mxe.Priority            = J1939_ACK_PRIORITY;
		_msg.Mxe.DataPage            = 0;
		_msg.Mxe.PDUFormat           = J1939_PF_ACKNOWLEDGMENT;
		_msg.Mxe.SourceAddress		 = J1939_Address;
		
		/*原文档规定 全局请求响应到全局*/
		if((OneMessage.Mxe.PDUSpecific == J1939_GLOBAL_ADDRESS) || ((uwPGN & 0xFF00) >= 0xF000))
		{
			_msg.Mxe.DestinationAddress  = 0xFF;
		}
		else
		{
			_msg.Mxe.DestinationAddress  = OneMessage.Mxe.SourceAddress;
		}
		
		_msg.Mxe.DataLength          = 8;
		_msg.Mxe.SourceAddress		 = J1939_Address;
		_msg.Mxe.Data[0]         = J1939_ACK_CONTROL_BYTE;
		_msg.Mxe.Data[1]         = 0xFF;
		_msg.Mxe.Data[2]         = 0xFF;
		_msg.Mxe.Data[3]         = 0xFF;
		_msg.Mxe.Data[4]         = 0xFF;
		_msg.Mxe.Data[5]         = (uwPGN & 0x0000FF);
		_msg.Mxe.Data[6]         = ((uwPGN >> 8) & 0x0000FF);
		_msg.Mxe.Data[7]         = ((uwPGN >> 16) & 0x0000FF);
		
		SendOneMessage( (J1939_MESSAGE *) &_msg);

		/*回一个确认响应单帧*/
		_msg.Mxe.Priority            = J1939_ACK_PRIORITY;
		_msg.Mxe.DataPage            = (((_requestList->PGN) >> 16) & 0x1);
		_msg.Mxe.PDUFormat           = ((_requestList->PGN) >> 8) & 0xFF;
		_msg.Mxe.SourceAddress		 = J1939_Address;
		
		/*原文档规定 全局请求响应到全局*/
		if(OneMessage.Mxe.PDUSpecific == J1939_GLOBAL_ADDRESS)
		{
			_msg.Mxe.DestinationAddress  = 0xFF;
		}
		else
		{
			_msg.Mxe.DestinationAddress  = OneMessage.Mxe.SourceAddress;
		}
		
		_msg.Mxe.DataLength = _requestList->lenght;
				
		for(_i = 0; _i < (_requestList->lenght); _i++)
		{
			_msg.Mxe.Data[_i] = _requestList->data[_i];
		}
		for(; _i < 8; _i++)
		{
			_msg.Mxe.Data[_i] = 0xFF;
		}
		
		SendOneMessage( (J1939_MESSAGE *) &_msg);
	}
}
#endif

