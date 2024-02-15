#ifndef MMU_H
#define MMU_H

#include "common/cpu_instr.h"
#include "common/types.h"

// TODO:等后续做缺页异常处理，
//      进行内存和磁盘之间的页面调度时
//       再控制Cr8寄存器进行快表内容的使无效

/**
 * 协处理器cr0(只读)的cache标识符为0xd172172
 * [28:25] = 0b0110:
 *      cache类型为：写回类型
 *      cache内容的清除方法：用寄存器cr7定义
 *      cache内容锁定方法：支持格式A
 * [23:12] = 0x172(数据cache的属性):
 *      块大小：32字节
 *      M: 0
 *      cache容量：16kb
 *      cache相关属性：64路相关
 *
 * [11:0] = 0x172(指令cache的属性):
 *      块大小：32字节
 *      M: 0
 *      cache容量：16kb
 *      cache相关属性：64路相关
 *
 */

// 定义cr1寄存器的位域
#define CR1_MMU_ENABEL (0x1 << 0)           // 使能mmu
#define CR1_DATA_CACHE_ENABLE (0x1 << 2)    // 使能数据cacahe
#define CR1_INSTR_CACHE_ENABLE (0x1 << 12)  // 使能指令cache

// 定义cr3寄存器的位域
#define CR3_D0 (1 << 0)  // 将D0域的权限控制设置为只由页表项的AP位确定

/**
 * 映射关系为：
 *      4GB = 4096x1mb(4096个页目录项)
 *      1MB = 1024x1kb(1024个页表项)
 *
 *所有空间都映射到cr3寄存器的0号域
 */

// 一级页表和二级页表基地址对齐要求
#define FIRST_LEVEL_PAGE_TABLE_ALIGN (16 * 1024)
#define SECOND_LEVEL_PAGE_TABLE_ALIGN (4 * 1024)

// 页表大小
#define MEM_PAGE_SIZE 1024

#define PDE_CNT \
  4096  // 页目录项的个数,一个页目录表映射整个4gb空间大小，每个页目录项映射1mb空间大小，所以需要4096个页目录项
#define PTE_CNT \
  1024  // 页表录项的个数,每个二级页表映射1mb的空间大小，每个页表项4字节，映射1kb空间大小,所以需要1024个页表项

// 定义页目录项相关的宏(细粒度二级页表)
#define PDE_FLAG \
  (3 << 0)  // 页目录项标识符，标志对应的为细粒度二级页表即1024x1kb

// 域标识符，标识页目录项所对应的1mb虚拟空间所在域,我就把所有空间放在0号域
#define PDE_DOMAIN (0 << 5)

// 定义页表项相关的宏(极小页1kb)
#define PTE_FLAG (3 << 0)  // 页标识符，极小页1kb为0b11
#define PTE_B (1 << 2)     // 页的写缓冲使能位
#define PTE_C (1 << 3)     // 页的cache使能位
// 页的访问权限控制位
#define PTE_AP_SYS (1 << 4)  // 只能特权级模式访问
#define PTE_AP_USR (3 << 4)  // 用户与特权模式都可访问
#define PTE_AP_USR_READONLY (2 << 4)  // 用户与特权模式都可访问,但用户模式只读

#pragma pack(1)

// csapp p578
// 定义页目录项PDE结构的联合体
typedef union _pde_t {
  uint32_t v;
  struct {
    uint32_t
        flag : 2;  // 页目录项标识位，标识二级页表类型，我只用了细粒度二级页表(0b11)
    uint32_t user_defing : 3;   // 用户自定义位
    uint32_t domain : 4;        // 本页目录项对应的1mb空间所属域
    uint32_t invalid_hold : 3;  // 无效保留位
    uint32_t phy_pt_addr : 20;  // 高20位，页表的物理地址
  } domain;

} pde_t;

// 定义页表项PTE结构的联合体
typedef union _pte_t {
  uint32_t v;
  struct {
    uint32_t flag : 2;  // flag标识位，标识页的类型，我只使用极小页(0b11)
    uint32_t write_buffer_enable : 1;  // 写缓冲使能位
    uint32_t cache_enable : 1;         // cache使能位
    uint32_t access_perm : 2;          // 访问权限控制位
    uint32_t invalid_hold : 4;         // 无效保留位
    uint32_t phy_page_addr : 22;       // 高22位，页的物理地址
  } domain;

} pte_t;

#pragma pack()

/**
 * @brief 获取虚拟地址的高12位[31:20]，即对应的页目录项在页目录表中的索引
 *
 * @param vstart
 * @return uint32_t
 */
static inline uint32_t pde_index(uint32_t vstart) { return (vstart >> 20); }

/**
 * @brief 获取虚拟地址的次10位[19:10]，及对应的页表项在页表中的索引
 *
 * @param vstart
 * @return uint32_t
 */
static inline uint32_t pte_index(uint32_t vstart) {
  return (vstart >> 10) & 0x3ff;
}

/**
 * @brief 获取页目录项中对应的页表的起始地址，及该页表第一个页表项的地址
 *
 * @param pde 页目录项
 * @return uint32_t 返回的页表的地址
 */
static inline uint32_t pde_to_pt_addr(pde_t *pde) {
  // 高20位为页表的物理地址的有效位，将其左移12位，及按4kb对齐后才是该页的物理地址
  return pde->domain.phy_pt_addr << 12;
  // return pde->phy_pt_addr << 12;
}

/**
 * @brief 获取页表项中对应的页的起始地址
 *
 * @param pte 页表项
 * @return uint32_t 返回的页的地址
 */
static inline uint32_t pte_to_pg_addr(pte_t *pte) {
  // 高20位为页的物理地址有效位，将其左移10位，及按1kb对齐后才是该页的物理地址
  return pte->domain.phy_page_addr << 10;
}

/**
 * @brief 获取页表项的权限
 *
 * @param pte
 * @return uint32_t
 */
static inline uint32_t get_pte_privilege(pte_t *pte) {
  return pte->v & 0x3f;  // 直接获取低6位即为所有权限
}

/**
 * @brief 设置页目录表的起始地址，将起始地址写入CR3寄存器
 *
 * @param paddr 页目录表的物理起始地址
 */
static inline void mmu_set_page_dir(uint32_t paddr) {
  // 设置cr2寄存器的高20位为页目录表的地址，因为按4kb对齐，所以
  // 页目录表的起始地址page_dir的高20位才为有效位，低12位为0，将cr3的低12位就设置为0
  cpu_cr2_write(paddr);
}

void enable_mmu();

#endif