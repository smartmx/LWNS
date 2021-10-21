/*
 * irq_manage.c
 *  ÷–∂œπ‹¿Ì
 *  Created on: Sep 17, 2021
 *      Author: WCH
 */
#include "irq_manage.h"

static UINT32 irq_status = 0;

void irq_disable_all(void) {
    SYS_DisableAllIrq(&irq_status);
}


void irq_enable(void) {
    SYS_RecoverIrq(irq_status);
}
