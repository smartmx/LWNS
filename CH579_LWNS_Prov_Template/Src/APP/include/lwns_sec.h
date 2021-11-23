/*
 * lwns_sec.h
 *
 *  Created on: Sep 17, 2021
 *      Author: WCH
 */

#ifndef _LWNS_SEC_H_
#define _LWNS_SEC_H_

#include "WCH_LWNS_LIB.h"
#include "easyflash.h"

int lwns_msg_encrypt(uint8_t *src,uint8_t *to,uint8_t mlen);

int lwns_msg_decrypt(uint8_t *src,uint8_t *to,uint8_t mlen);

void lwns_temp_set_key(uint8_t *keyValue);

EfErrCode lwns_save_key_to_flash(void);

void lwns_sec_init(void);

void lwns_get_key(uint8_t *to);

#endif /* _LWNS_SEC_H_ */
