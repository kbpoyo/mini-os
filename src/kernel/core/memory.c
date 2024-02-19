#include "core/memory.h"

#include "common/boot_info.h"
#include "common/os_config.h"
#include "core/mmu.h"
#include "tools/bitmap.h"
#include "tools/klib.h"
#include "tools/log.h"

// TODO:暂时固定内存容量信息
static boot_info_t boot_info = {
    // .ram_region_cfg[0] = {.start = 0, .size = 4 * 1024},
    .ram_region_cfg[0] = {.start = 0x30000000, .size = 64 * 1024 * 1024},
    .ram_region_count = 1};

// 定义全局内存页分配对象
static addr_alloc_t paddr_alloc;

// 声明页目录表结构，并且使该页目录的起始地址按页大小对齐
// 页目录项的高12位为页表的物理地址位，及4096个子页表
static pde_t kernel_page_dir[PDE_CNT]
    __attribute__((aligned(FIRST_LEVEL_PAGE_TABLE_ALIGN)));

/**
 * @brief 获取页的索引
 *
 * @param alloc
 * @param page_addr
 */
static inline int page_index(addr_alloc_t *alloc, uint32_t page_addr) {
  return (page_addr - alloc->start) / alloc->page_size;
}

/**
 * @brief 为页的引用计数+1
 *
 * @param alloc
 * @param page_addr 页起始地址
 */
static inline void page_ref_add(addr_alloc_t *alloc, uint32_t page_addr) {
  // 计算出页的索引
  int index = page_index(alloc, page_addr);

  mutex_lock(&alloc->mutex);
  // 引用计数+1
  alloc->page_ref[index]++;

  mutex_unlock(&alloc->mutex);
}

/**
 * @brief 页的引用计数-1
 *
 * @param alloc
 * @param page_addr
 */
static inline void page_ref_sub(addr_alloc_t *alloc, uint32_t page_addr) {
  // 计算出页的索引
  int index = page_index(alloc, page_addr);

  mutex_lock(&alloc->mutex);
  // 引用计数-1
  if (alloc->page_ref[index] > 0) alloc->page_ref[index]--;

  mutex_unlock(&alloc->mutex);
}

/**
 * @brief 获取页的引用计数
 *
 * @param alloc
 * @param page_addr
 */
static inline int get_page_ref(addr_alloc_t *alloc, uint32_t page_addr) {
  // 计算出页的索引
  int index = page_index(alloc, page_addr);

  mutex_lock(&alloc->mutex);

  int ref = alloc->page_ref[index];

  mutex_unlock(&alloc->mutex);

  return ref;
}

/**
 * @brief 清除所有页的引用
 *
 * @param alloc
 * @return int
 */
static inline void clear_page_ref(addr_alloc_t *alloc) {
  mutex_lock(&alloc->mutex);

  kernel_memset(alloc->page_ref, 0, alloc->size / alloc->page_size);

  mutex_unlock(&alloc->mutex);
}

/**
 * @brief  初始化内存分配对象
 *
 * @param alloc 内存分配对象
 * @param bitmap_start 内存分配对象所管理的位图数组的起始地址
 * @param start 管理内存的起始地址
 * @param size 管理内存的大小
 * @param page_size 管理的内存页的大小
 */
static void addr_alloc_init(addr_alloc_t *alloc, uint8_t *bitmap_start,
                            uint32_t start, uint32_t size, uint32_t page_size) {
  mutex_init(&alloc->mutex);
  alloc->start = start;
  alloc->size = size;
  alloc->page_size = page_size;
  bitmap_init(&alloc->bitmap, bitmap_start, alloc->size / alloc->page_size, 0);
  // 清空页的引用数组
  kernel_memset(alloc->page_ref, 0, alloc->size / alloc->page_size);
}

/**
 * @brief  申请连续的内存页，并且起始页按align对齐
 *
 * @param alloc
 * @param page_count 申请页的数量
 * @return uint32_t 申请的第一个页的起始地址， 0：分配失败
 */
static uint32_t addr_alloc_page_align(addr_alloc_t *alloc, int page_count,
                                      int align) {
  uint32_t addr = 0;  // 记录分配的页的起始地址

  // TODO：加锁
  mutex_lock(&alloc->mutex);

  // 在位图中取连续的page_count个页进行分配
  int page_index = bitmap_alloc_nbits_align(&alloc->bitmap, 0, page_count,
                                            align / MEM_PAGE_SIZE);
  if (page_count >= 1 && page_index >= 0) {
    // 计算出申请到的第一个页的起始地址
    addr = alloc->start + page_index * alloc->page_size;
  }

  // TODO：解锁
  mutex_unlock(&alloc->mutex);

  return addr;
}
/**
 * @brief  申请连续的内存页
 *
 * @param alloc
 * @param page_count 申请页的数量
 * @return uint32_t 申请的第一个页的起始地址， 0：分配失败
 */
