/********************************** (C) COPYRIGHT *******************************
* File Name          : GATTprofile.h
* Author             : WCH
* Version            : V1.0
* Date               : 2018/12/11
* Description        : 
            
*******************************************************************************/

#ifndef _RWPROFILE_H_
#define _RWPROFILE_H_

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

// Profile Parameters
#define rwProfile_CHAR1                   0  // RW uint8 - Profile Characteristic 1 value
#define rwProfile_CHAR2                   1  // RW uint8 - Profile Characteristic 2 value
#define rwProfile_CHAR3                   2  // RW uint8 - Profile Characteristic 3 value

  
// Simple Profile Service UUID
#define rwProfile_SERV_UUID               0xFDE0
    
// Key Pressed UUID
#define rwProfile_CHAR1_UUID            0xFDE1
#define rwProfile_CHAR2_UUID            0xFDE2
#define rwProfile_CHAR3_UUID            0xFDE3

  
// Simple Keys Profile Services bit fields
#define rwProfile_SERVICE               0x00000001
extern const char rwReadbuf1[5];
extern const char rwReadbuf2[5];
/*********************************************************************
 * TYPEDEFS
 */

  
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * Profile Callbacks
 */


// 读写操作函数回调
typedef void (*rwProfileRead_t)( unsigned char paramID );
typedef void (*rwProfileWrite_t)( unsigned char paramID ,unsigned char *p_data, unsigned char w_len);
// Callback when a characteristic value has changed
typedef void (*rwProfileChange_t)( uint8 paramID );

typedef struct
{
  rwProfileRead_t           pfnrwProfileRead;
  rwProfileWrite_t          pfnrwProfileWrite;
  rwProfileChange_t         pfnrwProfileChange;  // Called when characteristic value changes
} rwProfileCBs_t;



/*********************************************************************
 * API FUNCTIONS 
 */


/*
 * rwProfile_AddService- Initializes the Simple GATT Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 */

extern bStatus_t rwProfile_AddService( uint32 services );

/*
 * rwProfile_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t rwProfile_RegisterAppCBs( rwProfileCBs_t *appCallbacks );


bStatus_t rwProfile_Notify(uint16 connHandle, attHandleValueNoti_t *pNoti);

bStatus_t rwProfile_SendData(unsigned char paramID ,unsigned char *p_data, unsigned char send_len );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
