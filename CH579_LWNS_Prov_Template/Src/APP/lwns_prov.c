/*
 * lwns_prov.c
 *  lwns通过2.4g配网例程，也可以使用蓝牙、usb、串口等其他进行配网
   *  该例程需要一个设备充当配网设备，配网主设备需要信号触发进入配网模式，进入之前需要将mac层发送列表全部清空，或者配网设备只用作配网功能
   * 主设备会打开一个广播通道，恢复自身未配网时的秘钥和rf参数，进行广播数据的收发。
   * 主设备接收到未配网设备的广播数据后，将新的秘钥和rf参数广播发送回去。
   * 即完成配网流程。
 *  Created on: Sep 28, 2021
 *      Author: WCH
 */
#include "lwns_prov.h"
#include "lwns_adapter_csma_mac.h"
#include "lwns_adapter_blemesh_mac.h"
#include "lwns_adapter_no_mac.h"
#include "lwns_broadcast_example.h"

#define DEBUG_PRINT_IN_THIS_FILE 1
#if DEBUG_PRINT_IN_THIS_FILE
#define PRINTF(...) PRINT(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

u8 prov_task_id;
uint16 lwns_prov_ProcessEvent(uint8 task_id, uint16 events);

__attribute__((aligned(4)))  const char lwns_prov_index[]={"prov"};

//默认的配网参数
static const rf_config_params_t lwns_rf_params_default = {
        .Channel[0] = 8,
        .Channel[1] = 18,
        .Channel[2] = 28,
        .channelNum = 3,
        .CRCInit = 0x555555,
        .accessAddress = 0x17267162,
};

//默认的配网秘钥
static const unsigned char lwns_sec_key_default[16]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};

#if PROV_MASTER
//配网后的配网参数
static const rf_config_params_t lwns_rf_params_prov = {
        .Channel[0] = 9,
        .Channel[1] = 19,
        .Channel[2] = 29,
        .channelNum = 3,
        .CRCInit = 0x555555,
        .accessAddress = 0x71621726,
};
//配网后的配网秘钥
static const unsigned char lwns_sec_key_prov[16]={10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160};

#endif


static void prov_bc_recv(lwns_controller_ptr ptr,
        const lwns_addr_t* sender) {
#if PROV_MASTER
		void *prov_uc;
    //用户根据需要，可以选择将sender保存下来
    u8 prov_buffer[sizeof(rf_config_params_t) + sizeof(lwns_sec_key_prov)];//发送的数据中用户可以自行添加需要的校验等数据
    PRINTF("rec prov request from %02x,%02x,%02x,%02x,%02x,%02x\n",sender->u8[0],sender->u8[1],sender->u8[2],sender->u8[3],sender->u8[4],sender->u8[5]);
    prov_uc = lwns_controller_lookup(PROV_UC_PORT);//查找控制结构体指针
    if(prov_uc != NULL) {
        //用户可以添加其他配置参数进行校验
        tmos_memcpy(prov_buffer, &lwns_rf_params_prov, sizeof(rf_config_params_t));
        tmos_memcpy(prov_buffer + sizeof(rf_config_params_t), lwns_sec_key_prov, sizeof(lwns_sec_key_prov));
        lwns_buffer_load_data(prov_buffer, sizeof(rf_config_params_t) + sizeof(lwns_sec_key_prov));
        lwns_unicast_send(prov_uc, sender);
        PRINTF("send prov params\n");
    }
#else
    PRINTF("I am slave,ignore packet\n");
#endif
}

static const struct lwns_broadcast_callbacks prov_bc_callbacks = {
        prov_bc_recv, NULL };//声明广播回调函数结构体，注册回调函数

static void prov_uc_recv(lwns_controller_ptr ptr, const lwns_addr_t *from){
#if PROV_MASTER
    PRINTF("I am master,ignore packet\n");
#else
    uint8 *data,len;
    len = lwns_buffer_datalen(); //获取当前缓冲区接收到的数据长度
    if (len == sizeof(rf_config_params_t) + sizeof(lwns_sec_key_default)) {
        PRINTF("rec prov params from %02x,%02x,%02x,%02x,%02x,%02x\n",from->u8[0],from->u8[1],from->u8[2],from->u8[3],from->u8[4],from->u8[5]);
        data = lwns_buffer_dataptr();
        tmos_memcpy(&lwns_rf_params,data,sizeof(rf_config_params_t));
        rf_config_params_save_to_flash();//保存rf参数
        RF_Init();//重新初始化rf参数
        lwns_temp_set_key(data + sizeof(rf_config_params_t));
        lwns_save_key_to_flash();//保存key
        lwns_set_provisioned();//设置已配网
        tmos_stop_task(prov_task_id, PROV_BROADCAST_TX_PERIOD_EVT);
        tmos_clear_event(prov_task_id, PROV_BROADCAST_TX_PERIOD_EVT);
        prov_process_deinit();
        PRINTF("Start lwns user process\n");
        lwns_broadcast_process_init();

    } else {
        PRINTF("data len err\n");
    }
#endif
}
static const struct lwns_unicast_callbacks prov_uc_callbacks = {
        prov_uc_recv,NULL
};



