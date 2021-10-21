/********************************** (C) COPYRIGHT *******************************
 * File Name          : rwprofile.C
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2021/07/08
 * Description        :

 *******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "rwprofile.h"
#include "CH57x_common.h"

/*********************************************************************
 * MACROS
 */
// Position of Characteristic 3 value in attribute array
#define DATAEXCHANGEPROFILE_CHAR3_VALUE_POS            8
/*********************************************************************
 * CONSTANTS
 */

#define SERVAPP_NUM_ATTR_SUPPORTED        11

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// write and read buffer
static uint8 rwProfileChar1ReadLen = 0;
static uint8 rwProfileChar1ReadBuf[20] = {0};
static uint8 rwProfileChar1WriteLen = 0;
static uint8 rwProfileChar1WriteBuf[20] = {0};

static uint8 rwProfileChar2ReadLen = 0;
static uint8 rwProfileChar2ReadBuf[20] = {0};
static uint8 rwProfileChar2WriteLen = 0;
static uint8 rwProfileChar2WriteBuf[20] = {0};

// Simple GATT Profile Service UUID: 0xFFF0
CONST uint8 rwProfileServUUID[ATT_BT_UUID_SIZE] = { LO_UINT16(
        rwProfile_SERV_UUID), HI_UINT16(rwProfile_SERV_UUID) };

// Characteristic 1 UUID: 0xFFF1
CONST uint8 rwProfilechar1UUID[ATT_BT_UUID_SIZE] = { LO_UINT16(
        rwProfile_CHAR1_UUID), HI_UINT16(rwProfile_CHAR1_UUID) };

// Characteristic 2 UUID: 0xFFF2
CONST uint8 rwProfilechar2UUID[ATT_BT_UUID_SIZE] = { LO_UINT16(
        rwProfile_CHAR2_UUID), HI_UINT16(rwProfile_CHAR2_UUID) };

// Characteristic 3 UUID: 0xFFF3
CONST uint8 rwProfilechar3UUID[ATT_BT_UUID_SIZE] = { LO_UINT16(
        rwProfile_CHAR3_UUID), HI_UINT16(rwProfile_CHAR3_UUID) };

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static rwProfileCBs_t *rwProfile_AppCBs = NULL;


/*********************************************************************
 * Profile Attributes - variables
 */

// Simple Profile Service attribute
static CONST gattAttrType_t rwProfileService = { ATT_BT_UUID_SIZE,
        rwProfileServUUID };

// Simple Profile Characteristic 1 Properties
static uint8 rwProfileChar1Props = GATT_PROP_WRITE|GATT_PROP_READ;

// Characteristic 1 Value
static uint8 rwProfileChar1 = 0;

// Simple Profile Characteristic 1 User Description
static uint8 rwProfileChar1UserDesp[20] = "rw Port 1\0";

// Simple Profile Characteristic 2 Properties
static uint8 rwProfileChar2Props = GATT_PROP_WRITE|GATT_PROP_READ;

// Characteristic 2 Value
static uint8 rwProfileChar2 = 0;

// Simple Profile Characteristic 2 User Description
static uint8 rwProfileChar2UserDesp[23] = "rw Port 2\0";

// Simple Profile Characteristic 3 Properties
static uint8 rwProfileChar3Props = GATT_PROP_NOTIFY;

// Characteristic 3 Value
static uint8 rwProfileChar3 = 0;

