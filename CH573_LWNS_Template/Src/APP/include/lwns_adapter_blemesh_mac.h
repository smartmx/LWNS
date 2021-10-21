/*
 * lwns_adapter_blemesh_mac.h
 *
 *  Created on: Jul 19, 2021
 *      Author: WCH
 */

#ifndef _LWNS_ADAPTER_BLEMESH_MAC_H_
#define _LWNS_ADAPTER_BLEMESH_MAC_H_

#ifdef __cplusplus
extern "C"
{
#endif


#define LWNS_USE_BLEMESH_MAC            0   //是否使能模仿blemesh的mac协议，注意只能使能一个mac层协议。

#if LWNS_USE_BLEMESH_MAC

#include "WCH_LWNS_LIB.h"

struct blemesh_mac_phy_manage_struct{
    struct blemesh_mac_phy_manage_struct* next;
    u8* data;
};//模仿blemesh mac层发送管理结构体

#define LWNS_ENCRYPT_ENABLE               1   //是否使能加密

#define LWNS_ADDR_USE_BLE_MAC             1  //是否使用蓝牙硬件的mac地址作为库地址

#define LWNS_NEIGHBOR_MAX_NUM             8   //最大邻居数量

#define LWNS_MAC_TRANSMIT_TIMES           2   //一次发送，调用硬件发送几次

#define LWNS_MAC_PERIOD_MS                10  //mac接收周期，轮流切换

#define LWNS_MAC_SEND_DELAY_MAX_MS        10  //蓝牙mesh就是10ms以内的随机数。

#define LWNS_MAC_SEND_PACKET_MAX_NUM      8   //发送链表最多支持几个数据包等待发送

#define BLE_PHY_ONE_PACKET_MAX_625US      5

#define LLE_MODE_ORIGINAL_RX                        (0x80)  //如果配置LLEMODE时加上此宏，则接收第一字节为原始数据（原来为RSSI）

extern void RF_Init( void );

void lwns_init(void);

#define LWNS_HTIMER_PERIOD_MS             20 //为(1000/HTIMER_SECOND_NUM)

//RF_TX和RF_RX所用的类型，可以修改，不推荐改
#define USER_RF_RX_TX_TYPE 0xff

//receive process evt
#define LWNS_PHY_RX_OPEN_EVT              1
#define LWNS_PHY_RX_CHANGE_CHANNEL_EVT    2
//send process evt
#define LWNS_HTIMER_PERIOD_EVT            1
#define LWNS_PHY_OUTPUT_PREPARE_EVT       2
#define LWNS_PHY_OUTPUT_FINISH_EVT        4

#define LWNS_PHY_OUTPUT_TIMEOUT_MS        5

#ifdef __cplusplus
}
#endif

#endif  /* LWNS_USE_BLEMESH_MAC */

#endif /* _LWNS_ADAPTER_BLEMESH_MAC_H_ */