void prov_process_init(void) {
    void *prov_bc,*prov_uc;

    PRINTF("prov init\n");
#if PROV_MASTER
    prov_bc = tmos_msg_allocate(sizeof(lwns_broadcast_controller));//申请内存，可能并不是一直打开配网，所以尽量不占用全局变量
    if(prov_bc == NULL){
        PRINTF("failed\n");//初始化配网控制结构体失败
        return;
    }
    prov_uc = tmos_msg_allocate(sizeof(lwns_unicast_controller));//申请内存
    if(prov_uc == NULL){
        tmos_msg_deallocate(prov_bc);//释放通道控制结构体占用内存
        PRINTF("failed\n");//初始化配网控制结构体失败
        return;
    }
    lwns_broadcast_init(prov_bc, PROV_BC_PORT, &prov_bc_callbacks); //打开通道号为1的广播通道，进行配网数据的收发
    lwns_unicast_init(prov_uc, PROV_UC_PORT, &prov_uc_callbacks); //打开通道号为2的单播通道，进行配网数据的收发
    PRINTF("I am master,don't need to send prov request\n");
#else
    if(lwns_provisioned() == 0){
        prov_bc = tmos_msg_allocate(sizeof(lwns_broadcast_controller));//申请内存，可能并不是一直打开配网，所以尽量不占用全局变量
        if(prov_bc == NULL){
            PRINTF("failed\n");//初始化配网控制结构体失败
            return;
        }
        prov_uc = tmos_msg_allocate(sizeof(lwns_unicast_controller));//申请内存
        if(prov_uc == NULL){
            tmos_msg_deallocate(prov_bc);//释放通道控制结构体占用内存
            PRINTF("failed\n");//初始化配网控制结构体失败
            return;
        }
        lwns_broadcast_init(prov_bc, PROV_BC_PORT, &prov_bc_callbacks); //打开通道号为1的广播通道，进行配网数据的收发
        lwns_unicast_init(prov_uc, PROV_UC_PORT, &prov_uc_callbacks); //打开通道号为2的单播通道，进行配网数据的收发
        prov_task_id = TMOS_ProcessEventRegister(lwns_prov_ProcessEvent);//从机注册周期广播任务
        PRINTF("prov taskid:%d\n",prov_task_id);
        tmos_start_task(prov_task_id, PROV_BROADCAST_TX_PERIOD_EVT,
                MS1_TO_SYSTEM_TIME(1000));//开始周期性广播，寻求配网
    } else {
        PRINTF("I am provisioned,don't need to send prov request\n");
        PRINTF("Start lwns user process\n");
        lwns_broadcast_process_init();
    }
#endif
}



void prov_process_deinit(void){
    void *prov_bc,*prov_uc;
    prov_bc = lwns_controller_lookup(PROV_BC_PORT);//查找通道控制结构体指针
    if(prov_bc!=NULL) {
        lwns_broadcast_close(prov_bc);//关闭广播通道
        tmos_msg_deallocate(prov_bc);//释放通道控制结构体占用内存
    } else {
        PRINTF("find bc failed\n");
    }
    prov_uc = lwns_controller_lookup(PROV_UC_PORT);
    if(prov_uc != NULL) {
        lwns_unicast_close(prov_uc);//关闭单播通道
        tmos_msg_deallocate(prov_uc);//释放通道控制结构体占用内存
    } else {
        PRINTF("find uc failed\n");
    }
}


