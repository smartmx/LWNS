/*
 * rf_config_params.c
 * 使用easyflash
 * rf初始化相关参数的保存和读取
 *  Created on: Sep 27, 2021
 *      Author: WCH
 */
#include "rf_config_params.h"

#define DEBUG_PRINT_IN_THIS_FILE 1
#if DEBUG_PRINT_IN_THIS_FILE
#define PRINTF(...) PRINT(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif

//用户在此修改，为默认的rf config参数，第一次运行程序，flash中不会存有此参数，所以默认采用此参数进行rf配置
//通过配网，用户将新的rf config参数写入flash中，即完成配网操作
/**
 * 默认的rf参数信息
 */
rf_config_params_t lwns_rf_params = {
        .Channel[0] = 8,
        .Channel[1] = 18,
        .Channel[2] = 28,
        .channelNum = 3,
        .CRCInit = 0x555555,
        .accessAddress = 0x17267162,
};

__attribute__((aligned(4)))  const char lwns_rf_params_index[]={"rcfg"};

/*********************************************************************
 * @fn      rf_config_params_init
 *
 * @brief   rf参数初始化，从flash中读取
 *
 * @param   None.
 *
 * @return  None.
 */
void rf_config_params_init(void){
    size_t  len;
    ef_get_env_blob(lwns_rf_params_index, NULL, 0, &len);
    if(len == sizeof(rf_config_params_t)){
        ef_get_env_blob(lwns_rf_params_index, &lwns_rf_params, sizeof(rf_config_params_t), NULL);
        if(lwns_rf_params.channelNum > 3){
            lwns_rf_params.channelNum = 3;//check params
        } else if(lwns_rf_params.channelNum == 0){
            lwns_rf_params.channelNum = 1;//check params
        }
        PRINTF("rf config channel num:%d ,Channel[0]:%d,Channel[1]:%d,Channel[2]:%d\n",
                lwns_rf_params.channelNum,lwns_rf_params.Channel[0],
                lwns_rf_params.Channel[1],lwns_rf_params.Channel[2]);
        PRINTF("rf config CRCInit:0x%06x\n",lwns_rf_params.CRCInit);
        PRINTF("rf config accessAddress:0x%08x\n",lwns_rf_params.accessAddress);
    } else {
        PRINTF("use default rf_params\n");
    }
}

/*********************************************************************
 * @fn      rf_config_params_save_to_flash
 *
 * @brief   lwns将rf参数保存到flash
 *
 * @param   to     -   密钥将保存到的缓冲区头指针.
 *
 * @return  defined in EfErrCode.
 */
EfErrCode rf_config_params_save_to_flash(void)
{
    EfErrCode err;
    err = ef_set_env_blob(lwns_rf_params_index, &lwns_rf_params, sizeof(rf_config_params_t));
    if(err != EF_NO_ERR){
        PRINTF("set rf params err\n");
    } else {
        PRINTF("set rf params ok\n");
    }
    return err;
}