// Simple Profile Characteristic 3 User Description
static uint8 rwProfileChar3UserDesp[19] = "rw Notice\0";
// Simple Profile Characteristic 4 Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t rwProfileChar3Config[4];

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t rwProfileAttrTbl[11] =
        {
        // Simple Profile Service
                { { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
                GATT_PERMIT_READ, /* permissions */
                0, /* handle */
                (uint8 *) &rwProfileService /* pValue */
                },

                // Characteristic 1 Declaration
                { { ATT_BT_UUID_SIZE, characterUUID },
                GATT_PERMIT_READ, 0, &rwProfileChar1Props },

                // Characteristic Value 1
                { { ATT_BT_UUID_SIZE, rwProfilechar1UUID },
                        GATT_PERMIT_READ|GATT_PERMIT_WRITE, 0, &rwProfileChar1 },

                // Characteristic 1 User Description
                { { ATT_BT_UUID_SIZE, charUserDescUUID },
                GATT_PERMIT_READ, 0, rwProfileChar1UserDesp },

                // Characteristic 2 Declaration
                { { ATT_BT_UUID_SIZE, characterUUID },
                GATT_PERMIT_READ, 0, &rwProfileChar2Props },

                // Characteristic Value 2
                { { ATT_BT_UUID_SIZE, rwProfilechar2UUID },
                        GATT_PERMIT_READ|GATT_PERMIT_WRITE, 0, &rwProfileChar2 },

                // Characteristic 2 User Description
                { { ATT_BT_UUID_SIZE, charUserDescUUID },
                GATT_PERMIT_READ, 0, rwProfileChar2UserDesp },

                // Characteristic 3 Declaration
                { { ATT_BT_UUID_SIZE, characterUUID },
                GATT_PERMIT_READ, 0, &rwProfileChar3Props },
                // Characteristic Value 3
                { { ATT_BT_UUID_SIZE, rwProfilechar3UUID }, 0, 0,
                        &rwProfileChar3 },

                // Characteristic 3 configuration
                { { ATT_BT_UUID_SIZE, clientCharCfgUUID },
                GATT_PERMIT_READ | GATT_PERMIT_WRITE, 0,
                        (uint8 *) rwProfileChar3Config },

                // Characteristic 3 User Description
                { { ATT_BT_UUID_SIZE, charUserDescUUID },
                GATT_PERMIT_READ, 0, rwProfileChar3UserDesp }, };

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t rwProfile_ReadAttrCB(uint16 connHandle,
        gattAttribute_t *pAttr, uint8 *pValue, uint16 *pLen, uint16 offset,
        uint16 maxLen, uint8 method);
static bStatus_t rwProfile_WriteAttrCB(uint16 connHandle,
        gattAttribute_t *pAttr, uint8 *pValue, uint16 len, uint16 offset,
        uint8 method);

static void rwProfile_HandleConnStatusCB(uint16 connHandle, uint8 changeType);

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
gattServiceCBs_t rwProfileCBs = { rwProfile_ReadAttrCB, // Read callback function pointer
        rwProfile_WriteAttrCB, // Write callback function pointer
        NULL                       // Authorization callback function pointer
        };

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      rwProfile_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t rwProfile_AddService(uint32 services) {
    uint8 status = SUCCESS;
    // Initialize Client Characteristic Configuration attributes
    GATTServApp_InitCharCfg( INVALID_CONNHANDLE, rwProfileChar3Config );
    // Register with Link DB to receive link status change callback
    linkDB_Register( rwProfile_HandleConnStatusCB );
    if (services & rwProfile_SERVICE) {
        // Register GATT attribute list and CBs with GATT Server App
        status = GATTServApp_RegisterService(rwProfileAttrTbl,
                GATT_NUM_ATTRS(rwProfileAttrTbl),
                GATT_MAX_ENCRYPT_KEY_SIZE, &rwProfileCBs);
    }

    return (status);
}

/*********************************************************************
 * @fn      rwProfile_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call 
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t rwProfile_RegisterAppCBs(rwProfileCBs_t *appCallbacks) {
    if (appCallbacks) {
        rwProfile_AppCBs = appCallbacks;

        return ( SUCCESS);
    } else {
        return ( bleAlreadyInRequestedMode);
    }
}

/*********************************************************************
 * @fn          rwProfile_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 *
 * @return      Success or Failure
 */
static bStatus_t rwProfile_ReadAttrCB(uint16 connHandle,
        gattAttribute_t *pAttr, uint8 *pValue, uint16 *pLen, uint16 offset,
        uint16 maxLen, uint8 method) {
    bStatus_t status = SUCCESS;
    if (pAttr->type.len == ATT_BT_UUID_SIZE) {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        switch (uuid) {
        case rwProfile_CHAR1_UUID: {
            *pLen = rwProfileChar1ReadLen;
            tmos_memcpy( pValue, rwProfileChar1ReadBuf, rwProfileChar1ReadLen );
            rwProfileChar1ReadLen = 0;
            if( rwProfile_AppCBs && rwProfile_AppCBs->pfnrwProfileRead )
            {
                rwProfile_AppCBs->pfnrwProfileRead(rwProfile_CHAR1);
            }
            break;
        }
        case rwProfile_CHAR2_UUID:{
            *pLen = rwProfileChar2ReadLen;
            tmos_memcpy( pValue, rwProfileChar2ReadBuf, rwProfileChar2ReadLen );
            rwProfileChar2ReadLen = 0;
            if( rwProfile_AppCBs && rwProfile_AppCBs->pfnrwProfileRead )
            {
                rwProfile_AppCBs->pfnrwProfileRead(rwProfile_CHAR2);
            }
            break;
        }
        case rwProfile_CHAR3_UUID:
            break;

        default:
            // Should never get here! (characteristics 3 and 4 do not have read permissions)
            *pLen = 0;
            status = ATT_ERR_ATTR_NOT_FOUND;
            break;
        }
    } else {
        // 128-bit UUID
        *pLen = 0;
        status = ATT_ERR_INVALID_HANDLE;
    }

    return (status);
}

/*********************************************************************
 * @fn      rwProfile_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 *
 * @return  Success or Failure
 */
static bStatus_t rwProfile_WriteAttrCB(uint16 connHandle,
        gattAttribute_t *pAttr, uint8 *pValue, uint16 len, uint16 offset,
        uint8 method) {
    bStatus_t status = SUCCESS;

    // If attribute permissions require authorization to write, return error
    if (gattPermitAuthorWrite(pAttr->permissions)) {
        // Insufficient authorization
        return ( ATT_ERR_INSUFFICIENT_AUTHOR);
    }

    if (pAttr->type.len == ATT_BT_UUID_SIZE) {
        // 16-bit UUID
        uint16 uuid = BUILD_UINT16(pAttr->type.uuid[0], pAttr->type.uuid[1]);
        switch (uuid) {
        case rwProfile_CHAR1_UUID: {
            //Write the value
            if ( status == SUCCESS )
            {
                uint8* p_rec_buf;
                if(len<=20){
                    p_rec_buf = pValue;
                    rwProfileChar1WriteLen = len;
                    tmos_memcpy(rwProfileChar1WriteBuf, pValue, len);
                    if(rwProfileChar1WriteLen && rwProfile_AppCBs && rwProfile_AppCBs->pfnrwProfileWrite)
                    {
                        rwProfile_AppCBs->pfnrwProfileWrite(rwProfile_CHAR1,rwProfileChar1WriteBuf,rwProfileChar1WriteLen);
                        rwProfileChar1WriteLen = 0;
                    }
                }
            }
            break;
        }
        case rwProfile_CHAR2_UUID: {

            break;
        }
        case GATT_CLIENT_CHAR_CFG_UUID:
          status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                   offset, GATT_CLIENT_CFG_NOTIFY );
          break;
        default:
            // Should never get here! (characteristics 2 and 4 do not have write permissions)
            status = ATT_ERR_ATTR_NOT_FOUND;
            break;
        }
    } else {
        // 128-bit UUID
        status = ATT_ERR_INVALID_HANDLE;
    }

    return (status);
}

