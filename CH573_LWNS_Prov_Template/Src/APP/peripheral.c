/********************************** (C) COPYRIGHT *******************************
* File Name          : Peripheral.C
* Author             : WCH
* Version            : V1.0
* Date               : 2018/12/10
* Description        : 外设从机应用程序，初始化广播连接参数，然后广播，直至连接主机后，通过自定义服务传输数据
            
*******************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "CONFIG.h"
#include "CH57x_common.h"
#include "devinfoservice.h"
#include "Peripheral.h"
#include "rwprofile.h"
#include "irq_manage.h"
#include "lwns_prov.h"

/*********************************************************************
 * MACROS
 */
#define DEBUG_PRINT_IN_THIS_FILE 1
#if DEBUG_PRINT_IN_THIS_FILE
#define PRINTF(...) PRINT(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif
/*********************************************************************
 * CONSTANTS
 */

// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD               1000

// What is the advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          320

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely

#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     40

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     400

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         TRUE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

// Company Identifier: WCH  
#define WCH_COMPANY_ID                        0x07D7

#define INVALID_CONNHANDLE                    0xFFFF

// Length of bd addr as a string
#define B_ADDR_STR_LEN                        15

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Connection item list
static peripheralConnItem_t peripheralConnList;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 Peripheral_TaskID=0xff;   // Task ID for internal task/event processing

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[31] = {
        // complete name
        0x0e,// length of this data
        GAP_ADTYPE_LOCAL_NAME_COMPLETE, 'l', 'w', 'n', 's', '-', 'n', 'o', 'd',
        'e', '-', '0', '0', '1',
        // connection interval range
        0x05,// length of this data
        GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
        LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),   // 100ms
        HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
        LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),   // 1s
        HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),
        // Tx power level
        0x02,// length of this data
        GAP_ADTYPE_POWER_LEVEL, 0       // 0dBm
        };




// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData[] =
{
  // Flags; this sets the device to use limited discoverable
  // mode (advertises for 30 seconds at a time) instead of general
  // discoverable mode (advertises indefinitely)
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

  // service UUID, to notify central devices what services are included
  // in this peripheral
  0x01,   // length of this data
  GAP_ADTYPE_16BIT_MORE,      // some of the UUID's, but not all
};

// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = {"lwns-node-001"};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void peripheralInitConnItem( peripheralConnItem_t* peripheralConnList );
static void peripheralStateNotify( uint8 *pValue, uint16 len );
static void Peripheral_LinkTerminated( gapRoleEvent_t * pEvent );
static void Peripheral_LinkEstablished( gapRoleEvent_t * pEvent );
static void peripheralParamUpdateCB( uint16 connHandle, uint16 connInterval,
                                      uint16 connSlaveLatency, uint16 connTimeout );
static void Peripheral_ProcessTMOSMsg( tmos_event_hdr_t *pMsg );
static void peripheralStateNotificationCB( gapRole_States_t newState ,gapRoleEvent_t * pEvent );
static void rwProfileChangeCB( uint8 paramID );
void rw_ProfileWriteData( unsigned char index, unsigned char *p_data, unsigned char w_len );
void rw_ProfileReadDataComplete( unsigned char index );
/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t Peripheral_PeripheralCBs =
{
    peripheralStateNotificationCB,  // Profile State Change Callbacks
    NULL,                            // When a valid RSSI is read from controller (not used by application)
    NULL
};

// GAP Bond Manager Callbacks
static gapBondCBs_t Peripheral_BondMgrCBs =
{
    NULL,                     // Passcode callback (not used by application)
    NULL                      // Pairing / Bonding state Callback (not used by application)
};


// Simple GATT Profile Callbacks
static rwProfileCBs_t Peripheral_rwProfileCBs =
{
        rw_ProfileReadDataComplete,
        rw_ProfileWriteData,
        rwProfileChangeCB    // Charactersitic value change callback
};

// Callback when the connection parameteres are updated.
void PeripheralParamUpdate(uint16 connInterval,uint16 connSlaveLatency, uint16 connTimeout);

gapRolesParamUpdateCB_t PeripheralParamUpdate_t = NULL;


/*********************************************************************
 * PUBLIC FUNCTIONS
 */



