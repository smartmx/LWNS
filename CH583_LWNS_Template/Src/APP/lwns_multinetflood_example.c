/*
 * lwns_multinetflood_example.c
   *    使用需要去app_main.c中取消本例子初始化函数的注释
   *   组播网络泛洪传输例子，将数据发往订阅的地址，订阅的地址为2字节u16类型。
 * multinetflood
 *  Created on: Jul 19, 2021
 *      Author: WCH
 */

#include "lwns_multinetflood_example.h"
#include "CH58x_common.h"
#include "config.h"

#define DEBUG_PRINT_IN_THIS_FILE 1
#if DEBUG_PRINT_IN_THIS_FILE
#define PRINTF(...) PRINT(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif


static u8 subaddrs_index = 0;//发送订阅地址序号
#define SUBADDR_NUM     3//订阅地址数量
static uint16 subaddrs[SUBADDR_NUM]={1,2,3};//订阅地址数组

static uint8 TX_DATA[LWNS_DATA_SIZE] = {0};//最大长度数据收发测试
static uint8 RX_DATA[LWNS_DATA_SIZE] = {0};//最大长度数据收发测试
static uint16 lwns_multinetflood_ProcessEvent(uint8 task_id, uint16 events);
static void multinetflood_recv(lwns_controller_ptr ptr,u16 subaddr,const lwns_addr_t *sender, uint8_t hops);//组播网络泛洪接收回调函数
static void multinetflood_sent(lwns_controller_ptr ptr);//组播网络泛洪发送完成回调函数

static lwns_multinetflood_controller multinetflood;//声明组播网络泛洪控制结构体

static uint8 multinetflood_taskID;//组播网络泛洪控制任务id

static void multinetflood_recv(lwns_controller_ptr ptr,u16 subaddr,const lwns_addr_t *sender, uint8_t hops){
    uint8 len;
    len = lwns_buffer_datalen(); //获取当前缓冲区接收到的数据长度
    lwns_buffer_save_data(RX_DATA); //接收数据到用户数据区域
    PRINTF("multinetflood %d rec from %02x %02x %02x %02x %02x %02x\n",
            get_lwns_object_port(ptr),
            sender->u8[0], sender->u8[1], sender->u8[2], sender->u8[3],
            sender->u8[4], sender->u8[5]);//from为接收到的数据的发送方地址
    PRINTF("subaddr:%d,data:",subaddr);
    for (uint8 i = 0; i < len; i++) {
        PRINTF("%02x ", RX_DATA[i]);//打印出数据
    }
    PRINTF("\n");
}

static void multinetflood_sent(lwns_controller_ptr ptr) {
    PRINTF("multinetflood %d sent\n",get_lwns_object_port(ptr));
}

static const struct lwns_multinetflood_callbacks multinetflood_callbacks =
{multinetflood_recv,multinetflood_sent};//注册组播网络泛洪回调函数


void lwns_multinetflood_process_init(void) {
    multinetflood_taskID = TMOS_ProcessEventRegister(lwns_multinetflood_ProcessEvent);
    for(uint8 i = 0; i < LWNS_DATA_SIZE ;i++){
        TX_DATA[i]=i;
    }
    lwns_multinetflood_init(&multinetflood,
                137,//打开一个端口号为137的组播网络泛洪结构体
                HTIMER_SECOND_NUM,//最大等待转发时间
                1,//在等待期间，接收到几次同样的数据包就取消本数据包的发送
                3,//最大转发层级
                FALSE,//在等待转发过程中，收到了新的需要转发的数据包，旧数据包是立刻发送出去还是丢弃，FALSE为立刻发送，TRUE为丢弃。
                20,//网络恢复参数，该值定义了一个差距，如果包序号比内存内保存的数据包序号小的值大于此值，则会认为网络故障恢复，继续接收该数据包。
                //同时，该值也决定了判定为新数据包的差值，即来自同一个节点的新数据包的序号不可以比内存中的大过多，即比此值还大。
                //例如，内存中保存的为10，新数据包序号为50，差值为40，大于此时设置的20，所以将不会被认为为新的数据包，被丢弃。
                //只有序号为29，差值为19，小于该值，才会被接收。
                TRUE,//本机是否转发目标非本机的数据包，类似于蓝牙mesh是否启用relay功能。
                subaddrs,//订阅的地址数组指针
                SUBADDR_NUM,//订阅地址数量
                &multinetflood_callbacks
                );//返回0代表打开失败。返回1打开成功。
#if 1
    tmos_start_task(multinetflood_taskID, MULTINETFLOOD_EXAMPLE_TX_PERIOD_EVT,
            MS1_TO_SYSTEM_TIME(1000));
#endif
}

uint16 lwns_multinetflood_ProcessEvent(uint8 task_id, uint16 events) {
    if (events & MULTINETFLOOD_EXAMPLE_TX_PERIOD_EVT) {
       uint8 temp;
       temp = TX_DATA[0];
       for (uint8 i = 0; i < 9; i++) {
           TX_DATA[i] = TX_DATA[i + 1];//移位发送数据，以便观察效果
       }
       TX_DATA[9] = temp;
       lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA));//载入需要发送的数据到缓冲区
       if(subaddrs_index >= SUBADDR_NUM){
          subaddrs_index = 0;
       }
       lwns_multinetflood_send(&multinetflood,subaddrs[subaddrs_index]);//组播网络泛洪发送数据到订阅地址
       subaddrs_index++;

       tmos_start_task(multinetflood_taskID, MULTINETFLOOD_EXAMPLE_TX_PERIOD_EVT,
               MS1_TO_SYSTEM_TIME(1000));//周期性发送
       return events ^ MULTINETFLOOD_EXAMPLE_TX_PERIOD_EVT;
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

