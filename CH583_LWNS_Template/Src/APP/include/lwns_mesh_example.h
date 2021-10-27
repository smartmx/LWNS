/*
 * lwns_mesh_example.h
 *
 *  Created on: Jul 28, 2021
 *      Author: WCH
 */

#ifndef _LWNS_MESH_EXAMPLE_H_
#define _LWNS_MESH_EXAMPLE_H_

#include "WCH_LWNS_LIB.h"


#define MESH_EXAMPLE_TX_PERIOD_EVT                 1<<(0)
#define MESH_EXAMPLE_TX_NODE_EVT                     1<<(1)
void lwns_mesh_process_init(void);


#endif /* _LWNS_MESH_EXAMPLE_H_ */
