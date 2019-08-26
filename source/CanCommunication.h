/********************************************************************
*XXXXXX System Development XXXXX.XXXX Software
*Copyright (c) 2019-2029, XXX LTD., All rights reserved.
*File Name:   XXXXX.h
*File Description: Implementation of  CanCommunication.h
*Modification History:
*a	 08/22/2019 15:43:08  WangLin	Created
********************************************************************/

#ifndef _J1939_CONFIG_H_INCLUDE_
#define _J1939_CONFIG_H_INCLUDE_ 


#include "J1939.h"
#include "DeviceCAN.h"
#include "TypeDefineBase.h"

#ifdef __cplusplus
extern "C" {
#endif
#pragma  pack()



extern CAN_NODE   Can_Node;   //CAN硬件选择




/***************************J1939 地址配置*****************************/
//设备默认的地址（地址命名是有规定的，参考J1939的附录B 地址和标识符的分配）
#define J1939_STARTING_ADDRESS_1 0x01
#define J1939_STARTING_ADDRESS_2 244
#define J1939_STARTING_ADDRESS_3 247
#define J1939_STARTING_ADDRESS_4 0


/******************************J1939功能配置***************************/
#define J1939_RX_QUEUE_SIZE 3
//当mcu来不及处理消息，接收消息列队是否允许被新的消息覆盖
#define J1939_OVERWRITE_RX_QUEUE J1939_FALSE
#define J1939_TX_QUEUE_SIZE 3
//当mcu来不及处理消息，发送消息列队是否允许被新的消息覆盖
#define J1939_OVERWRITE_TX_QUEUE J1939_FALSE
//是否使用轮询模式（否则使用中断模式）
#define J1939_POLL_ECAN J1939_TRUE
//是否启用软件滤波器
#define J1939SoftwareFilterEn J1939_TRUE
/******************************J1939移植配置函数************************/

#define Port_CAN_Transmit(pstMsg) J1939_CAN_Transmit(pstMsg)
#define Port_CAN_Receive(pstMsg) J1939_CAN_Receive(pstMsg)
#define Port_SetAddressFilter(Address) J1939_SetAddressFilter(Address)


/*不使用中断模式，不对下面的函数进行移植*/
#if J1939_POLL_ECAN == J1939_FALSE
	#define Port_RXinterruptEnable() J1939_RXinterruptEnable() 
	#define Port_RXinterruptDisable() J1939_RXinterruptDisable() 
	#define Port_TXinterruptEnable() J1939_TXinterruptEnable() 
	#define Port_TXinterruptDisable() J1939_TXinterruptDisable() 
	#define Port_TXinterruptOk() J1939_TXinterruptOk()
	#define Port_CAN_identifier_clc()  CAN_identifier_clc()
#endif 





void J1939_SetAddressFilter(unsigned char Ps_Address);
void J1939_CAN_Transmit(J1939_MESSAGE *pstMsg);
int J1939_CAN_Receive(J1939_MESSAGE *pstMsg);
void ECU_CAN_ReceiveMsg(J1939_MESSAGE *pstMsg);






#pragma  pack()
#ifdef __cplusplus
}
#endif

#endif /*!defined(_J1939_Config_H_)*/

