/********************************** (C) COPYRIGHT *******************************
 * File Name          : lwns_broadcast_example.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/06/20
 * Description        : broadcast，广播程序例子
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/
#ifndef _LWNS_BROADCAST_EXAMPLE_H_
#define _LWNS_BROADCAST_EXAMPLE_H_

#include "lwns_config.h"

#define BROADCAST_EXAMPLE_TX_PERIOD_EVT    1 << (0)

void lwns_broadcast_process_init(void);

#endif /* LWNS_BROADCAST_EXAMPLE_H_ */
