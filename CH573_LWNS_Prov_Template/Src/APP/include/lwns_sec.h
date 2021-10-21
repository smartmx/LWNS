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

int lwns_msg_encrypt(u8* src,u8 * to,u8 mlen);

int lwns_msg_decrypt(u8* src,u8 * to,u8 mlen);

void lwns_temp_set_key(u8* keyValue);

EfErrCode lwns_save_key_to_flash(void);

void lwns_sec_init(void);

void lwns_get_key(u8* to);

#endif /* _LWNS_SEC_H_ */
