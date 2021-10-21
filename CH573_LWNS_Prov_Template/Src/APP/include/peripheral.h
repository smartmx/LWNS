/********************************** (C) COPYRIGHT *******************************
* File Name          : peripheral.h
* Author             : WCH
* Version            : V1.0
* Date               : 2018/12/11
* Description        : 
            
*******************************************************************************/

#ifndef _PERIPHERAL_H
#define _PERIPHERAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */


// Simple BLE Peripheral Task Events
#define SBP_START_DEVICE_EVT                              0x0001

	
	

/*********************************************************************
 * MACROS
 */
typedef struct
{
  uint16 connHandle;           // Connection handle of current connection
  uint16 connInterval;
  uint16 connSlaveLatency;
  uint16 connTimeout;
} peripheralConnItem_t;
/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the BLE Application
 */
extern void Peripheral_Init(void);

/*
 * Task Event Processor for the BLE Application
 */
extern uint16 Peripheral_ProcessEvent( uint8 task_id, uint16 events );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif 
