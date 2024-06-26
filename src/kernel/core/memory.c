#include "core/memory.h"

#include "common/boot_info.h"
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
static addr_alloc_t paddr_alloc __attribute__((section(".data"), aligned(4)));

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
 * @brief 检测页索引是否越界
 *
 * @param alloc
 * @param index
 * @return int
 */
static inline int page_index_err(addr_alloc_t *alloc, int index) {
  if (index < 0 || index >= alloc->size / alloc->page_size) {
    return 1;
  }

  return 0;
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

  if (page_index_err(alloc, index)) {  // 索引超范围
    return 0;
  }

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

  if (page_index_err(alloc, index)) {  // 索引超范围
    return 0;
  }

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

  if (page_index_err(alloc, index)) {  // 索引超范围
    return 0;
  }

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
    uint32_t page_count = PTE_CNT * sizeof(pte_t) / MEM_PAGE_SIZE;
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
  kernel_memset(kernel_page_dir, 0, PDE_CNT * sizeof(pde_t));

  // 声明内核只读段的起始与结束地址和数据段的起始地址
  extern char s_text, e_text, s_data;

  // 对内核的虚拟空间进行一一映射，即物理地址=虚拟地址，以预防使能mmu时的未知错误
  static memory_map_t kernal_map[] = {
      {SDRAM_INSIDE_START, SDRAM_INSIDE_START + SDRAM_INSIDE_SIZE,
       SDRAM_INSIDE_START,
       PTE_AP_SYS | PTE_C},  // 内部4kb的空间映射关系，即0x0~0x1000
      {&s_text, &e_text, &s_text,
       PTE_AP_SYS | PTE_C},  // 只读段的映射关系(内核.text和.rodata段)
      {&s_data, (void *)MEM_EXT_START, &s_data,
       PTE_AP_SYS | PTE_C},  // 可读写段的映射关系
      {(void *)MEM_EXT_START, (void *)MEM_EXT_END, (void *)MEM_EXT_START,
       PTE_AP_SYS |
           PTE_C},  // 将sdram基地址 + 1mb以上的空间都映射给操作系统使用
      {(void *)MEM_UART_START, (void *)MEM_UART_END, (void *)MEM_UART_START,
       PTE_AP_SYS},  // 映射串口相关寄存器地址范围
      {(void *)MEM_IRQ_START, (void *)MEM_IRQ_END, (void *)MEM_IRQ_START,
       PTE_AP_SYS},  // 映射中断相关寄存器地址范围
      {(void *)MEM_TIMER_START, (void *)MEM_TIMER_END, (void *)MEM_TIMER_START,
       PTE_AP_SYS},  // 映射定时器相关寄存器地址范围
      {(void *)MEM_GPIO_START, (void *)MEM_GPIO_END, (void *)MEM_GPIO_START,
       PTE_AP_SYS},  // 映射gpio相关寄存器地址范围
      {(void *)MEM_NADNFLASH_START, (void *)MEM_NANDFLASH_END,
       (void *)MEM_NADNFLASH_START,
       PTE_AP_SYS},  // 映射nandflash相关寄存器地址范围
      {(void *)MEM_SD_START, (void *)MEM_SD_END, (void *)MEM_SD_START,
       PTE_AP_SYS}  // 映射SD控制器相关寄存器组
  };

  // memory_show_bitmap();

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
  }

  // 清空内核空间对页的引用
  clear_page_ref(&paddr_alloc);
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
  ASSERT(mem_free < ((uint8_t *)MEM_EXT_START - 2 * STACK_SVC_SIZE));

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
uint32_t memory_alloc_page(int page_count) {
  uint32_t addr = addr_alloc_page(&paddr_alloc, page_count);
  return addr;
}

/**
 * @brief 为进程的内核空间分配page_count页内存
 * 并让起始页按align对齐
 *
 * @return uint32_t 内存的起始地址
 */
uint32_t memory_alloc_page_align(int page_count, int align) {
  uint32_t addr = addr_alloc_page_align(&paddr_alloc, page_count, align);
  return addr;
}

