#include "core/irq.h"
#include "tools/log.h"
#include "common/types.h"
/**
 * @brief 初始化irq中断向量表
 *
 */
void irq_init() {
    log_printf("irq init start......\n");
    //设置中断模式，全部为irq
    rINTMOD = 0x0;

    //设置中断优先级,0~6个中断发生模块全部启用轮询
    rPRIORITY = 0x7f;

    irq_clear_all();

    irq_enable(EINT8_PRIM, EINT8_SUB);

    log_printf("irq init success......\n");
}

/**
 * @brief 开中断
 * 
 */
void irq_start() {


}

/**
 * @brief 关中断
 * 
 */
void irq_close() {

}

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
        if (irq_num_prim == EINT4_27 || irq_num_prim == EINT8_23) { //使能外部中断屏蔽寄存器
            rEINTMASK = ((~(1 << irq_num_sub)) & rEINTMASK);
        } else {    //使能次中断源屏蔽寄存器
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
        if (irq_num_prim == EINT4_27 || irq_num_prim == EINT8_23) {
            rEINTMASK = ((1 << irq_num_sub) | rEINTMASK);   //操作外部中断屏蔽寄存器
        } else {//操作次中断源屏蔽寄存器
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
        if (irq_num_prim == EINT4_27 || irq_num_prim == EINT8_23) {
            rEINTPEND = ((1 << irq_num_sub) & rEINTPEND);
        } else {
            rSUBSRCPND = ((1 << irq_num_sub) & rSUBSRCPND);
        }
    }

    if (irq_num_prim >= 0) {
        rSRCPND = ((1 << irq_num_prim) & rSRCPND);
        rINTPND = ((1 << irq_num_prim) & rINTPND);
    }
}

/**
 * @brief 清楚全部中断
 *
 */
void irq_clear_all() {
    //先清除次级中断挂起寄存器和外部中断挂起寄存器
    rSUBSRCPND = 0xffffffff;
    rEINTPEND = 0xffffffff;
    
    //再清除源中断挂起寄存器的全部位
    rSRCPND = 0xffffffff;

    //再清除中断未决寄存器的全部位
    rINTPND = 0xffffffff;
}


/**
 * @brief 中断处理函数
 * 
 */
void irq_handler() {
   int irq_num = rINTOFFSET; 

    log_printf("irq_num = %d\n", irq_num);
    irq_clear(irq_num, NOSUBINT); 
    irq_clear_all(); 
}