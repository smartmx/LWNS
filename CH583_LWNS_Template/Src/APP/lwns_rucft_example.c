/*
 * lwns_rucft_example.c
   *   可靠单播块传输例子
 *   reliable unicast file transfer
   *    使用需要去app_main.c中取消本例子初始化函数的注释
 *  Created on: Jul 19, 2021
 *      Author: WCH
 */
#include "lwns_rucft_example.h"
#include "CH58x_common.h"
#include "config.h"

#define DEBUG_PRINT_IN_THIS_FILE 1
#if DEBUG_PRINT_IN_THIS_FILE
#define PRINTF(...) PRINT(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

#if 1
static lwns_addr_t dst_addr = { { 0x66, 0xdf, 0x38, 0xe4, 0xc2, 0x84 } }; //目标节点地址，测试时，请根据电路板芯片MAC地址不同进行修改。修改为接收方的MAC地址，请勿使用自己的MAC地址
#else
static lwns_addr_t dst_addr = { { 0xd9, 0x37, 0x3c, 0xe4, 0xc2, 0x84 } };
#endif

static uint8 rucft_taskID;

static lwns_rucft_controller rucft; //声明rucft控制结构体

#define FILESIZE 4000
static char strsend[FILESIZE]; //发送缓冲区
static char *strp;
static void write_file(lwns_controller_ptr ptr, const lwns_addr_t *sender,
        int offset, int flag, char *data, int datalen);
static int read_file(lwns_controller_ptr ptr, int offset, char *to,
        int maxsize);
static void timedout_rucft(lwns_controller_ptr ptr);

const static struct lwns_rucft_callbacks rucft_callbacks = { write_file,
        read_file, timedout_rucft };

uint16 lwns_rucft_ProcessEvent(uint8 task_id, uint16 events);
static void write_file(lwns_controller_ptr ptr, const lwns_addr_t *sender,
        int offset, int flag, char *data, int datalen) {
    //sender为发送方的地址
    //如果需要接收不同的文件，需要在此函数中做好接口
    if (datalen > 0) {     //声明个缓冲从data里取数据打印
        PRINTF("r:%c\n", *data);
    }
    if (flag == LWNS_RUCFT_FLAG_END) {
        PRINTF("re\n");
        //本次文件传输的最后一个包
    } else if (flag == LWNS_RUCFT_FLAG_NONE) {
        PRINTF("ru\n");
        //本次文件传输正常的包
    } else if (flag == LWNS_RUCFT_FLAG_NEWFILE) {
        PRINTF("rn\n");
        //本次文件传输的第一个包
    }
}

static int read_file(lwns_controller_ptr ptr, int offset, char *to,
        int maxsize) {
    //to为需要保存数据过去的指针
    //如果需要发送不同的文件，需要在此函数中做好接口
    int size = maxsize;
    if (offset >= FILESIZE) {
        //上次已经发完,本次是最后确认
        PRINTF("Send done\n");
        tmos_start_task(rucft_taskID, RUCFT_EXAMPLE_TX_PERIOD_EVT,
                MS1_TO_SYSTEM_TIME(5000)); //5秒钟后继续发送测试
        return 0;
    } else if (offset + maxsize >= FILESIZE) {
        size = FILESIZE - offset;

    }
    //把本次需要发送的内容压进包缓冲
    tmos_memcpy(to, strp + offset, size);
    return size;
}
static void timedout_rucft(lwns_controller_ptr ptr) {
    //rucft中，发送方再重发次数超过最大重发次数后，会调用该回调。
    //接收方超时没接收到下一个包也会调用
    PRINTF("rucft %d timedout\r\n", get_lwns_object_port(ptr));
}
void lwns_rucft_process_init(void) {
    lwns_addr_t MacAddr;
    rucft_taskID = TMOS_ProcessEventRegister(lwns_rucft_ProcessEvent);
    lwns_rucft_init(&rucft, 137, //端口号
            HTIMER_SECOND_NUM / 10, //等待目标节点ack时间
            5, //最大重发次数，与ruc中的ruc_send的重发次数功能一样
            &rucft_callbacks//回调函数
            ); //返回0代表打开失败。返回1打开成功。
    int i;
    for (i = 0; i < FILESIZE; i++) {    //LWNS_RUCFT_DATASIZE个LWNSNK_RUCFT_DATASIZE个b，等等，初始化需要发送的数据
        strsend[i] = 'a' + i / LWNS_RUCFT_DATASIZE;
    }
    strp = strsend;
    GetMACAddress(MacAddr.u8);
    if (lwns_addr_cmp(&MacAddr, &dst_addr)) {

    } else {
        tmos_start_task(rucft_taskID, RUCFT_EXAMPLE_TX_PERIOD_EVT,
                MS1_TO_SYSTEM_TIME(1000));
    }
}
uint16 lwns_rucft_ProcessEvent(uint8 task_id, uint16 events) {
    if (events & RUCFT_EXAMPLE_TX_PERIOD_EVT) {
        PRINTF("send\n");
        lwns_rucft_send(&rucft, &dst_addr); //开始发送至目标节点，用户启用发送时要配置好回调函数中的数据包读取
        return events ^ RUCFT_EXAMPLE_TX_PERIOD_EVT;
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
