/*
 * lwns_config.h
 *
 *  Created on: Nov 1, 2021
 *      Author: WCH
 */

#ifndef _LWNS_CONFIG_H_
#define _LWNS_CONFIG_H_

#include "config.h"
#include "CH57x_common.h"
#include "WCH_LWNS_LIB.h"

#define LWNS_ADDR_USE_BLE_MAC           1       //是否使用蓝牙硬件的mac地址作为默认lwns地址

#define LWNS_ENCRYPT_ENABLE             1       //是否使能加密

#define QBUF_MANUAL_NUM                 4       //qbuf缓存数量配置

#define ROUTE_ENTRY_MANUAL_NUM          32      //路由条目数量配置

#define LWNS_NEIGHBOR_MAX_NUM           8       //最大邻居数量

#endif /* _LWNS_CONFIG_H_ */