/**
 * @brief 返回当前进程的页目录表的地址
 *
 * @return pde_t*
 */
static pde_t *curr_page_dir() {
  return (pde_t *)(task_current()->task_sw.page_dir);
}

/**
 * @brief 释放一页内存空间
 *
 * @param addr
 */
void memory_free_page(uint32_t addr, int page_count) {
  for (int i = 0; i < page_count; ++i) {
    if (addr < MEM_TASK_BASE) {  // 释放内核空间的一页内存
      addr_free_page(
          &paddr_alloc, addr,
          1);  // 因为内核空间为一一映射关系，虚拟地址即为物理地址,且不需要解除映射关系
    } else {   // 释放用户空间的一页内存
      // 1.用虚拟地址找到该页对应的页表项
      pte_t *pte = find_pte(curr_page_dir(), addr, 0);
      if (pte == (pte_t *)0 || pte->domain.flag == 0) {  // 未找到对应的页表项
        log_error("free page failed. no pte i =%d\n", i);
      }
      ASSERT(pte != (pte_t *)0 && pte->domain.flag);

      // 2.用该页的物理地址释放该页
      addr_free_page(&paddr_alloc, pte_to_pg_addr(pte), 1);

      // 3.将页表项清空，解除映射关系
      pte->v = 0;
    }

    addr += MEM_PAGE_SIZE;
  }
}

/**
 * @brief 为进程在物理地址空间中分配对应的页空间，并进行映射，
 *        使进程的虚拟地址与物理地址对应起来
 *
 * @param page_dir 进程的页目录表
 * @param vaddr 进程各个代码段的起始虚拟地址
 * @param alloc_size 为进程分配的页空间大小
 * @param priority 进程页空间的权限
 * @return int
 */
int memory_alloc_for_page_dir(uint32_t page_dir, uint32_t vaddr,
                              uint32_t alloc_size, uint32_t privilege) {
  // 1.记录当前分配的虚拟页的起始地址
  uint32_t curr_vaddr = vaddr;
  // 2.计算需要分配多少页
  int page_count = up2(alloc_size, MEM_PAGE_SIZE) / MEM_PAGE_SIZE;

  // 3.逐页进行映射
  for (int i = 0; i < page_count; ++i) {
    uint32_t paddr = addr_alloc_page(&paddr_alloc, 1);
    if (paddr == 0) {  // 分配失败
      log_error("mem alloc failed. no memory\n");
      // TODO:当分配失败时应该将之前分配的页全部归还，且将映射关系也全部解除
      return -1;
    }

    int err =
        memory_creat_map((pde_t *)page_dir, curr_vaddr, paddr, 1, privilege);
    if (err < 0) {  // 分配失败
      log_error("create memory failed. err = %d\n", err);
      // TODO:当分配失败时应该将之前分配的页全部归还，且将映射关系也全部解除
      return -1;
    }

    curr_vaddr += MEM_PAGE_SIZE;

    if (i == 955) {
      int a = 0;
    }
  }

  return 0;
}

/**
 * @brief 为当前进程的虚拟地址分配页空间创建映射关系
 *
 * @param vaddr 待分配空间的起始地址
 * @param alloc_size 为其分配的空间大小
 * @param privilege 对应页空间的权限
 * @return int 错误码
 */
int memory_alloc_page_for(uint32_t vaddr, uint32_t alloc_size,
                          uint32_t privilege) {
  return memory_alloc_for_page_dir(task_current()->task_sw.page_dir, vaddr,
                                   alloc_size, privilege);
}

/**
 * @brief 创建用户进程的虚拟内存空间，即页目录表
 *
 * @return uint32_t
 */
