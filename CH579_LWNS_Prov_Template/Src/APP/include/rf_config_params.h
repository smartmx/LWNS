/*
 * rf_config_params.h
 *
 *  Created on: Sep 27, 2021
 *      Author: WCH
 */

#ifndef _RF_CONFIG_PARAMS_H_
#define _RF_CONFIG_PARAMS_H_

#include "config.h"
#include "easyflash.h"

typedef struct  _rf_config_params_struct
{
    uint8_t  Channel[3];                              // rf channel(0-39)
    uint8_t  channelNum;
  u32 accessAddress;                        // access address,32bit PHY address
  u32 CRCInit;                              // crc initial value
}rf_config_params_t;

extern rf_config_params_t lwns_rf_params;

extern const char lwns_rf_params_index[];

extern void rf_config_params_init(void);

extern EfErrCode rf_config_params_save_to_flash(void);

#endif /* _RF_CONFIG_PARAMS_H_ */
