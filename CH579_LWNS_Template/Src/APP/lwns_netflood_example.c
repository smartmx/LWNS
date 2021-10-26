/*
 * lwns_netflood_example.c
   *   网络泛洪
   *    使用需要去app_main.c中取消本例子初始化函数的注释
 *  Created on: Jul 20, 2021
 *      Author: WCH
 */
#include "CH57x_common.h"
#include "lwns_netflood_example.h"
#include "config.h"

#define DEBUG_PRINT_IN_THIS_FILE 1
#if DEBUG_PRINT_IN_THIS_FILE
#define PRINTF(...) PRINT(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif


static uint8 TX_DATA[LWNS_DATA_SIZE] = {0};//最大长度数据收发测试
static uint8 RX_DATA[LWNS_DATA_SIZE] = {0};//最大长度数据收发测试
static uint16 lwns_netflood_ProcessEvent(uint8 task_id, uint16 events);
static int netflood_recv(lwns_controller_ptr ptr,
        const lwns_addr_t *from,
        const lwns_addr_t *originator, uint8_t hops);
static void netflood_sent(lwns_controller_ptr ptr);
static void netflood_dropped(lwns_controller_ptr ptr);

static const struct lwns_netflood_callbacks callbacks = { netflood_recv,
        netflood_sent, netflood_dropped };

static uint8 netflood_taskID;

void lwns_netflood_process_init(void);

static lwns_netflood_controller netflood;//网络泛洪控制结构体

static int netflood_recv(lwns_controller_ptr ptr,
        const lwns_addr_t *from,
        const lwns_addr_t *originator, uint8_t hops) {
    uint8 len;
    len = lwns_buffer_datalen(); //获取当前缓冲区接收到的数据长度
    PRINTF("netflood %d rec %02x %02x %02x %02x %02x %02x,hops=%d\r\n",get_lwns_object_port(ptr),
            from->u8[0], from->u8[1], from->u8[2], from->u8[3], from->u8[4],
            from->u8[5], hops);//打印转发者，即为收到的本次转发数据是谁转发的。
    PRINTF("netflood orec %02x %02x %02x %02x %02x %02x,hops=%d\r\n",
            originator->u8[0], originator->u8[1], originator->u8[2],
            originator->u8[3], originator->u8[4], originator->u8[5],
            hops);//打印出信息发起者，即为发起本次网络泛洪的节点地址。
    lwns_buffer_save_data(RX_DATA); //接收数据到用户数据区域
    PRINTF("data:");
    for (uint8 i = 0; i < len; i++) {
        PRINTF("%02x ", RX_DATA[i]);
    }
    PRINTF("\n");
    return 1;//返回1，则本节点将继续发送netflood，转发数据
    //return 0;//返回0，则本节点不再继续netflood，直接终止
}
static void netflood_sent(lwns_controller_ptr ptr) {
    PRINTF("netflood %d sent\n",get_lwns_object_port(ptr));
}
static void netflood_dropped(lwns_controller_ptr ptr) {
    PRINTF("netflood %d dropped\n",get_lwns_object_port(ptr));
}

void lwns_netflood_process_init(void) {
    netflood_taskID = TMOS_ProcessEventRegister(lwns_netflood_ProcessEvent);
    for(uint8 i = 0; i < LWNS_DATA_SIZE ;i++){
        TX_DATA[i]=i;
    }
    lwns_netflood_init(&netflood,
            137,//打开一个端口号为137的泛洪结构体
            HTIMER_SECOND_NUM*1,//等待转发时间
            1,//在等待期间，接收到几次同样的数据包就取消本数据包的发送
            3,//最大转发层级
            FALSE,//在等待转发过程中，收到了新的需要转发的数据包，旧数据包是立刻发送出去还是丢弃，FALSE为立刻发送，TRUE为丢弃。
            50,//网络恢复参数，该值定义了一个差距，如果包序号比内存内保存的数据包序号小的值大于此值，则会认为网络故障恢复，继续接收该数据包。
            //同时，该值也决定了判定为新数据包的差值，即来自同一个节点的新数据包的序号不可以比内存中的大过多，即比此值还大。
            //例如，内存中保存的为10，新数据包序号为60，差值为50，大于等于此时设置的50，所以将不会被认为为新的数据包，被丢弃。
            //只有序号为59，差值为49，小于该值，才会被接收。
            &callbacks);//返回0代表打开失败。返回1打开成功。
#if 1
    tmos_start_task(netflood_taskID, NETFLOOD_EXAMPLE_TX_PERIOD_EVT,
            MS1_TO_SYSTEM_TIME(1000));
#endif
}

static uint16 lwns_netflood_ProcessEvent(uint8 task_id, uint16 events) {
    if (events & NETFLOOD_EXAMPLE_TX_PERIOD_EVT) {
        PRINTF("send\n");
        lwns_buffer_load_data(TX_DATA, sizeof(TX_DATA)); //载入需要发送的数据到缓冲区
        lwns_netflood_send(&netflood); //发送网络泛洪数据包
        tmos_start_task(netflood_taskID, NETFLOOD_EXAMPLE_TX_PERIOD_EVT,
                MS1_TO_SYSTEM_TIME(3000)); //10s发送一次
        return (events ^ NETFLOOD_EXAMPLE_TX_PERIOD_EVT);
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
