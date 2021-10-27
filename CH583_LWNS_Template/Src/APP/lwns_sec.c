/*
 * lwns_sec.c
   *  消息加密
 *  Created on: Sep 17, 2021
 *      Author: WCH
 */

#include "lwns_sec.h"
#include "config.h"

static unsigned char lwns_sec_key[16]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};//用户自行更改为自己的秘钥，或者改为可以从主机获取，并存储在eeprom中

//加密消息，将src开始的mlen个字节数据，加密到to指向的内存空间。
int lwns_msg_encrypt(u8 *src,u8 *to,u8 mlen){
    unsigned short i = 0;
    unsigned char esrc[16] = {0};
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

//解密消息，将src开始的mlen个字节数据，解密到to指向的内存空间。
int lwns_msg_decrypt(u8 *src,u8 *to,u8 mlen){//长度mlen必须为16的倍数
    unsigned short i = 0;
    while(1){
        LL_Decrypt(lwns_sec_key, src + i, to + i);
        i+=16;
        if(i >= mlen){
            break;
        }
    }
    return i;
}
