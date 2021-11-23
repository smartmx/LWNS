/*
 * lwns_prov.h
 *
 *  Created on: Sep 28, 2021
 *      Author: WCH
 */

#ifndef _LWNS_PROV_H_
#define _LWNS_PROV_H_

#include "lwns_config.h"
#include "rf_config_params.h"
#include "lwns_sec.h"

#define PROV_BC_PORT 1  //配网广播通道端口
#define PROV_UC_PORT 2  //配网单播通道端口

#define PROV_MASTER 0 //是否为主机设备，为0即为从机设备
#define BLE_PROV    1 //是否使能使用蓝牙配网



#define PROV_BROADCAST_TX_PERIOD_EVT                 1<<(0)

void prov_process_init(void);

void prov_process_deinit(void);

uint8 lwns_provisioned(void);

EfErrCode lwns_set_provisioned(void);

EfErrCode lwns_provisioned_reset(void);

typedef enum {
    LWNS_PROV_CMD_TEST = 0,               //通信测试，发什么回复什么
    LWNS_PROV_CMD_GET_PROV_STATE,         //获取配网状态
    LWNS_PROV_CMD_GET_RF_CONFIG_PARAMS,   //获取当前rf参数
    LWNS_PROV_CMD_GET_RF_KEY,             //获取当前rf加密秘钥
    LWNS_PROV_CMD_SET_RF_CONFIG_PARAMS,   //设置rf配网参数
    LWNS_PROV_CMD_SET_RF_KEY,             //设置rf配网加密秘钥
    LWNS_PROV_CMD_PROV_ENABLE,            //设置配网状态，才会将之前设置的配网参数存入设备
    LWNS_PROV_CMD_PROV_RESET,
    LWNS_PROV_CMD_MAX,
} LWNS_PROV_CMD_t;


extern void Rec_BLE_PROV_DataDeal(unsigned char *p_data, unsigned char w_len ,unsigned char *r_data, unsigned char* r_len);

#endif /* _LWNS_PROV_H_ */