uint32_t memory_creat_uvm() {
  // 1.分配一页作为页目录表
  pde_t *page_dir = (pde_t *)addr_alloc_page_align(
      &paddr_alloc, sizeof(pde_t) * PDE_CNT / MEM_PAGE_SIZE,
      FIRST_LEVEL_PAGE_TABLE_ALIGN);
  if (page_dir == 0) return 0;

  // 2.将该页的内容清空
  kernel_memset((void *)page_dir, 0, sizeof(pde_t) * PDE_CNT);

  // 3.获取用户进程空间的第一个页目录项索引, 用户进程空间的起始地址MEM_TASK_BASE
  // = 0x800 00000
  uint32_t user_pde_start = pde_index(MEM_TASK_BASE);

  // 4.将用户进程空间以下的空间映射给操作系统使用，即将0~user_pde_start的pde提供给操作系统作为页目录项
  for (int i = 0; i < user_pde_start; ++i) {
    page_dir[i].v = kernel_page_dir[i].v;  // 所有进程都共享操作系统的页表
  }

  return (uint32_t)page_dir;
}

/**
 * @brief 销毁该页目录表对应的所有虚拟空间资源，包括映射关系与内存空间
 *
 * @param page_dir 页目录表的地址
 * @param is_read_share 是否开启了读共享策略，1开启，0未开启
 */
void memory_destroy_uvm(uint32_t page_dir) {
  // 1.获取用户进程虚拟地址的起始地址对应的该页目录项
  uint32_t user_task_start = pde_index(MEM_TASK_BASE);
  pde_t *pde = (pde_t *)page_dir + user_task_start;

  // 2.遍历每一个页目录项，清理对应资源
  for (int i = user_task_start; i < PDE_CNT; ++i, ++pde) {
    if (!pde->domain.flag) continue;

    // 3.获取页目录项对应的页表的起始地址
    pte_t *pte = (pte_t *)pde_to_pt_addr(pde);

    // 4.遍历所有页表项，清理对应资源
    for (int j = 0; j < PTE_CNT; ++j, ++pte) {
      if (!pte->domain.flag) continue;

      // 5.释放该物理页
      addr_free_page(&paddr_alloc, pte_to_pg_addr(pte), 1);
    }

    // 6.释放存储该页表的物理页
    addr_free_page(&paddr_alloc, pde_to_pt_addr(pde),
                   PTE_CNT * sizeof(pte_t) / MEM_PAGE_SIZE);
  }

  // 7.释放存储该页目录表的物理页
  addr_free_page(&paddr_alloc, page_dir,
                 PDE_CNT * sizeof(pde_t) / MEM_PAGE_SIZE);
}

/**
 * @brief 拷贝页目录表的映射关系
 *
 * @param to_page_dir 拷贝到的目标页目录表地址
 * @param from_page_dir 被拷贝的源页目录表地址
 * @return uint32_t
 */
int memory_copy_uvm(uint32_t to_page_dir, uint32_t from_page_dir) {
  // 1.获取用户程序虚拟地址的起始pde索引，即0x8000 0000 的pde索引值
  uint32_t user_pde_start = pde_index(MEM_TASK_BASE);
  pde_t *pde = (pde_t *)from_page_dir + user_pde_start;

  // 2.遍历源页目录表中的每一个页目录项，拷贝给目标目录
  for (int i = user_pde_start; i < PDE_CNT; ++i, ++pde) {
    if (!pde->domain.flag)  // 当前页目录项不存在
      continue;

    // 3.获取页目录项指向的页表的起始地址
    pte_t *pte = (pte_t *)pde_to_pt_addr(pde);

    // 4.遍历页表的页表项，进行读共享写复制的映射操作
    for (int j = 0; j < PTE_CNT; ++j, ++pte) {
      if (!pte->domain.flag)  // 当前页表项不存在
        continue;

      // 5.获取该页表项对应的虚拟地址
      uint32_t vaddr = (i << 20) | (j << 10);

      // 6.判断当前页表项指向的页是否支持写操作
      if (pte->v & PTE_AP_USR) {  // 7当前页支持写操作，需进行复制操作
        // 7.1分配一个新的页，进行拷贝
        uint32_t page = addr_alloc_page(&paddr_alloc, 1);
        if (page == 0)  // 分配失败
          goto copy_uvm_failed;

        // 7.2在目标进程空间中记录映射关系
        int err = memory_creat_map((pde_t *)to_page_dir, vaddr, page, 1,
                                   get_pte_privilege(pte));
        if (err < 0) goto copy_uvm_failed;

        // 7.3将该页内容拷贝到目标进程空间中
        kernel_memcpy((void *)page, (void *)vaddr, MEM_PAGE_SIZE);

      } else {  // 8.当前页为只读页，直接共享该页即可，即只复制页表项即可
        // 8.1获取该页的物理地址
        uint32_t page = pte_to_pg_addr(pte);
        // 8.2直接在目标进程空间中记录映射关系
        int err = memory_creat_map((pde_t *)to_page_dir, vaddr, page, 1,
                                   get_pte_privilege(pte));
        if (err < 0) goto copy_uvm_failed;
      }
    }
  }

  return 1;

copy_uvm_failed:
  memory_destroy_uvm(to_page_dir);
  return -1;
}

