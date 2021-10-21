/*
 * lwns_ruc_example.c
   *   可靠单播传输例子
 *  reliable unicast
   *    使用需要去app_main.c中取消本例子初始化函数的注释
 *  Created on: Jul 19, 2021
 *      Author: WCH
 */
#include "CH57x_common.h"
#include "config.h"
#include "lwns_ruc_example.h"

#define DEBUG_PRINT_IN_THIS_FILE 1
#if DEBUG_PRINT_IN_THIS_FILE
#define PRINTF(...) PRINT(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#if 1
static lwns_addr_t dst_addr = { { 0xa3, 0xdf, 0x38, 0xe4, 0xc2, 0x84 } };//目标节点地址，测试时，请根据电路板芯片MAC地址不同进行修改。修改为接收方的MAC地址，请勿使用自己的MAC地址
#else
static lwns_addr_t dst_addr = { { 0xd9, 0x37, 0x3c, 0xe4, 0xc2, 0x84 } };
#endif

static lwns_ruc_controller ruc;//声明可靠单播控制结构体

static uint8 TX_DATA[10] =
        { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };
static uint8 RX_DATA[10];

static uint8 ruc_taskID;
uint16 lwns_ruc_ProcessEvent(uint8 task_id, uint16 events);


void lwns_ruc_process_init(void);

static void recv_ruc(lwns_controller_ptr ptr,
        const lwns_addr_t* sender);

static void sent_ruc(lwns_controller_ptr ptr,
        const lwns_addr_t* to, uint8_t retransmissions);
static void timedout_ruc(lwns_controller_ptr ptr,
        const lwns_addr_t* to);




static void recv_ruc(lwns_controller_ptr ptr,
        const lwns_addr_t* sender) {
    //ruc中接收到发送给自己的数据后，才会调用该回调
    uint8 len;
    len = lwns_buffer_datalen(); //获取当前缓冲区接收到的数据长度
    if (len == 10) {
        lwns_buffer_save_data(RX_DATA); //接收数据到用户数据区域
        PRINTF("ruc %d rec %02x %02x %02x %02x %02x %02x\r\n",get_lwns_object_port(ptr), sender->u8[0],
                sender->u8[1], sender->u8[2], sender->u8[3], sender->u8[4], sender->u8[5]);
        PRINTF("data:");
        for (uint8 i = 0; i < len; i++) {
            PRINTF("%02x ", RX_DATA[i]);
        }
        PRINTF("\n");
    } else {
        PRINTF("data len err\n");
    }
}


static void sent_ruc(lwns_controller_ptr ptr,
        const lwns_addr_t* to, uint8_t retransmissions) {
    //ruc中发送成功，并且收到目标节点的ack回复后，才会调用该回调
    PRINTF("ruc %d sent %d\r\n",get_lwns_object_port(ptr),retransmissions);
    tmos_start_task(ruc_taskID, RUC_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000));//更新任务时间，发送并收到回复后，1秒钟后再发送
}
static void timedout_ruc(lwns_controller_ptr ptr,
        const lwns_addr_t* to) {
    //ruc中，再重发次数超过最大重发次数后，会调用该回调。
    PRINTF("ruc %d timedout\n",get_lwns_object_port(ptr));
    tmos_start_task(ruc_taskID, RUC_EXAMPLE_TX_PERIOD_EVT,
                    MS1_TO_SYSTEM_TIME(1000));
}




static const struct lwns_ruc_callbacks ruc_callbacks = {
        recv_ruc, sent_ruc, timedout_ruc };//声明可靠单播回调结构体

void lwns_ruc_process_init(void) {
    lwns_ruc_init(&ruc,
            144,//打开一个端口号为144的可靠单播
            HTIMER_SECOND_NUM,//等待ack时间间隔，没收到就会重发
            &ruc_callbacks);//返回0代表打开失败。返回1打开成功。
    ruc_taskID = TMOS_ProcessEventRegister(lwns_ruc_ProcessEvent);
    tmos_start_task(ruc_taskID, RUC_EXAMPLE_TX_PERIOD_EVT,
                MS1_TO_SYSTEM_TIME(1000));
}


uint16 lwns_ruc_ProcessEvent(uint8 task_id, uint16 events) {
    if (events & RUC_EXAMPLE_TX_PERIOD_EVT) {
        uint8 temp;
        temp = TX_DATA[0];
        for (uint8 i = 0; i < 9; i++) {
            TX_DATA[i] = TX_DATA[i + 1];//移位发送数据，以便观察效果
        }
        TX_DATA[9] = temp;
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA));//载入需要发送的数据到缓冲区
        lwns_ruc_send(&ruc,
                &dst_addr,//可靠单播目标地址
                4//最大重发次数
                );//可靠单播发送函数：发送参数，目标地址，最大重发次数，默认一秒钟重发一次
        return events ^ RUC_EXAMPLE_TX_PERIOD_EVT;
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