/*********************************************************************
 * @fn      Peripheral_Init
 *
 * @brief   Initialization function for the Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by TMOS.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void Peripheral_Init( )
{
  Peripheral_TaskID = TMOS_ProcessEventRegister(Peripheral_ProcessEvent);

  // Setup the GAP Peripheral Role Profile
  {
		// For other hardware platforms, device starts advertising upon initialization
     uint8 initial_advertising_enable = TRUE;

    // Set the GAP Role Parameters
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
    GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );
	
	//PeripheralParamUpdate_t = (gapRolesParamUpdateCB_t *)PeripheralParamUpdate;
	
	//PRINTF( "PeripheralParamUpdate_t %08x \n",(int)PeripheralParamUpdate_t );
	
	//GAPRole_PeripheralRegisterAppCBs( (gapRolesParamUpdateCB_t *)&PeripheralParamUpdate ); 
  }

  // Set the GAP Characteristics
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );

  // Set advertising interval
  {
    uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;

    GAP_SetParamValue( TGAP_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_DISC_ADV_INT_MAX, advInt );
  }

  // Setup the GAP Bond Manager
  {
    uint32 passkey = 0; // passkey "000000"
    uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8 mitm = TRUE;
    uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
    uint8 bonding = TRUE;
    GAPBondMgr_SetParameter( GAPBOND_PERI_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PERI_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_PERI_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_PERI_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_PERI_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
  }

  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );            // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
  DevInfo_AddService();                           // Device Information Service
  rwProfile_AddService(GATT_ALL_SERVICES);
  // 
  {
	  //GAPRole_PeripheralRegisterAppCBs( (gapRolesParamUpdateCB_t *)PeripheralParamUpdate_t ); 
	  
  }
  peripheralInitConnItem( &peripheralConnList );
  rwProfile_RegisterAppCBs(&Peripheral_rwProfileCBs);
	// Setup a delayed profile startup
  tmos_set_event( Peripheral_TaskID, SBP_START_DEVICE_EVT );

}

void PeripheralParamUpdate(uint16 connInterval,uint16 connSlaveLatency, uint16 connTimeout)
{
	PRINTF( "update %d %d %d \n",connInterval,connSlaveLatency,connTimeout );
	
//		GAPRole_SendUpdateParam( DEFAULT_DESIRED_MIN_CONN_INTERVAL, DEFAULT_DESIRED_MAX_CONN_INTERVAL,
//                                 DEFAULT_DESIRED_SLAVE_LATENCY, DEFAULT_DESIRED_CONN_TIMEOUT, GAPROLE_NO_ACTION );	
	
}

/*********************************************************************
 * @fn      Peripheral_ProcessEvent
 *
 * @brief   Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The TMOS assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
uint16 Peripheral_ProcessEvent( uint8 task_id, uint16 events )
{

//  VOID task_id; // TMOS required parameter that isn't used in this function

  if ( events & SYS_EVENT_MSG ){
    uint8 *pMsg;

    if ( (pMsg = tmos_msg_receive( Peripheral_TaskID )) != NULL ){
      Peripheral_ProcessTMOSMsg( (tmos_event_hdr_t *)pMsg );
      // Release the TMOS message
      tmos_msg_deallocate( pMsg );
    }
    // return unprocessed events
    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & SBP_START_DEVICE_EVT ){
	// Start the Device
    GAPRole_PeripheralStartDevice( Peripheral_TaskID, &Peripheral_BondMgrCBs, &Peripheral_PeripheralCBs );
    return ( events ^ SBP_START_DEVICE_EVT );
  }

	// Discard unknown events
	return 0;
}

/*********************************************************************
 * @fn      Peripheral_ProcessTMOSMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void Peripheral_ProcessTMOSMsg( tmos_event_hdr_t *pMsg )
{
  switch ( pMsg->event ){
  case GATT_MSG_EVENT:
    {
        gattMsgEvent_t *p_msg = (gattMsgEvent_t *)pMsg;
        if( p_msg->method == ATT_MTU_UPDATED_EVENT )
        {
            //peripheral_Mtu = p_msg->msg.mtuEvt.MTU;
            PRINTF("Mtu:%d\n",p_msg->msg.mtuEvt.MTU);

        }
        break;
    }
  default:
      break;
  }
}
/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB( gapRole_States_t newState, gapRoleEvent_t * pEvent )
{
  switch ( newState )
  {
    case GAPROLE_STARTED:
      PRINTF( "Initialized..\n" );
      break;

    case GAPROLE_ADVERTISING:
      if( pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT )
      {
        Peripheral_LinkTerminated( pEvent );
        PRINTF( "Disconnected.. Reason:%x\n",pEvent->linkTerminate.reason );
      }
      PRINTF( "Advertising..\n" );
      break;

    case GAPROLE_CONNECTED:
      if( pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT )
      {
        Peripheral_LinkEstablished( pEvent );
      }
      PRINTF( "Connected..\n" );
      break;

    case GAPROLE_CONNECTED_ADV:
      PRINTF( "Connected Advertising..\n" );
      break;      

    case GAPROLE_WAITING:
      if( pEvent->gap.opcode == GAP_END_DISCOVERABLE_DONE_EVENT )
      {
        PRINTF( "Waiting for advertising..\n" );
      }
      else if( pEvent->gap.opcode == GAP_LINK_TERMINATED_EVENT )
      {
        Peripheral_LinkTerminated( pEvent );
        PRINTF( "Disconnected.. Reason:%x\n",pEvent->linkTerminate.reason );
      }
      else if( pEvent->gap.opcode == GAP_LINK_ESTABLISHED_EVENT )
            {
                if( pEvent->gap.hdr.status != SUCCESS )
                {
                    PRINTF( "Waiting for advertising..\n" );
                }
                else
                {
                    PRINTF( "Error..\n" );
                }
            }
            else
            {
                PRINTF( "Error..%x\n",pEvent->gap.opcode );
            }
      break;

    case GAPROLE_ERROR:
            PRINTF( "Error..\n" );
      break;

    default:
      break;
  }
}
/*********************************************************************
 * @fn
 *
 * @brief   Prepare and send simpleProfileChar4 notification
 *
 * @param   pValue - data to notify
 *          len - length of data
 *
 * @return  none
 */