/*********************************************************************
 * @fn          simpleProfile_Notify
 *
 * @brief       Send a notification containing a heart rate
 *              measurement.
 *
 * @param       connHandle - connection handle
 * @param       pNoti - pointer to notification structure
 *
 * @return      Success or Failure
 */
bStatus_t rwProfile_Notify(uint16 connHandle, attHandleValueNoti_t *pNoti) {
    uint16 value = GATTServApp_ReadCharCfg(connHandle, rwProfileChar3Config);

    // If notifications enabled
    if (value & GATT_CLIENT_CFG_NOTIFY) {
        // Set the handle
        pNoti->handle =
                rwProfileAttrTbl[DATAEXCHANGEPROFILE_CHAR3_VALUE_POS].handle;

        // Send the notification
        return GATT_Notification(connHandle, pNoti, FALSE);
    }
    return bleIncorrectMode;
}

/*******************************************************************************
* Function Name  : rwProfile_SendData
* Description    : rwProfile通道发送数据
* Input          : paramID：OTA通道选择
                   p_data：数据指针
                   send_len：发送数据长度
* Output         : None
* Return         : 函数执行状态
*******************************************************************************/
bStatus_t rwProfile_SendData(unsigned char paramID ,unsigned char *p_data, unsigned char send_len )
{
    bStatus_t status = SUCCESS;

    /* 数据长度超出范围 */
    if( send_len > 20 ){
        return 0xfe;
    }
    if(paramID == rwProfile_CHAR1){
        rwProfileChar1ReadLen = send_len;
        tmos_memcpy( rwProfileChar1ReadBuf, p_data, rwProfileChar1ReadLen );
    } else if(paramID == rwProfile_CHAR2){
        rwProfileChar2ReadLen = send_len;
        tmos_memcpy( rwProfileChar2ReadBuf, p_data, rwProfileChar2ReadLen );
    }
    return status;
}

/*********************************************************************
 * @fn          simpleProfile_HandleConnStatusCB
 *
 * @brief       Simple Profile link status change handler function.
 *
 * @param       connHandle - connection handle
 * @param       changeType - type of change
 *
 * @return      none
 */
static void rwProfile_HandleConnStatusCB( uint16 connHandle, uint8 changeType )
{
  // Make sure this is not loopback connection
  if ( connHandle != LOOPBACK_CONNHANDLE )
  {
    // Reset Client Char Config if connection has dropped
    if ( ( changeType == LINKDB_STATUS_UPDATE_REMOVED )      ||
         ( ( changeType == LINKDB_STATUS_UPDATE_STATEFLAGS ) &&
           ( !linkDB_Up( connHandle ) ) ) )
    {
      GATTServApp_InitCharCfg( connHandle, rwProfileChar3Config );
    }
  }
}

/*********************************************************************
 *********************************************************************/