static uint32_t addr_alloc_page(addr_alloc_t *alloc, int page_count) {
  return addr_alloc_page_align(alloc, page_count,
                               1 * 1024);  // 默认按1kb对齐方式分配页
}

/**
 * @brief  释放连续内存页
 *
 * @param alloc
 * @param addr 第一个内存页的起始地址
 * @param page_count 内存页的数量
 */
static void addr_free_page(addr_alloc_t *alloc, uint32_t addr, int page_count) {
  mutex_lock(&alloc->mutex);

  // 将所有页引用-1
  for (int i = 0; i < page_count; ++i) {
    // 获取当前页的地址
    uint32_t page_addr = addr + i * MEM_PAGE_SIZE;
    // 引用-1
    page_ref_sub(alloc, page_addr);
    // 获取当前页引用
    int ref = get_page_ref(alloc, page_addr);
    if (ref == 0) {  // 引用为0，释放该页
      bitmap_set_bit(&alloc->bitmap, page_index(alloc, page_addr), 1, 0);
    }
  }

  mutex_unlock(&alloc->mutex);
}

/**
 * @brief  打印1m以内内存的可用空间
 *
 * @param boot_info
 */
static void show_mem_info(boot_info_t *boot_info) {
  log_printf("mem region:\n");
  for (int i = 0; i < boot_info->ram_region_count; ++i) {
    log_printf("[%d]: 0x%x - 0x%x\n", i, boot_info->ram_region_cfg[i].start,
               boot_info->ram_region_cfg[i].size);
  }

  log_printf("\n");
}

/**
 * @brief  计算总的可用内存空间大小
 *
 * @param boot_info
 * @return uint32_t
 */
static uint32_t total_mem_size(boot_info_t *boot_info) {
  uint32_t mem_size = 0;
  for (int i = 0; i < boot_info->ram_region_count; ++i) {
    mem_size += boot_info->ram_region_cfg[i].size;
  }

  return mem_size;
}

/**
 * @brief
 * 通过虚拟地址的高12位在页目录表中索引到对应的页目录项，及页目录项对应的页表
 *        再通过虚拟地址的次10位在索引到的页表中对应的页表项
 *
 * @param page_dir 被索引的页目录表
 * @param vstart  虚拟地址
 * @param is_alloc 若没有索引到对应的页表项
 *                 1：表示创建一个新页目录项，并为其分配一个新页作为页表
 *                 0: 返回0
 *
 * @return pte_t* 索引到的页表项
 */
pte_t *find_pte(pde_t *page_dir, uint32_t vstart, int is_alloc) {
  pte_t *page_table;  // 记录被索引的页表
  // 1.通过虚拟地址高10位索引到对应的页目录项
  pde_t *pde = page_dir + pde_index(vstart);

  // 2.判断该页目录项是否已存在，及该页目录项是否已指向一个被分配的页表
  if (pde->domain
          .flag) {  // 该页目录项存在，及存在对应的页表，可以索引到对应的页表
    page_table = (pte_t *)pde_to_pt_addr(pde);
  } else {  // 该目录项不存在内存中，及对应的页表不存在
    if (is_alloc == 0) {  // 不为该目录项创建对应的页表
      return (pte_t *)0;
    }

    // 为该目录项分配空间作为页表, 且页表基地址按4kb对齐
    uint32_t page_count = PTE_CNT * 4 / MEM_PAGE_SIZE;
    uint32_t pg_addr = addr_alloc_page_align(&paddr_alloc, page_count,
                                             SECOND_LEVEL_PAGE_TABLE_ALIGN);
    if (pg_addr == 0) {  // 分配失败
      return (pte_t *)0;
    }

    // 分配成功, 索引对应的页表
    page_table = (pte_t *)pg_addr;
    kernel_memset(page_table, 0, MEM_PAGE_SIZE * page_count);

    // 将该页表的起始地址放入对应的页目录项中并放入页目录表中，方便后续索引到该页表
    // 并将该页目录项对应的空间放入d0域且权限都放宽，即普通用户可访问，对应的页表的所有页可读写，将具体的权限交给每一页来进一步限制
    pde->v = pg_addr | PDE_FLAG | PDE_DOMAIN;
  }

  // log_printf("sizeof(pte_t) = %d", sizeof(pte_t));

  // 3.返回在该页表中索引到的页表项
  return page_table + pte_index(vstart);
}

