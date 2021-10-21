/*
 * lwns_unicast_example.c
   *    使用需要去app_main.c中取消本例子初始化函数的注释
   *   单播传输例子
   *   single-hop unicast
 *  Created on: Jul 19, 2021
 *      Author: WCH
 */

#include "lwns_unicast_example.h"
#include "CH57x_common.h"
#include "config.h"

#define DEBUG_PRINT_IN_THIS_FILE 1
#if DEBUG_PRINT_IN_THIS_FILE
#define PRINTF(...) PRINT(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#if 1
static lwns_addr_t dst_addr = { { 0xab, 0xdf, 0x38, 0xe4, 0xc2, 0x84 } };//目标节点地址，测试时，请根据电路板芯片MAC地址不同进行修改。修改为接收方的MAC地址，请勿使用自己的MAC地址
#else
static lwns_addr_t dst_addr = { { 0xd9, 0x37, 0x3c, 0xe4, 0xc2, 0x84 } };
#endif

static uint8 TX_DATA[10] =
        { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };
static uint8 RX_DATA[10];
static uint16 lwns_unicast_ProcessEvent(uint8 task_id, uint16 events);
static void unicast_recv(lwns_controller_ptr c, const lwns_addr_t *from);//单播接收回调函数
static void unicast_sent(lwns_controller_ptr ptr);//单播发送完成回调函数

static lwns_unicast_controller unicast;//声明单播控制结构体

static uint8 unicast_taskID;//声明单播控制任务id

static void unicast_recv(lwns_controller_ptr ptr, const lwns_addr_t *from){
    uint8 len;
    len = lwns_buffer_datalen(); //获取当前缓冲区接收到的数据长度
    if (len == 10) {
        lwns_buffer_save_data(RX_DATA); //接收数据到用户数据区域
        PRINTF("unicast %d rec from %02x %02x %02x %02x %02x %02x\n",
                get_lwns_object_port(ptr),
                from->u8[0], from->u8[1], from->u8[2], from->u8[3],
                from->u8[4], from->u8[5]);//from为接收到的数据的发送方地址
        PRINTF("data:");
        for (uint8 i = 0; i < len; i++) {
            PRINTF("%02x ", RX_DATA[i]);
        }
        PRINTF("\n");
    } else {
        PRINTF("data len err\n");
    }
}

static void unicast_sent(lwns_controller_ptr ptr) {
    PRINTF("unicast %d sent\n",get_lwns_object_port(ptr));
}

static const struct lwns_unicast_callbacks unicast_callbacks =
{unicast_recv,unicast_sent};//注册回调函数


void lwns_unicast_process_init(void) {
    unicast_taskID = TMOS_ProcessEventRegister(lwns_unicast_ProcessEvent);
    lwns_unicast_init(&unicast,
            136,//打开一个端口号为136的单播
            &unicast_callbacks
            );//返回0代表打开失败。返回1打开成功。
    tmos_start_task(unicast_taskID, UNICAST_EXAMPLE_TX_PERIOD_EVT,
            MS1_TO_SYSTEM_TIME(1000));
}

uint16 lwns_unicast_ProcessEvent(uint8 task_id, uint16 events) {
    if (events & UNICAST_EXAMPLE_TX_PERIOD_EVT) {
        uint8 temp;
        temp = TX_DATA[0];
        for (uint8 i = 0; i < 9; i++) {
            TX_DATA[i] = TX_DATA[i + 1];//移位发送数据，以便观察效果
        }
        TX_DATA[9] = temp;
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA));//载入需要发送的数据到缓冲区
        lwns_unicast_send(&unicast,&dst_addr);//单播发送数据给指定节点
        tmos_start_task(unicast_taskID, UNICAST_EXAMPLE_TX_PERIOD_EVT,
                MS1_TO_SYSTEM_TIME(1000));//周期性发送
        return events ^ UNICAST_EXAMPLE_TX_PERIOD_EVT;
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

