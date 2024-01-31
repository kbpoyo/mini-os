#include "core/irq.h"

#include "common/types.h"
#include "tools/assert.h"
#include "tools/log.h"

static irq_handler_t irq_handler_call[IRQ_NUM_MAX];


/**
 * @brief 使能某一中断
 *
 * @param irq_num_prim 主中断源号
 * @param irq_num_sub 次中断源号
 */
void irq_enable(int irq_num_prim, int irq_num_sub) {
    if (irq_num_prim >= 0) {
        rINTMSK = ((~(1 << irq_num_prim)) & rINTMSK);
    }

    if (irq_num_sub >= 0) {
        if (irq_num_prim == EINT4_7 || irq_num_prim == EINT8_23) {  // 使能外部中断屏蔽寄存器
            rEINTMASK = ((~(1 << irq_num_sub)) & rEINTMASK);
        } else {  // 使能次中断源屏蔽寄存器
            rINTSUBMSK = ((~(1 << irq_num_sub)) & rINTSUBMSK);
        }
    }
}

/**
 * @brief 屏蔽某一中断
 *
 * @param irq_num_prim 主中断源号
 * @param irq_num_secon 次中断源号
 */
void irq_disable(int irq_num_prim, int irq_num_sub) {
    if (irq_num_prim >= 0) {
        rINTMSK = ((1 << irq_num_prim) | rINTMSK);
    }

    if (irq_num_sub >= 0) {
        if (irq_num_prim == EINT4_7 || irq_num_prim == EINT8_23) {
            rEINTMASK = ((1 << irq_num_sub) | rEINTMASK);  // 操作外部中断屏蔽寄存器
        } else {                                           // 操作次中断源屏蔽寄存器
            rINTSUBMSK = ((1 << irq_num_sub) | rINTSUBMSK);
        }
    }
}

/**
 * @brief 使能全部中断
 *
 */
void irq_enable_all() {
    rINTSUBMSK = 0x0;
    rEINTMASK = 0x0;
    rINTMSK = 0x0;
}

/**
 * @brief 屏蔽全部中断
 *
 */
void irq_disable_all() {
    rINTMSK = 0xffffffff;
    rINTSUBMSK = 0xffffffff;
    rEINTMASK = 0xffffffff;
}

/**
 * @brief 清楚某一中断
 *
 * @param irq_num
 */
void irq_clear(int irq_num_prim, int irq_num_sub) {
    if (irq_num_sub >= 0) {
        if (irq_num_prim == EINT4_7 || irq_num_prim == EINT8_23) {//清楚外部中断挂起寄存器的相应位
            rEINTPEND = ((1 << irq_num_sub) & rEINTPEND);
        } else {//清除次级中断源挂起寄存器的相应位
            rSUBSRCPND = ((1 << irq_num_sub) & rSUBSRCPND);
        }
    }

    if (irq_num_prim >= 0) {    //清除主中断源挂起寄存器和挂起寄存器
        rSRCPND = ((1 << irq_num_prim) & rSRCPND);
        rINTPND = ((1 << irq_num_prim) & rINTPND);
    }
}

/**
 * @brief 清楚全部中断
 *
 */
void irq_clear_all() {
    // 先清除次级中断挂起寄存器和外部中断挂起寄存器
    rSUBSRCPND = 0xffffffff;
    rEINTPEND = 0xffffffff;

    // 再清除源中断挂起寄存器的全部位
    rSRCPND = 0xffffffff;

    // 再清除中断未决寄存器的全部位
    rINTPND = 0xffffffff;
}

/**
 * @brief 中断处理函数
 *
 */
void irq_handler() {
    int irq_num = rINTOFFSET;


    irq_handler_call[irq_num]();
}

/**
 * @brief 为中断向量号注册中断函数
 *
 * @param irq_num
 * @param handler_for_irq
 */
void irq_handler_register(int irq_num, irq_handler_t handler_for_irq) {
    ASSERT(irq_num >= 0 && irq_num < IRQ_NUM_MAX);

    irq_handler_call[irq_num] = handler_for_irq;
}

/**
 * @brief 外部中断8-23的中断处理函数
 *
 */
void irq_handler_for_eint8_23() {
    ASSERT(rINTOFFSET = EINT8_23);

    if (((1 << EINT8_SUB) & rEINTPEND) && !((1 << EINT8_SUB) & rEINTMASK)) {  // EINT8触发成功
        log_printf("EINT8 has complete!\n");

        // 清除中断
        irq_clear(EINT8_PRIM, EINT8_SUB);
    }

    if (((1 << EINT11_SUB) & rEINTPEND) && !((1 << EINT11_SUB) & rEINTMASK)) {  // EINT11触发成功
        log_printf("EINT11 has complete!\n");

        irq_clear(EINT11_PRIM, EINT11_SUB);
    }
}

/**
 * @brief 初始化irq中断向量表
 *
 */
void irq_init() {
    log_printf("irq init start......\n");
    // 设置中断模式，全部为irq
    rINTMOD = 0x0;

    // 设置中断优先级,0~6个中断发生模块全部启用轮询
    rPRIORITY = 0x7f;

    irq_clear_all();

    irq_enable(EINT8_PRIM, EINT8_SUB);
    irq_enable(EINT11_PRIM, EINT11_SUB);

    // 注册中断处理函数
    irq_handler_register(EINT8_23, irq_handler_for_eint8_23);

    log_printf("irq init success......\n");
}