/**
 * @brief 获取虚拟地址在页目录表中关联的物理页的物理地址
 *
 * @param page_dir
 * @param vaddr
 * @return uint32_t
 */
uint32_t memory_get_paddr(uint32_t page_dir, uint32_t vaddr) {
  // 查找vaddr对应的页表项
  pte_t *pte = find_pte((pde_t *)page_dir, vaddr, 0);

  // 若没有找到该页表项或该页表项还不存在，则返回0
  if (!pte || !pte->domain.flag) {
    return 0;
  }

  // 找到并存在该页表项，返回绑定的物理地址
  return pte_to_pg_addr(pte) | (vaddr & (MEM_PAGE_SIZE - 1));
}

/**
 * @brief 将当前任务的虚拟空间中的内容拷贝到目标虚拟空间中
 *
 * @param to_addr 目标虚拟空间的起始地址
 * @param to_page_dir 目标虚拟空间的页目录表
 * @param from_addr 当前虚拟空间中的源地址
 * @param size 拷贝的大小
 */
int memory_copy_uvm_data(uint32_t to_vaddr, uint32_t to_page_dir,
                         uint32_t from_vaddr, uint32_t size) {
  // 由于虚拟地址空间是连续的而物理地址空间不一定，所以需要一页一页的单独拷贝
  while (size > 0) {
    // 1.获取to_vaddr对应的物理地址
    uint32_t to_paddr = memory_get_paddr(to_page_dir, to_vaddr);
    if (to_paddr == 0) {
      return -1;
    }

    // 2.获取to_paddr所在的物理页可以写入的空间大小
    // 即从to_paddr到该页的末尾的空间大小
    uint32_t offset = (MEM_PAGE_SIZE - 1) & to_paddr;
    uint32_t curr_size = MEM_PAGE_SIZE - offset;

    // 3.判断size是否  < curr_size
    if (size < curr_size) {  // size < curr_size则在当前物理页拷贝szie个字节即可
      curr_size = size;
    }

    // 4.拷贝内容并更新到下一个需要拷贝的地方
    kernel_memcpy((void *)to_paddr, (void *)from_vaddr, curr_size);
    size -= curr_size;
    to_vaddr += curr_size;
    from_vaddr += curr_size;
  }

  return 0;
}

/**
 * @brief 拓展堆区内存
 *
 * @param incr 增长指定字节
 * @return char*
 */
