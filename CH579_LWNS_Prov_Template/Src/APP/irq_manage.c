/*
 * irq_manage.c
 *  中断管理
 *  Created on: Sep 17, 2021
 *      Author: WCH
 */
#include "irq_manage.h"

static uint32_t irq_status = 0;

/*********************************************************************
 * @fn      irq_disable_all
 *
 * @brief   关闭所有中断，保存中断值.
 *
 * @param   None.
 *
 * @return  None.
 */
void irq_disable_all(void) {
    SYS_DisableAllIrq(&irq_status);
}

/*********************************************************************
 * @fn      irq_enable
 *
 * @brief   恢复所有中断.
 *
 * @param   None.
 *
 * @return  None.
 */
void irq_enable(void) {
    SYS_RecoverIrq(irq_status);
}
