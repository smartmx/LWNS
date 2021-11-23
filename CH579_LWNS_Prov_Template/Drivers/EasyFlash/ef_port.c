/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2015-2019, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-01-16
 */

#include "easyflash.h"
#include <stdarg.h>
#include "stdio.h"
#include "string.h"
#include "irq_manage.h"
#include "config.h"
#include "rf_config_params.h"

#define DEBUG_PRINT_IN_THIS_FILE 1
#if DEBUG_PRINT_IN_THIS_FILE
#define PRINTF(...) PRINT(__VA_ARGS__)
#define VPRINTF    vprintf
#else
#define PRINTF(...) do {} while (0)
#define VPRINTF(...) do {} while (0)
#endif


/* default environment variables set for user */
static ef_env default_env_set[] = {
        { (char*)lwns_rf_params_index, (void *)(&lwns_rf_params), sizeof(rf_config_params_t)},
};

/**
 * Flash port for hardware initialize.
 *
 * @param default_env default ENV set for user
 * @param default_env_size default ENV size
 *
 * @return result
 */
EfErrCode ef_port_init(ef_env **default_env, size_t *default_env_size) {
    EfErrCode result = EF_NO_ERR;

    *default_env = default_env_set;
    *default_env_size = sizeof(default_env_set) / sizeof(default_env_set[0]);
		ef_print("ef_port_init\n");
    return result;
}

/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
EfErrCode ef_port_read(uint32_t addr, uint32_t *buf, size_t size) {
    EfErrCode read_result = EF_NO_ERR;
    //ef_print("ef_port_read:%04x,%04x,%04x\n",addr,buf,size);
		tmos_memcpy(buf,(const void *)addr,size);
    return read_result;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
EfErrCode ef_port_erase(uint32_t addr, size_t size) {
    EfErrCode ef_port_erase_result = EF_NO_ERR;
    size_t erase_size = 0;
    /* make sure the start address is a multiple of FLASH_ERASE_MIN_SIZE */
    EF_ASSERT(addr % EF_ERASE_MIN_SIZE == 0);
    //ef_print("ef_port_erase:0x%04x,0x%04x\n", addr, size);
		while(erase_size < size){
			if(FlashBlockErase(addr + erase_size) != 0){
				ef_port_erase_result = EF_ERASE_ERR;
				break;
			}
			erase_size += EF_ERASE_MIN_SIZE;
		}
    return ef_port_erase_result;
}
/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
EfErrCode ef_port_write(uint32_t addr, const uint32_t *buf, size_t size) {
    EfErrCode result = EF_NO_ERR;
		//ef_print("ef_port_write:0x%04x,0x%04x,0x%04x\n", addr, buf, size);
		if((uint32_t)buf % 4 == 0){
			//buf四字节对齐
			if (FlashWriteBuf(addr, (PUINT32)buf, size) != 0) {
					result = EF_WRITE_ERR;
			}
		}else{
			//buf非四字节对齐
			size_t  w_size = 0;
			uint32_t write_data[1];
			while(w_size < size){
				tmos_memcpy(write_data, (const void *)((uint32_t)buf + w_size), 4);
				if(FlashWriteDW(addr + w_size, write_data[0]) != 0){
						result = EF_WRITE_ERR;
						break;
				}
				w_size = w_size + 4;
			}
		}
    return result;
}

/**
 * lock the ENV ram cache
 */
void ef_port_env_lock(void) {
    /* You can add your code under here. */
    irq_disable_all();
}

/**
 * unlock the ENV ram cache
 */
void ef_port_env_unlock(void) {
    /* You can add your code under here. */
    irq_enable();
}

/**
 * This function is print flash debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 *
 */
void ef_log_debug(const char *file, const long line, const char *format, ...) {
    va_list args;
    /* args point to the first variable parameter */
    va_start(args, format);
    PRINTF("[DBG](%s:%ld) ", file, line);
    VPRINTF(format, args);
    PRINTF("\n");
    va_end(args);
}

/**
 * This function is print flash routine info.
 *
 * @param format output format
 * @param ... args
 */
void ef_log_info(const char *format, ...) {
    va_list args;
    /* args point to the first variable parameter */
    va_start(args, format);
    PRINTF("[LOG]");
    VPRINTF(format, args);
    PRINTF("\n");
    va_end(args);
}
/**
 * This function is print flash non-package info.
 *
 * @param format output format
 * @param ... args
 */
void ef_print(const char *format, ...) {
    va_list args;
    /* args point to the first variable parameter */
    va_start(args, format);
    VPRINTF(format, args);
    va_end(args);
}