/**
 * @brief 让虚拟地址和物理地址在页目录表中形成对照关系
 *        通过虚拟地址的高10位索引到对应的页目录项，及对应的页表
 *        再通过虚拟地址的次10位索引到页表中的页表项，及对应的页
 *
 * @param page_dir 页目录表的地址
 * @param vstart 虚拟地址的起始地址
 * @param pstart 物理地址的起始地址
 * @param page_count 分页数量
 * @param access_perim 该段虚拟地址的特权级
 * @return int -1:分配失败
 */
int memory_creat_map(pde_t *page_dir, uint32_t vstart, uint32_t pstart,
                     int page_count, uint32_t access_perim) {
  // 1.为每一页创建对应的页表项
  for (int i = 0; i < page_count; ++i) {
    // //打印调试信息
    // log_printf("creat map: v-0x%x, p-0x%x, access_perim:0x%x", vstart,
    // pstart,
    //            access_perim);

    // 2.通过虚拟地址在页目录表中获取对应的页目录项指向的页表,
    // 且当没有该页目录项时，为其分配一个页作为页表并让一个目录项指向该页表
    pte_t *pte = find_pte(page_dir, vstart, 1);
    if (pte == (pte_t *)0) {  // 没有找到可用的页表项
      log_printf("creat pte failed pte == 0\n");
      return -1;
    }

    // log_printf("pte addr : 0x%x", (uint32_t)pte);
    // 3.确保该页无效，即未分配
    ASSERT(pte->domain.flag == 0);

    // 4.在页表项中创建对应的映射关系，并该页权限，页权限以当前权限为主，因为pde处已放宽权限
    pte->v = pstart | access_perim | PTE_FLAG;

    // 5.将该页引用计数+1
    page_ref_add(&paddr_alloc, pstart);

    // 6.切换为下一页
    vstart += MEM_PAGE_SIZE;
    pstart += MEM_PAGE_SIZE;
  }

  return 1;
}

/**
 * @brief 创建内核的虚拟页表
 *
 */
void create_kernal_table(void) {
  // 清空kernal_page_dir
  kernel_memset(kernel_page_dir, 0, PDE_CNT * 4);

  // 声明内核只读段的起始与结束地址和数据段的起始地址
  extern char s_text, e_text, s_data;

  // 对内核的虚拟空间进行一一映射，即物理地址=虚拟地址，以预防使能mmu时的未知错误
  static memory_map_t kernal_map[] = {
      {SDRAM_INSIDE_START, SDRAM_INSIDE_START + SDRAM_INSIDE_SIZE,
       SDRAM_INSIDE_START, PTE_AP_SYS},  // 内部4kb的空间映射关系，即0x0~0x1000
      {&s_text, &e_text, &s_text,
       PTE_AP_SYS},  // 只读段的映射关系(内核.text和.rodata段)
      {&s_data, (void *)MEM_EXT_START, &s_data,
       PTE_AP_SYS},  // 可读写段的映射关系
      {(void *)MEM_EXT_START, (void *)MEM_EXT_END, (void *)MEM_EXT_START,
       PTE_AP_SYS},  // 将sdram基地址 + 1mb以上的空间都映射给操作系统使用
      {(void *)MEM_UART_START, (void *)MEM_UART_END, (void *)MEM_UART_START,
       PTE_AP_SYS},  // 映射串口相关寄存器地址范围
      {(void *)MEM_IRQ_START, (void *)MEM_IRQ_END, (void *)MEM_IRQ_START,
       PTE_AP_SYS},  // 映射中断相关寄存器地址范围}
      {(void *)MEM_TIMER_START, (void *)MEM_TIMER_END, (void *)MEM_TIMER_START,
       PTE_AP_SYS}};  // 映射定时器相关寄存器地址范围};

  for (int i = 0; i < sizeof(kernal_map) / sizeof(kernal_map[0]); ++i) {
    memory_map_t *map = kernal_map + i;

    // 将虚拟地址的起始地址按页大小对齐，为了不丢失原有的虚拟地址空间，所以向下对齐vstart
    // 理论上虚拟地址是不需要上下边缘对齐的，这里主要是为了计算所需页数
    // 因为虚拟地址的每一页都和页目录项以及页表项捆绑了，
    // 只需用页目录项和页表项为该页映射一个物理页即可，所以物理页才必须上下边缘按4kb对齐
    uint32_t vstart = down2((uint32_t)map->vstart, MEM_PAGE_SIZE);
    uint32_t pstart = down2((uint32_t)map->pstart, MEM_PAGE_SIZE);
    // 将虚拟地址的结束地址按页大小4kb对齐,
    // 为了不丢失原有的虚拟地址空间，所以向上对齐vend
    uint32_t vend = up2((uint32_t)map->vend, MEM_PAGE_SIZE);
    // 计算该虚拟空间需要的页数
    int page_count = (vend - vstart) / MEM_PAGE_SIZE;

    // 创建内存映射关系
    memory_creat_map(kernel_page_dir, vstart, pstart, page_count,
                     map->access_perim);
    // 清空内核空间对页的引用
    clear_page_ref(&paddr_alloc);
  }
}

