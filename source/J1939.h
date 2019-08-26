/********************************************************************
*XXXXXX System Development XXXXX.XXXX Software
*Copyright (c) 2019-2029, XXX LTD., All rights reserved.
*File Name:   DeviceCAN.h
*File Description: Implementation of  J1939.h
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
	*a	2019/08/19  15:43:08 WangLin	Created
********************************************************************/

#ifndef __J1939_H_INCLUDE_ 
#define __J1939_H_INCLUDE_

#include "TypeDefineBase.h"

#ifdef __cplusplus
extern "C" {
#endif
#pragma  pack()


/******************************类型声明*********************************/

//函数返回代码
#define RC_SUCCESS			    0  /**< 成功*/
#define RC_QUEUEEMPTY			1  /**< 列队为空*/
#define RC_QUEUEFULL			1  /**< 列队满*/
#define RC_CANNOTRECEIVE		2  /**< 不能接收*/
#define RC_CANNOTTRANSMIT		2  /**< 不能传输*/
#define RC_PARAMERROR			3  /**< 参数错误*/
 
//内部常量
#define J1939_FALSE				0  /**< 代表函数错误返回*/
#define J1939_TRUE				1  /**< 代表函数正确返回*/

// J1939 默认的优先级（参考J1939文档）
#define J1939_CONTROL_PRIORITY			0x03 /**< J1939文档默认的优先级*/
#define J1939_INFO_PRIORITY			    0x06 /**< J1939文档默认的优先级*/ 
#define J1939_PROPRIETARY_PRIORITY		0x06 /**< J1939文档默认的优先级*/ 
#define J1939_REQUEST_PRIORITY			0x06 /**< J1939文档默认的优先级*/ 
#define J1939_ACK_PRIORITY			    0x06 /**< J1939文档默认的优先级*/ 
#define J1939_TP_CM_PRIORITY			0x07 /**< J1939文档默认的优先级*/ 
#define J1939_TP_DT_PRIORITY			0x07 /**< J1939文档默认的优先级*/ 
 
// J1939 定义的地址
#define J1939_GLOBAL_ADDRESS			255 /**< 全局地址*/
#define NULL_ADDRESS			    254 /**< 空地址*/
 
//J1939协议栈的PNG请求响应，相关的定义
#define J1939_PF_REQUEST2				201  /**< J1939协议栈的请求 PF */
#define J1939_PF_TRANSFER				202  /**< J1939协议栈的转移 PF */

#define J1939_PF_REQUEST				234  /**< 请求 或 用于握手机制*/
#define J1939_PF_ACKNOWLEDGMENT			232  /**< 确认请求 或 用于握手机制*/

#define J1939_ACK_CONTROL_BYTE			0    /**< 用于TP(长帧数据)，代表确认*/
#define J1939_NACK_CONTROL_BYTE			1    /**< 用于TP(长帧数据)，PNG不被支持。否定消息*/
#define J1939_ACCESS_DENIED_CONTROL_BYTE	2/**< 拒绝访问，但是信息是被支持，暂时不能响应（需要再次发送请求）*/
#define J1939_CANNOT_RESPOND_CONTROL_BYTE	3/**< 不能做出反应，有空但是接受的缓存不够，或则发送资源被占领，暂时不能响应（需要再次发送请求）*/

//TP协议的一些宏定义 
#define J1939_PF_DT				        235	    /**< 协议传输---数据传输 PF*/
#define J1939_PF_TP_CM				    236		/**< 协议传输---链接管理 PF*/

//TP的超时时间，单位（ms）
#define J1939_TP_Tr						200 /**< 宏定义TP的超时时间*/
#define J1939_TP_Th						500 /**< 宏定义TP的超时时间*/
#define J1939_TP_T1						750 /**< 宏定义TP的超时时间*/
#define J1939_TP_T2						1250 /**< 宏定义TP的超时时间*/
#define J1939_TP_T3						1250 /**< 宏定义TP的超时时间*/
#define J1939_TP_T4						1050 /**< 宏定义TP的超时时间*/
#define J1939_TP_TIMEOUT_NORMAL			0    /**< 未超时正常*/
#define J1939_TP_TIMEOUT_ABNORMAL		1    /**< 超时*/
#define J1939_RTS_CONTROL_BYTE			16	 /**< TP.CM_RTS*/
#define J1939_CTS_CONTROL_BYTE			17	 /**< TP.CM_CTS*/
#define J1939_EOMACK_CONTROL_BYTE		19	 /**< 消息应答结束*/
#define J1939_BAM_CONTROL_BYTE			32	 /**< 广播公告消息*/
#define J1939_CONNABORT_CONTROL_BYTE	255	 /**< 连接中断控制字节（放弃连接）*/
#define J1939_RESERVED_BYTE             0xFF /**< 变量的保留位的值*/
 
//与J1939网络层有关的定义 
#define J1939_PGN2_REQ_ADDRESS_CLAIM	0x00 
#define J1939_PGN1_REQ_ADDRESS_CLAIM	0xEA   
#define J1939_PGN0_REQ_ADDRESS_CLAIM	0x00 
 
#define J1939_PGN2_COMMANDED_ADDRESS	0x00 
#define J1939_PGN1_COMMANDED_ADDRESS	0xFE	/**< 命令地址消息*/
#define J1939_PGN0_COMMANDED_ADDRESS	0xD8    /**< 参考J1939-81 地址命令配置*/
 
#define J1939_PF_ADDRESS_CLAIMED		238
#define J1939_PF_CANNOT_CLAIM_ADDRESS	238
#define J1939_PF_PROPRIETARY_A			239     /**< 专用A*/ 
#define J1939_PF_PROPRIETARY_B			255     /**< 专用B*/ 

/**< 是否对TP协议的支持（是否支持长帧（大于8字节的数据）的发送与接受）*/
#define J1939_TP_RX_TX J1939_TRUE
/**< TP协议的支持的最大接受发送消息长度(最大可配置为1785)*/
#define J1939_TP_MAX_MESSAGE_LENGTH 240

/**CAN节点的选择枚举
*
* 默认支持最大4路CAN硬件\n
*/
typedef enum
{
	Select_CAN_NODE_Null,	/**< 不选择任何CAN硬件*/ 
	Select_CAN_NODE_1,		/**< 选择CAN硬件 1*/ 
	Select_CAN_NODE_2,		/**< 选择CAN硬件 2*/
	Select_CAN_NODE_3,		/**< 选择CAN硬件 3*/
	Select_CAN_NODE_4,		/**< 选择CAN硬件 4*/
}CAN_NODE;

#if J1939_TP_RX_TX
/**TP的状态描述枚举
*
*/
typedef enum
{
	J1939_TP_NULL,  /**< 长数据传输处于空闲，只有TP系统处于空闲，才能用处理下一个发送，和接受请求*/ 
	J1939_TP_RX,	/**< 长数据传输处于接收*/
	J1939_TP_TX,	/**< 长数据传输处于发送*/
	J1939_TP_OSBUSY,/**< 长数据传输处于繁忙，比如刚接受一整段长数据，但是CPU没来得处理，又一个长数据请求到来，为了数据不被覆盖，将状态设为本值*/
}J1939_TP_State;
/**TP的标志位结构体
*
* 本结构体记录了TP的状态，使用TP发送和接受的CAN硬件编号
*/
typedef struct
{
	J1939_TP_State state;          /**< TP的连接状态*/
	CAN_NODE       TP_RX_CAN_NODE; /**< TP接受请求产生的 CAN硬件编号*/
	CAN_NODE       TP_TX_CAN_NODE; /**< TP接受发送产生的 CAN硬件编号*/
}J1939_TP_Flags;
/**J1939消息对象的结构体
*
* 本结构体实现了 J1939的消息对象
*/
typedef struct
{
	UINT32 PGN ;  /**< J1939的消息对象的 PGN*/
	UINT8  data[J1939_TP_MAX_MESSAGE_LENGTH] ;/**< J1939的消息对象的 数据*/
	UINT16 byte_count;/**< J1939的消息对象的 数据大小*/
	UINT8  SA;   /**< J1939的消息对象的 目标地址（发送目的地  或  接受来源地）*/

} J1939_MESSAGE_T ;
/**J1939消息对象的结构体
*
* 本结构体实现了 J1939的多帧消息对象
*/
typedef struct
{
    UINT8 *data;        /**< 缓存区指针*/
    UINT16 data_num;    /**< 缓存区大小*/
    UINT8 SA;           /**< J1939的消息对象的 数据 源地址*/
    UINT16 byte_count;  /**< J1939的消息对象的 数据大小*/
    UINT32 PGN ;        /**< J1939的消息对象的 PGN*/
}TP_RX_MESSAGE;
/**J1939_TP_Tx_Step枚举
*
* 实现了记录长帧（多帧）传输的TX 的步骤
*/
typedef enum
{
	J1939_TP_TX_WAIT,
	J1939_TP_TX_CM_START,
	J1939_TP_TX_CM_WAIT,
	J1939_TP_TX_DT,
	J1939_TP_WAIT_ACK,
	J1939_TP_TX_ERROR,
	J1939_TX_DONE,
}J1939_TP_Tx_Step;//协议的发送步骤
/**J1939_TRANSPORT_TX_INFO 结构体
*
* 实现了长帧传输中产生的临时数据，和一些传输交换数据
*/
typedef struct
{
	J1939_MESSAGE_T       tp_tx_msg;           /**< J1939的消息对象*/
	UINT16        time;                /**< 时间*/
	UINT8         packet_offset_p;     /**< 帧数据包偏移指针*/
	UINT8         packets_total;       /**< 总共有多少个帧数据包*/
	UINT8         packets_request_num; /**< 请求发送的帧数据包数（接受方准备接受的数据包数）*/
	J1939_TP_Tx_Step      state ;              /**< 协议的发送步骤*/
} J1939_TRANSPORT_TX_INFO;
/**J1939_TP_Rx_Step枚举
*
* 实现了记录长帧（多帧）传输的RX 的步骤
*/
typedef enum
{
	J1939_TP_RX_WAIT,
	J1939_TP_RX_READ_DATA,
	J1939_TP_RX_DATA_WAIT,
	J1939_TP_RX_ERROR,
	J1939_RX_DONE,
}J1939_TP_Rx_Step;//协议的接收步骤

/**J1939_TRANSPORT_RX_INFO 结构体
*
* 实现了长帧传输中产生的临时数据，和一些传输交换数据
*/
typedef struct
{
	J1939_MESSAGE_T    	 tp_rx_msg; /**< J1939的消息对象*/
	UINT8		 osbusy;    /**< 此位置1，代表系统繁忙，cpu需要处理其他的事物，直接拒绝一切的链接请求\n 如果正在接受中，此位置1，则会发出链接保持消息帧。*/
	UINT16    	 time;		/**< 时间*/
	UINT8        packets_total; /**< 总共有多少个数据包*/
	UINT8        packets_ok_num;/**< 已经接受的数据包数*/
	J1939_TP_Rx_Step     state ;     /**< 协议的接受步骤*/
} J1939_TRANSPORT_RX_INFO;

#endif //J1939_TP_RX_TX

/**
* @note 实现Request_PGN 的响应
*/
struct Request_List{
	UINT8  *data;
	UINT16 lenght;
	UINT32 PGN;
	CAN_NODE       Can_Node;
	void (*update)();  /**< 在函数里需要对data更新，如果不用更新data赋值为NULL*/
	struct Request_List *next;   /**< 链表末尾，需要一直保持NULL*/
};

// J1939 Data Structures 
// J1939_MESSAGE_STRUCT旨在J1939消息块映射到设备的地址映射。 只有字段PDU格式不映射到设备寄存器。
// 结构应该简单地使用PDUFormat和忽视PDUFormat_Top。调整将立即接收和传输之前。
// 注:编译器创建结构从低一点的位置高一些位置，所以可能出现不匹配的设备寄存器。
#define J1939_MSG_LENGTH	9  //消息长度
#define J1939_DATA_LENGTH	8  //数据长度



/** j1939 的 ID 组成结构体
*
*/	
struct j1939_PID
{ 
	UINT8	DataPage			: 1;  /**< 数据页*/
	UINT8	Res 				: 1;  /**< Res位*/
	UINT8	Priority			: 3;  /**< 优先级*/
	UINT8	Reserve 			: 3;  /**< 空闲*/
	UINT8	PDUFormat;				  /**< PF*/
	UINT8	PDUSpecific;			  /**< PS*/
	UINT8	SourceAddress;			  /**< SA*/
	UINT8	DataLength			: 4;  /**< 数据长度*/
	UINT8	RTR 				: 4;  /**< RTR位*/
	UINT8	Data[J1939_DATA_LENGTH];  /**< 数据*/
	UINT32	PGN 				:24;  /**< 参数群编号*/
	UINT32	ReservePGN			: 8;  /**< 空闲*/
};

/** J1939_MESSAGE_UNION 结构体
* 实现了J1939消息对象
*
*
*/
union J1939_MESSAGE_UNION 
{ 

	struct j1939_PID Mxe;  /**< j1939 的 ID 组成结构体*/
	UINT8		Array[J1939_MSG_LENGTH + J1939_DATA_LENGTH]; /**< 联合体数组，方便快速处理结构体赋值*/
};

#define GroupExtension 		PDUSpecific 
#define DestinationAddress 	PDUSpecific 
/** 一个宏定义，具体变量名称作用命名
*
*/
typedef union J1939_MESSAGE_UNION J1939_MESSAGE; 

#pragma anon_unions
union J1939_FLAGS_UNION
{ 
	struct 
	{ 
		UINT8	TransmitMessagesdCover				: 1;  //发送数据时，J1939协议接受缓存有数据覆盖
		UINT8	ReceivedMessagesdCoverOrDroppedNode	: 3;
		UINT8	ReceivedMessagesdCover				: 1;  //接受数据时，J1939协议接受缓存有数据覆盖
		UINT8	ReceivedMessagesDropped				: 1;  //接受数据时，J1939协议接受缓存有数据溢出
	}; 
	UINT8 FlagVal; 
}; 

typedef union J1939_FLAGS_UNION J1939_FLAG; 

/******************************wanglin******************************************/
typedef struct _CAN_QUEUE
{
	UINT8 Head;
	UINT8 Tail;
	UINT8 Count;
	J1939_MESSAGE Msg[3];	

}CAN_QUEUE;

typedef enum _can_node_id
{
	CAN_NODE_1 = 1,
	CAN_NODE_2 = 2,
	CAN_NODE_3 = 3,
	CAN_NODE_4 = 4,
	
	CAN_NODES = 4
}can_node_id;
/********************************************API**************************************************************/

//初始化函数
extern void 			J1939_Initialization( );
//CAN驱动收发中断入口
extern void			    J1939_ISR( );
//心跳函数,定时被调用
extern void 			J1939_Poll( );
//读取单帧消息
extern UINT8	J1939_Read_Message( J1939_MESSAGE *MsgPtr, CAN_NODE  _Can_Node);
//发送单帧消息
extern UINT8  	J1939_Send_Message( J1939_MESSAGE *MsgPtr, CAN_NODE  _Can_Node);
//多帧（多组）消息发送函数  (RTS/CTS传输协议)
extern INT8 	J1939_TP_TX_Message(UINT32 PGN, UINT8 DA, UINT8 *data, UINT16 data_num, CAN_NODE  _Can_Node);
//多帧（多组）消息接受函数  (RTS/CTS传输协议)
extern INT8 	J1939_TP_RX_Message(TP_RX_MESSAGE *msg, CAN_NODE _Can_Node);
//请求获去一个PGN
extern void             J1939_Request_PGN(UINT32 pgn ,UINT8 DA, CAN_NODE  _Can_Node);
//创建一个PGN响应
extern void             J1939_Create_Response(UINT8 data[], UINT16 dataLenght, UINT32 PGN, void (*dataUPFun)(), CAN_NODE  _Can_Node);


#pragma  pack()
#ifdef __cplusplus
}
#endif

#endif /*!defined(__J1939_H_INCLUDE_)*/

 
  