uint16 lwns_prov_ProcessEvent(uint8 task_id, uint16 events) {
    if (events & PROV_BROADCAST_TX_PERIOD_EVT) {
        uint8 *prov_bc;
        lwns_buffer_load_data(NULL, 0);//载入需要发送的数据到缓冲区
        prov_bc = lwns_controller_lookup(PROV_BC_PORT);
        if(prov_bc != NULL) {
            PRINTF("send prov request\n");
            lwns_broadcast_send(prov_bc);//广播发送数据
            tmos_start_task(task_id, PROV_BROADCAST_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000));//周期性发送
        }
        return events ^ PROV_BROADCAST_TX_PERIOD_EVT;
    }
    if (events & SYS_EVENT_MSG) {
        uint8 *pMsg;

        if ((pMsg = tmos_msg_receive(task_id)) != NULL) {
            // Release the TMOS message
            tmos_msg_deallocate(pMsg);
        }
        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }
    return 0;
}

//获取设备配网状态
uint8 lwns_provisioned(void)
{
    size_t  len;
    uint8 value;
    ef_get_env_blob(lwns_prov_index, NULL, 0, &len);
    if(len == 1){
        ef_get_env_blob(lwns_prov_index, &value, 1, NULL);
        return value;
    }
    return 0;
}

//设置设备已配网状态
EfErrCode lwns_set_provisioned(void)
{
    EfErrCode err;
    uint8 state = 1;
    err = ef_set_env_blob(lwns_prov_index, &state, 1);
    if(err != EF_NO_ERR){
        PRINTF("set prov err\n");
    } else {
        PRINTF("set prov ok\n");
    }
    return err;
}

//恢复设备未配网状态
EfErrCode lwns_provisioned_reset(void)
{
    EfErrCode err;
    tmos_memcpy(&lwns_rf_params,&lwns_rf_params_default,sizeof(rf_config_params_t));//恢复默认的rf参数
    rf_config_params_save_to_flash();//保存rf参数
    lwns_temp_set_key((unsigned char *)lwns_sec_key_default);//恢复默认的秘钥参数
    lwns_save_key_to_flash();//保存key
    err = ef_del_env(lwns_prov_index);
    PRINTF("prov reset:%d\n",err);
    return err;
}

/*
 * lwns通过手机蓝牙进行配网
   *  该例程需要一个设备开启蓝牙功能，手机通过rwprofile给设备发送配网参数
   * 即完成配网流程。
 */

#if BLE_PROV

void Rec_BLE_PROV_DataDeal(unsigned char *p_data, unsigned char w_len ,unsigned char *r_data, unsigned char* r_len)
{
    r_data[0] = p_data[0];
    switch(p_data[0]){
    case LWNS_PROV_CMD_TEST:{
        PRINTF("test ok\n");
        tmos_memcpy(r_data, p_data, w_len);
        *r_len = w_len;
        break;
    }
#if PROV_MASTER
    //主机不配网
#else
    case LWNS_PROV_CMD_GET_PROV_STATE:{
        r_data[1] = lwns_provisioned();
        *r_len = 2;
        break;
    }
    case LWNS_PROV_CMD_GET_RF_CONFIG_PARAMS:{
        tmos_memcpy(r_data + 1, &lwns_rf_params, sizeof(lwns_rf_params));
        *r_len = 1 + sizeof(lwns_rf_params);
        break;
    }
    case LWNS_PROV_CMD_GET_RF_KEY:{
        lwns_get_key(r_data + 1);
        *r_len = 17;
        break;
    }
    case LWNS_PROV_CMD_SET_RF_CONFIG_PARAMS:{
        tmos_memcpy(&lwns_rf_params, p_data + 1, sizeof(lwns_rf_params));
        r_data[1] = 0;
        *r_len = 2;
        break;
    }
    case LWNS_PROV_CMD_SET_RF_KEY:{
        lwns_temp_set_key(p_data + 1);
        r_data[1] = 0;
        *r_len = 2;
        break;
    }
    case LWNS_PROV_CMD_PROV_ENABLE:{
        EfErrCode err;
        *r_len = 2;
        err = rf_config_params_save_to_flash();//保存rf参数
        if(err){
            r_data[1] = err;
            break;
        }
        RF_Init();//重新初始化rf参数
        err = lwns_save_key_to_flash();//保存key
        if(err){
            r_data[1] = err;
            break;
        }
        err = lwns_set_provisioned();//设置已配网
        if(err){
            r_data[1] = err;
            break;
        }
        r_data[1] = 0;
        break;
    }
    case LWNS_PROV_CMD_PROV_RESET:{
        r_data[1] = lwns_provisioned_reset();
        *r_len = 2;
        break;
    }
#endif
    default:
        r_data[1] = 0xfe;
        *r_len = 2;
        break;
    }
}

#endif