char *sys_sbrk(int incr) {
  task_t *task = task_current();
  char *pre_heap_end = (char *)task->heap_end;

  if (incr == 0) {
    return pre_heap_end;
  }

  if (incr > 0) {
    uint32_t start = task->heap_end;  // 堆区原始末尾位置
    uint32_t after_heap_end = start + incr;      // 需要拓展到的末尾位置
    uint32_t start_offset =
        start % MEM_PAGE_SIZE;  // 获取末尾位置在当前页内的偏移量
    if (start_offset) {         // 先将当前页的剩余空间分配出去
      if (start_offset + incr <= MEM_PAGE_SIZE) {  // 当前页剩余内存可供分配
        task->heap_end = after_heap_end;
        incr = 0;
      } else {  // 当前页剩余内存不够分配
        uint32_t curr_size =
            MEM_PAGE_SIZE - start_offset;  // 获取当前页剩余大小
        // 将当前页剩余内存全部分配出
        start += curr_size;
        incr -= curr_size;
      }
    }

    if (incr) {                          // 还需要继续拓展
      uint32_t curr_size = after_heap_end - start;  // 还需拓展的大小
      int err = memory_alloc_page_for(
          start, curr_size, PTE_FLAG | PTE_AP_USR);  // 为该部分内存创建映射关系
      if (err < 0) {
        log_error("sbrk: alloc mem failed.\n");
        return (char *)-1;
      }
    }

    task->heap_end = after_heap_end;
    return (char *)pre_heap_end;
  }

  //释放分配的内存
  if (incr < 0) {
    incr = 0 - incr;
    uint32_t offset = task->heap_end % MEM_PAGE_SIZE;
    uint32_t after_heap_end = task->heap_end - incr;
    
    if (offset) {
      if (offset - incr > 0) {  // 不需要释放页
        task->heap_end = after_heap_end;
        return after_heap_end;
      } else {
        incr -= offset;
      }
    }

    if (incr >= 0) {
      uint32_t align_heap_end = up2(after_heap_end, MEM_PAGE_SIZE);  // 向上对齐

      memory_free_page(
          align_heap_end,
          up2(task->heap_end - align_heap_end, MEM_PAGE_SIZE) / MEM_PAGE_SIZE);

      task->heap_end = after_heap_end;
    }
    return (char *)after_heap_end;
  }
}

/**
 * @brief 查看已使用内存量
 *
 * @return int
 */
static int memory_used() {
  int cnt = 0;
  for (int i = 0; i < paddr_alloc.bitmap.bit_count; ++i) {
    if (bitmap_is_set(&paddr_alloc.bitmap, i)) {
      cnt++;
    }
  }

  return cnt * paddr_alloc.page_size;
}

/**
 * @brief 获取内存使用情况
 *
 * @return int
 */
int sys_memory_stat(char *buf, int size) {
  if (size <= (sizeof(int) * 6 * 8 + 60)) {
    return -1;
  }
  kernel_memset(buf, 0, size);
  int mem_used = memory_used();
  int mem_free = paddr_alloc.size - mem_used;
  kernel_sprintf(buf,
                 "mem_start:\t0x%x.\nmem_size:\t%dM.\nmem_used:\t%dMB-%dKB."
                 "\nmem_free:\t%dMB-%dKB.\n",
                 paddr_alloc.start, paddr_alloc.size / (1024 * 1024),
                 (mem_used / (1024 * 1024)), (mem_used % (1024 * 1024)) / 1024,
                 (mem_free / (1024 * 1024)), (mem_free % (1024 * 1024)) / 1024);

  return 0;
}

/**
 * @brief 查看页目录表中已映射的页数
 *
 * @param page_dir
 * @return int
 */
int memory_page_count_used(uint32_t page_dir) {
  int cnt = 0;
  // 1.获取用户程序虚拟地址的起始pde索引，即0x8000 0000 的pde索引值
  uint32_t user_pde_start = pde_index(MEM_TASK_BASE);
  pde_t *pde = (pde_t *)page_dir + user_pde_start;

  for (int i = user_pde_start; i < PDE_CNT; ++i, ++pde) {
    if (!pde->domain.flag) {  // 当前页目录项不存在
      continue;
    }

    // 3.获取页目录项指向的页表的起始地址
    pte_t *pte = (pte_t *)pde_to_pt_addr(pde);

    for (int j = 0; j < PTE_CNT; ++j, ++pte) {
      if (!pte->domain.flag) {  // 当前页表项不存在
        continue;
      }

      cnt++;
    }
  }

  return cnt;
}