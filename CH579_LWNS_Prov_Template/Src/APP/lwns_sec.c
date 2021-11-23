/*
 * lwns_sec.c
   *  消息加密
 *  Created on: Sep 17, 2021
 *      Author: WCH
 */

#include "lwns_sec.h"
#include "config.h"

#define DEBUG_PRINT_IN_THIS_FILE 1
#if DEBUG_PRINT_IN_THIS_FILE
#define PRINTF(...) PRINT(__VA_ARGS__)
#else
#define PRINTF(...) do {} while (0)
#endif


//用户在此修改，为默认的lwns_sec_key参数，第一次运行程序，flash中不会存有此参数，所以默认采用此参数进行加密解密
//通过配网，用户将新的lwns_sec_key参数写入flash中，即完成配网操作
static uint8_t lwns_sec_key[16]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};//用户自行更改为自己的秘钥，或者改为可以从主机获取，并存储在flash中

__attribute__((aligned(4)))  const char lwns_rf_key_index[]={"skey"};

/*********************************************************************
 * @fn      lwns_sec_init
 *
 * @brief   lwns消息加密密钥初始化，从flash中读取出密钥
 *
 * @param   None.
 *
 * @return  None.
 */
void lwns_sec_init(void){
    size_t  len;
    ef_get_env_blob(lwns_rf_key_index, NULL, 0, &len);
    if(len == 16){
        ef_get_env_blob(lwns_rf_key_index, lwns_sec_key, 16, NULL);
    } else {
        PRINTF("use default rf_key\n");
    }
    PRINTF("rf sec key:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n"
            ,lwns_sec_key[0],lwns_sec_key[1],lwns_sec_key[2],lwns_sec_key[3]
            ,lwns_sec_key[4],lwns_sec_key[5],lwns_sec_key[6],lwns_sec_key[7]
            ,lwns_sec_key[8],lwns_sec_key[9],lwns_sec_key[10],lwns_sec_key[11]
            ,lwns_sec_key[12],lwns_sec_key[13],lwns_sec_key[14],lwns_sec_key[15]);
}

/*********************************************************************
 * @fn      lwns_msg_encrypt
 *
 * @brief   lwns消息加密
 *
 * @param   src     -   待加密的数据缓冲头指针.
 * @param   to      -   待存储加密数据的数据缓存区头指针.
 * @param   mlen    -   待加密的数据长度.
 *
 * @return  加密后的数据长度.
 */
int lwns_msg_encrypt(uint8_t *src,uint8_t *to,uint8_t mlen){
    uint16_t i = 0;
    uint8_t esrc[16] = {0};
    while(1){
        if((mlen - i) < 16){
            tmos_memcpy(esrc, src + i, (mlen - i));//扩充到16字节，其他为0
            LL_Encrypt(lwns_sec_key, esrc , to + i);
        } else {
            LL_Encrypt(lwns_sec_key, src + i , to + i);
        }
        i+=16;
        if(i >= mlen){
            break;
        }
    }
    return i;//返回加密后数据长度
}

/*********************************************************************
 * @fn      lwns_msg_decrypt
 *
 * @brief   lwns消息解密
 *
 * @param   src     -   待解密的数据缓冲头指针.
 * @param   to      -   待存储解密数据的数据缓存区头指针.
 * @param   mlen    -   待解密的数据长度.
 *
 * @return  解密后的数据长度.
 */
int lwns_msg_decrypt(uint8_t *src,uint8_t *to,uint8_t mlen){//长度mlen必须为16的倍数
    uint16_t i = 0;
    while(1){
        LL_Decrypt(lwns_sec_key, src + i, to + i);
        i+=16;
        if(i >= mlen){
            break;
        }
    }
    return i;
}

/*********************************************************************
 * @fn      lwns_temp_set_key
 *
 * @brief   lwns设置临时密钥，不存入flash
 *
 * @param   keyValue     -   待设置的密钥值缓冲头指针.
 *
 * @return  None.
 */
void lwns_temp_set_key(uint8_t* keyValue)
{
    tmos_memcpy(lwns_sec_key, keyValue, 16);
}

/*********************************************************************
 * @fn      lwns_get_key
 *
 * @brief   lwns获取密钥
 *
 * @param   to     -   密钥将保存到的缓冲区头指针.
 *
 * @return  None.
 */
void lwns_get_key(uint8_t* to){
    tmos_memcpy(to, lwns_sec_key, 16);
}

/*********************************************************************
 * @fn      lwns_save_key_to_flash
 *
 * @brief   lwns将临时密钥保存到flash
 *
 * @param   to     -   密钥将保存到的缓冲区头指针.
 *
 * @return  defined in EfErrCode.
 */
EfErrCode lwns_save_key_to_flash(void)
{
    EfErrCode err;
    err = ef_set_env_blob(lwns_rf_key_index, lwns_sec_key, 16);
    if(err != EF_NO_ERR){
        PRINTF("set rf key err\n");
    } else {
        PRINTF("set rf key ok\n");
    }
    return err;
}