static void peripheralStateNotify( uint8 *pValue, uint16 len )
{
  attHandleValueNoti_t noti;
  noti.len = len;
  noti.pValue = GATT_bm_alloc( peripheralConnList.connHandle, ATT_HANDLE_VALUE_NOTI, noti.len, NULL, 0 );
  tmos_memcpy( noti.pValue, pValue, noti.len );
  if( rwProfile_Notify( peripheralConnList.connHandle, &noti ) != SUCCESS )
  {
    GATT_bm_free( (gattMsg_t *)&noti, ATT_HANDLE_VALUE_NOTI );
  }
}

/*********************************************************************
 * @fn      simpleProfileChangeCB
 *
 * @brief   Callback from Profile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void rwProfileChangeCB( uint8 paramID )
{
  uint8 newValue;
  uint8 notifybuffer[13];
  switch( paramID )
  {
    case rwProfile_CHAR1:
    case rwProfile_CHAR2:
        PRINTF("profile ChangeCB CHAR..\n");
        //peripheralStateNotify(notifybuffer,13);
      break;
    default:
      // should not reach here!
      break;
  }
}


/*********************************************************************
*********************************************************************/


void peripheralInitConnItem( peripheralConnItem_t* peripheralConnList )
{
    peripheralConnList->connHandle = GAP_CONNHANDLE_INIT;
    peripheralConnList->connInterval = 0;
    peripheralConnList->connSlaveLatency = 0;
    peripheralConnList->connTimeout = 0;
}

/*********************************************************************
 * @fn      Peripheral_LinkEstablished
 *
 * @brief   Process link established.
 *
 * @param   pEvent - event to process
 *
 * @return  none
 */
static void Peripheral_LinkEstablished( gapRoleEvent_t * pEvent )
{
  gapEstLinkReqEvent_t *event = (gapEstLinkReqEvent_t *) pEvent;

  // See if already connected
  if( peripheralConnList.connHandle != GAP_CONNHANDLE_INIT )
  {
    GAPRole_TerminateLink( pEvent->linkCmpl.connectionHandle );
    PRINTF( "Connection max...\n" );
  }
  else
  {
    peripheralConnList.connHandle = event->connectionHandle;
    peripheralConnList.connInterval = event->connInterval;
    peripheralConnList.connSlaveLatency = event->connLatency;
    peripheralConnList.connTimeout = event->connTimeout;

    // Set timer for periodic event
    //tmos_start_task( Peripheral_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );

    // Set timer for param update event
    //tmos_start_task( Peripheral_TaskID, SBP_PARAM_UPDATE_EVT, SBP_PARAM_UPDATE_DELAY );



    PRINTF("Conn %x - Int %x \n", event->connectionHandle, event->connInterval);
  }
}

/*********************************************************************
 * @fn      Peripheral_LinkTerminated
 *
 * @brief   Process link terminated.
 *
 * @param   pEvent - event to process
 *
 * @return  none
 */
static void Peripheral_LinkTerminated( gapRoleEvent_t * pEvent )
{
  gapTerminateLinkEvent_t *event = (gapTerminateLinkEvent_t *) pEvent;

  if( event->connectionHandle == peripheralConnList.connHandle )
  {
    peripheralConnList.connHandle = GAP_CONNHANDLE_INIT;
    peripheralConnList.connInterval = 0;
    peripheralConnList.connSlaveLatency = 0;
    peripheralConnList.connTimeout = 0;
    //tmos_stop_task( Peripheral_TaskID, SBP_PERIODIC_EVT );

    // Restart advertising
    {
      uint8 advertising_enable = TRUE;
      GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &advertising_enable );
    }
  }
  else
  {
    PRINTF("ERR..\n");
  }
}
/*******************************************************************************
* Function Name  : rw_ProfileReadDataComplete
* Description    : rwprofile 数据读取完成处理
* Input          : none
* Output         : none
* Return         : none
*******************************************************************************/
void rw_ProfileReadDataComplete( unsigned char index )
{
    PRINTF("rw Send Comp!\r\n");
}

/*******************************************************************************
* Function Name  : rw_ProfileWriteData
* Description    : rw 通道数据接收完成处理
* Input          : index：rw 通道序号
                   p_data：写入的数据
                   w_len：写入的长度
* Output         : none
* Return         : none
*******************************************************************************/
void rw_ProfileWriteData( unsigned char index, unsigned char *p_data, unsigned char w_len )
{
    unsigned char res_data[20];
    unsigned char res_len = 0;
    if(index == rwProfile_CHAR1){
#if BLE_PROV
        Rec_BLE_PROV_DataDeal(p_data, w_len, res_data, &res_len);
#endif
        rwProfile_SendData(rwProfile_CHAR1, res_data, res_len);
    }
}
