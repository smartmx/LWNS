/*
 * lwns_addr_manage.h
 *
 *  Created on: Oct 15, 2021
 *      Author: WCH
 */

#ifndef _LWNS_ADDR_MANAGE_H_
#define _LWNS_ADDR_MANAGE_H_

#include "config.h"
#include "easyflash.h"
#include "WCH_LWNS_LIB.h"

void lwns_addr_init(lwns_addr_t *to);

EfErrCode lwns_addr_save_to_flash(lwns_addr_t *from);


#endif /* _LWNS_ADDR_MANAGE_H_ */
