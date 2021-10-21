/*
 * lwns_adapter_no_mac.h
 *
 *  Created on: Jul 19, 2021
 *      Author: WCH
 */

#ifndef _LWNS_ADAPTER_NO_MAC_H_
#define _LWNS_ADAPTER_NO_MAC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define LWNS_USE_NO_MAC  0 //是否使能纯透传mac协议，适合不存在并发情况的星型网络，即主机问从机答的星型网络。

#if LWNS_USE_NO_MAC

#include "WCH_LWNS_LIB.h"

typedef enum {
    BLE_PHY_MANAGE_STATE_FREE = 0,
    BLE_PHY_MANAGE_STATE_SENDING,
} BLE_PHY_MANAGE_STATE_t;

#define LWNS_ENCRYPT_ENABLE               1   //是否使能加密

#define LWNS_ADDR_USE_BLE_MAC             1  //是否使用蓝牙硬件的mac地址作为库地址

#define LWNS_NEIGHBOR_MAX_NUM             1   //最大邻居数量，null mac不使用邻居表

#define LLE_MODE_ORIGINAL_RX                        (0x80)  //如果配置LLEMODE时加上此宏，则接收第一字节为原始数据（原来为RSSI）

extern void RF_Init( void );

void lwns_init(void);

#define LWNS_HTIMER_PERIOD_MS             20//为(1000/HTIMER_SECOND_NUM)


//RF_TX和RF_RX所用的类型，可以修改，不推荐改
#define USER_RF_RX_TX_TYPE 0xff


//receive process evt
#define LWNS_PHY_RX_OPEN_EVT          1
//send process evt
#define LWNS_HTIMER_PERIOD_EVT        1
#define LWNS_PHY_OUTPUT_FINISH_EVT    2

#define LWNS_PHY_OUTPUT_TIMEOUT_MS        5



#endif  /* LWNS_USE_NO_MAC */

#ifdef __cplusplus
}
#endif

#endif /* _LWNS_ADAPTER_NO_MAC_H_ */