/**
 * @brief  初始化化内存
 *
 */
void memory_init() {
  // 声明紧邻内核first_task段后面的空间地址，用于存储位图，该变量定义在kernel.lds中
  extern char mem_kernel_end;

  log_printf("memory init...\n");

  log_printf("mem_kernel_end: 0x%x\n", &mem_kernel_end);

  show_mem_info(&boot_info);

  // 去除低1mb大小后可用的内存空间大小
  uint32_t mem_up1MB_free =
      total_mem_size(&boot_info) - (MEM_EXT_START - SDRAM_START);

  // 将可用空间大小下调到页大小的整数倍
  mem_up1MB_free = down2(mem_up1MB_free, MEM_PAGE_SIZE);

  log_printf("free memory: 0x%x, size: 0x%x\n", MEM_EXT_START, mem_up1MB_free);

  // mem_free_start被分配的地址在链接文件中定义，紧邻着first_task段
  uint8_t *mem_free = (uint8_t *)&mem_kernel_end;

  // 用paddr_alloc，内存页分配对象管理1mb以上的所有空闲空间，页大小为MEM_PAGE_SIZE=4kb
  addr_alloc_init(&paddr_alloc, mem_free, MEM_EXT_START, mem_up1MB_free,
                  MEM_PAGE_SIZE);

  // 跳过存储位图的内存区域,
  // 位图的每一位表示一个页，计算位图所站的字节数即可跳过该区域
  mem_free += bitmap_byte_count(paddr_alloc.size / MEM_PAGE_SIZE);

  log_printf("bitmap start addr: 0x%x, end addr: 0x%x\n", &mem_kernel_end,
             mem_free);

  // 判断mem_free是否已越过可用数据区
  ASSERT(mem_free < (uint8_t *)MEM_EXT_START);

  // 创建内核的页表映射
  create_kernal_table();

  // 设置内核的页目录表到CR2寄存器，并开启分页机制
  mmu_set_page_dir(kernel_page_dir);
  // 使能mmu
  enable_mmu();

  log_printf("memory init success...\n");
}

/**
 * @brief 为进程的内核空间分配一页内存，需特权级0访问
 *
 * @return uint32_t 内存的起始地址
 */
uint32_t memory_alloc_page() {
  // 操作系统内核已对整个内存空间进行了一一映射，而每个程序的2gb以下空间都使用操作系统的虚拟页表
  // 所以直接返回该页物理地址，也就是该页在操作系统虚拟地址空间中的虚拟地址
  // 需要注意的是后续访问该页需要sys特权级，因为访问的是内核空间
  uint32_t addr = addr_alloc_page(&paddr_alloc, 1);
  return addr;
}

/**
 * @brief 返回当前进程的页目录表的地址
 *
 * @return pde_t*
 */
static pde_t *curr_page_dir() { return (pde_t *)(task_current()->page_dir); }

/**
 * @brief 释放一页内存空间
 *
 * @param addr
 */
void memory_free_page(uint32_t addr) {
  if (addr < MEM_TASK_BASE) {  // 释放内核空间的一页内存
    addr_free_page(
        &paddr_alloc, addr,
        1);  // 因为内核空间为一一映射关系，虚拟地址即为物理地址,且不需要解除映射关系
  } else {   // 释放用户空间的一页内存
    // 1.用虚拟地址找到该页对应的页表项
    pte_t *pte = find_pte(curr_page_dir(), addr, 0);
    ASSERT(pte != (pte_t *)0 && pte->domain.flag);

    // 2.用该页的物理地址释放该页
    addr_free_page(&paddr_alloc, pte_to_pg_addr(pte), 1);

    // 3.将页表项清空，解除映射关系
    pte->v = 0;
  }
}