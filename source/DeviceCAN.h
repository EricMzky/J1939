/********************************************************************
*XXXXXX System Development XXXXX.XXXX Software
*Copyright (c) 2019-2029, XXX LTD., All rights reserved.
*File Name:   DeviceCAN.h
*File Description: Implementation of  DeviceCAN.h
*Modification History:
*a	 08/22/2019 15:43:08  WangLin	Created
********************************************************************/

#ifndef _DEVICE_CAN_H_INCLUDE_
#define _DEVICE_CAN_H_INCLUDE_

#include "TypeDefineBase.h"


#ifdef __cplusplus
extern "C" {
#endif
#pragma  pack()

/********************************DEFINE*****************************/
/*CAN过滤器标识符及掩码宏*/
#define CAN_FILT_IDE_DST_ID(x)	(x << 23)
#define CAN_FILT_MASK_DST_ID	(0xF << 23)

#define ECU_DEV_CAN_NUM (2U)
#define ECU_DEV_CAN0 (0U)
#define ECU_DEV_CAN1 (1U)
#define ECU_CAN_BITRATE_MIN (1U)
#define ECU_CAN_BITRATE_MAX (1000000U)


#define ECU_CAN_NODE_ID_OFFSET (20U)
#define ECU_CAN_EFF_FLAG (1) /* EFF/SFF is set in the MSB */
#define ECU_CAN_SFF_FLAG (0) /* EFF/SFF is set in the MSB */
#define ECU_CAN_RTR_FLAG (0x40000000U>>30) /* remote transmission request */
#define ECU_CAN_ERR_FLAG (0x20000000U>>29) /* error message frame */

#define CAN_FRAME_DATA_MAX (8U)
#define CAN_FRAME_END_OK (1U)
#define CAN_FRAME_END_NO (0U)

/*****can.h*****/
#define CAN_MAIL_BOX_0 (0U)
#define CAN_MAIL_BOX_1 (1U)
#define CAN_MAIL_BOX_2 (2U)

#define CAN_IDE_STD (0U)
#define CAN_IDE_ETD (1U)


/****************************Define ON/OFF*********************************/
#define CAN_FILTER_REG FALSE




/********************************ENUM******************************
typedef enum _XXXX_XX_ENUM
{
	xx,
	xx,
	
} XXXX_XX_ENUM;*/



/********************************STRUCT*****************************/
typedef struct _CanFrameHead_t
{
	UINT8 EndFlag:1;
	UINT8 SubIndex:7;

	UINT8 MsgIndex;

	UINT8 SrcID:4;
	UINT8 DstID:4;

	UINT8 MsgType:5;
	UINT8 ERF:1;
	UINT8 RTR:1;//
	UINT8 FF:1;//

	UINT8 DataLen /*__attribute__((aligned(4)))*/;
}CanFrameHead_t	;/*兼容socket CAN*/


typedef struct _CanFrame_t
{
	CanFrameHead_t MsgHead;
	UINT8 MsgData[CAN_FRAME_DATA_MAX] /*__attribute__((aligned(8)))*/;
}CanFrame_t;

/*以上结构体后期需要重定义*/

typedef struct _CanFilter_t
{
	UINT32 can_id;
	UINT32 can_mask;
}CanFilter_t;

typedef struct _CanFiFoCache
{
	UINT32 can_reg_rir;
	UINT32 can_reg_rdtr;
	UINT32 can_reg_rdlr;
	UINT32 can_reg_rdhr;
}CanFiFoCache;




/********************************DECLAR*****************************/
/*void*/


/*BOOL*/
BOOL ECU_CanMsgQueueInit(void);
BOOL ECU_Can1Init(void);
BOOL ECU_CanRead(UINT8 canIndex,UINT8 *pFrame);
BOOL ECU_CanWrite(UINT8 canIndex,UINT8 *pFrame);


/*UINT8*/


/*UINT16*/


/*UINT32*/


#pragma  pack()
#ifdef __cplusplus
}
#endif

#endif /*!defined(_DEVICE_CAN_H_INCLUDE_)*/

