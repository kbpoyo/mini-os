#include "core/task.h"

#include "common/cpu_instr.h"
#include "common/os_config.h"
#include "core/irq.h"
#include "core/memory.h"
#include "ipc/mutex.h"
#include "tools/klib.h"
#include "tools/log.h"

// 定义全局唯一的任务管理器对象
static task_manager_t task_manager;
// 定义静态的任务表，用于任务的静态分配
static task_t task_table[TASK_COUNT];
// 定义用于维护task_table的互斥锁
static mutex_t task_table_lock;

/**
 * @brief 分配task任务结构
 *
 * @return task_t*
 */
task_t *task_alloc(void) {
  // 加锁
  mutex_lock(&task_table_lock);
  task_t *task = (task_t *)0;
  for (int i = 0; i < TASK_COUNT; ++i) {
    task = task_table + i;
    if (task->pid == 0) {
      break;
    }
  }

  if (task == (task_t *)0 || task >= task_table + TASK_COUNT) {
    // 解锁
    mutex_unlock(&task_table_lock);
    return 0;
  }

  task->pid = 0xffffffff;  // 标记已分配

  // 解锁
  mutex_unlock(&task_table_lock);

  return task;
}

/**
 * @brief 初始化寄存器组
 *
 * @param task
 * @param entry
 * @param sp
 * @param flag
 * @return int
 */
static int registers_init(task_t *task, uint32_t entry, uint32_t sp,
                          uint32_t flag) {
  // 在sp对应的栈空间中将寄存器组压入栈中
  if (entry <= 0 || sp <= 0) return 0;  // 调试用

  // 初始化任务的寄存器组
  register_group_t reg_group;
  kernel_memset(&reg_group, 0, sizeof(register_group_t));
  reg_group.spsr = flag == TASK_FLAGS_USER ? TASK_CPSR_USER : TASK_CPSR_SYS;
  reg_group.cpsr = flag == TASK_FLAGS_USER ? TASK_CPSR_USER : TASK_CPSR_SYS;
  reg_group.r13 = sp;
  reg_group.r15 = entry;

  // 为任务分配一页内核栈，并将寄存器组拷贝到内核栈中，等待任务的初始化
  uint32_t sp_addr = memory_alloc_page();
  sp_addr = sp_addr + MEM_PAGE_SIZE - sizeof(register_group_t);
  kernel_memcpy((void *)sp_addr, &reg_group, sizeof(register_group_t));

  // // sp_ptr[0] = spsr, [1] = cpsr, .... [17] = r15
  // sp_ptr[0] =
  //     flag == TASK_FLAGS_USER
  //         ? TASK_CPSR_USER
  //         : TASK_CPSR_SYS;  //
  //         初始化用户模式时的spsr，新任务执行使，spsr会传入给cpsr
  // sp_ptr[1] = flag == TASK_FLAGS_USER
  //                 ? TASK_CPSR_USER
  //                 : TASK_CPSR_SYS;  // 初始化用户模式时的cpsr
  // sp_ptr[15] = (uint32_t)sp;        // 初始化sp指针
  // sp_ptr[17] = entry;               // 初始化pc指针

  task->task_sp.svc_sp = sp_addr;
  // task->base_svc_sp = task->svc_sp;

  return 0;
}

/**
 * @brief  初始化任务
 *
 * @param task 任务对象
 * @param entry 任务的入口地址
 * @param sp 任务指行时所用的栈顶指针
 * @param flag 任务属性标志位，如特权级
 * @return int
 */
int task_init(task_t *task, const char *name, uint32_t entry, uint32_t sp,
              uint32_t flag) {
  ASSERT(task != (task_t *)0);
  // 1.初始化任务的初始寄存器状态
  int err = registers_init(task, entry, sp, flag);
  if (err == -1) return err;

  // 2.初始化任务名称
  kernel_strncpy(task->name, name, TASK_NAME_SIZE);

  // 3.初始化任务队列节点及就绪队列节点
  list_node_init(&task->ready_node);
  list_node_init(&task->task_node);
  list_node_init(&task->wait_node);

  // 4.初始化最大时间片数与当前拥有时间片数,以及延时时间片数
  task->state = TASK_CREATED;
  task->slice_max = task->slice_curr = TASK_TIME_SLICE_DEFAULT;
  task->sleep = 0;
  task->pid = (uint32_t)task;
  // task->parent = (task_t *)0;
  // task->heap_start = task->heap_end = 0;
  // task->status = 0;

  // 5.初始化文件表
  // kernel_memset(&task->file_table, 0, sizeof(task->file_table));

  // 6.将任务加入任务队列
  list_insert_last(&task_manager.task_list, &task->task_node);

  return 1;
}

// 空闲进程的栈空间
static uint32_t empty_task_stack[EMPTY_TASK_STACK_SIZE];
/**
 * @brief  空闲进程，当所有进程都延时运行时，让cpu运行空闲进程
 *
 */
static void empty_task(void) {
  while (1) {
    // 停止cpu运行，让cpu等待时间中断

    // log_printf("empty task working!\n");
    // for (uint32_t i = 0; i < 1000/*ms*/; i++) {
    //   for (uint32_t j = 0; j < 400; j++);
    // }
  };
}

/**
 * @brief  初始化任务管理器
 *
 */
void task_manager_init(void) {
  log_printf("task manager init start...\n");
  // 1.初始化所有任务队列
  list_init(&task_manager.ready_list);
  list_init(&task_manager.task_list);
  list_init(&task_manager.sleep_list);

  // 3.将当前任务置零
  task_manager.curr_task = (task_t *)0;

  // 4.初始化空闲进程
  task_init(&task_manager.empty_task, "empty_task", (uint32_t)empty_task,
            (uint32_t)&empty_task_stack[EMPTY_TASK_STACK_SIZE],
            TASK_FLAGS_SYSTEM);

  // 5.初始化静态任务表,及其互斥锁
  kernel_memset(task_table, 0, sizeof(task_table));
  mutex_init(&task_table_lock);

  log_printf("task manager init success...\n");
}

/**
 * @brief 将任务插入任务链表中并设为就绪态，标志该任务可被调度
 *
 * @param task
 */
void task_start(task_t *task) {
  // 将任务设置为就绪态
  task_set_ready(task);
}

/**
 * @brief  初始化第一个任务
 *
 */
void task_first_init(void) {
  // 1.初始化任务，当前任务是在任务管理器启用前就执行的，
  // 拥有自己的栈空间，所以入口地址直接和栈空间都置0即可
  // 这一步只是为当前任务绑定一段空间用于储存寄存器组，并将其绑定到一个task对象
  task_init(&task_manager.first_task, "first task", 0, 0, 0);

  // 3.将当前任务执行第一个任务
  task_manager.curr_task = &task_manager.first_task;

  // 4.将当前任务状态设置为运行态
  task_start(task_manager.curr_task);

  task_manager.curr_task->state = TASK_RUNNING;
}

/**
 * @brief 进入临界保护区，使用关中断的方式进行保护
 *
 * @return uint32_t
 */
uint32_t task_enter_protection(void) {
  uint32_t state = cpu_get_cpser();
  cpu_irq_close();
  return state;
}

/**
 * @brief 离开临界保护区
 *
 * @param state
 */
void task_leave_protection(uint32_t state) { cpu_set_cpsr(state); }

/**
 * @brief  获取当前任务管理器的第一个任务
 *
 * @return task_t*
 */
task_t *task_first_task(void) { return &task_manager.first_task; }

/**
 * @brief  将任务task加入就绪队列
 *
 * @param task 需要加入就绪队列的任务
 */
void task_set_ready(task_t *task) {
  ASSERT(task != (task_t *)0);
  // if (task == (task_t*)0) return;
  cpu_state_t state = task_enter_protection();

  // 1.将任务插入到就绪队列的尾部
  list_insert_last(&task_manager.ready_list, &task->ready_node);
  task->state = TASK_READY;

  task_leave_protection(state);

  // 2.将任务状态设置为就绪态
  // task->state = TASK_READY;
}

/**
 * @brief  将任务task从就绪队列中取下
 *
 * @param task
 */
void task_set_unready(task_t *task) {
  ASSERT(task != (task_t *)0);
  // if (task == (task_t*)0) return;
  cpu_state_t state = task_enter_protection();

  list_remove(&task_manager.ready_list, &task->ready_node);

  task_leave_protection(state);
}

/**
 * @brief  获取就绪队列中的第一个任务
 *
 */
task_t *task_ready_first(void) {
  list_node_t *ready_node = list_get_first(&task_manager.ready_list);

  return list_node_parent(ready_node, task_t, ready_node);
}

/**
 * @brief  获取当前正在运行的任务
 *
 * @return task_t*
 */
task_t *task_current(void) { return task_manager.curr_task; }

extern void task_switch_by_sp(task_sp_t *sp_from, task_sp_t *sp_to);

/**
 * @brief  将任务从from切换到to
 *
 * @param from 切换前的任务
 * @param to 切换后的任务
 */
static void task_switch_from_to(task_t *from, task_t *to) {
  // 跳转到对应的tss段读取并恢复cpu任务状态
  task_switch_by_sp(&(from->task_sp), &(to->task_sp));
}

/**
 * @brief  任务管理器进行任务切换
 *
 */
void task_switch(void) {
  cpu_state_t state = task_enter_protection();  // TODO:加锁

  // 1.获取就绪队列中的第一个任务
  task_t *to = task_ready_first();

  // 2.若获取到的任务不是当前任务就进行切换
  if (to != task_manager.curr_task) {
    // 3.获取当前任务
    task_t *from = task_manager.curr_task;

    // 4.目标任务若为空，则所有任务都在延时，让cpu运行空闲任务
    if (to == (task_t *)0) {
      to = &task_manager.empty_task;
    }
    // 5.切换当前任务, 并将当前任务置为运行态
    to->state = TASK_RUNNING;
    if (from->state == TASK_RUNNING) {
      from->state = TASK_READY;
    }
    task_manager.curr_task = to;

    // 6.进行任务切换
    task_switch_from_to(from, to);
  }

  task_leave_protection(state);  // TODO:解锁
}
/**
 * @brief  设置进程延时的时间片数
 *
 * @param task 需要延时的进程
 * @param slice 延时的时间片数
 */
void task_set_sleep(task_t *task, uint32_t slice) {
  ASSERT(task != (task_t *)0);
  if (slice == 0) return;

  cpu_state_t state = task_enter_protection();

  task->sleep = slice;
  task->state = TASK_SLEEP;
  list_insert_last(&task_manager.sleep_list, &task->ready_node);

  task_leave_protection(state);
}

/**
 * @brief  唤醒正在延时的进程
 *
 * @param task
 */
void task_set_wakeup(task_t *task) {
  ASSERT(task != (task_t *)0);

  cpu_state_t state = task_enter_protection();

  list_remove(&task_manager.sleep_list, &task->ready_node);
  task->state = TASK_CREATED;

  task_leave_protection(state);
}

/**
 * @brief  提供给时钟中断使用，每中断一次，当前任务的时间片使用完一次
 *         减少当前任务的时间片数，并判断是否还有剩余时间片，若没有就进行任务切换
 *
 */
void task_slice_end(void) {
  // 1.遍历当前延时队列，判断是否有可唤醒的任务
  list_node_t *curr_sleep_node = list_get_first(&task_manager.sleep_list);

  // 2.遍历并判断每一个任务执行完当前时间片是否可被唤醒，若可以则唤醒
  while (curr_sleep_node) {
    list_node_t *next_sleep_node = list_node_next(curr_sleep_node);

    task_t *curr_sleep_task =
        list_node_parent(curr_sleep_node, task_t, ready_node);
    if (--curr_sleep_task->sleep == 0) {
      task_set_wakeup(curr_sleep_task);  // 从延时队列中取下
      task_set_ready(curr_sleep_task);   // 加入就绪队列
    }

    curr_sleep_node = next_sleep_node;
  }

  // task_switch(); 没有必要立马进行任务切换，当前任务时间片用完后会自动切换
  // 3.获取当前任务
  task_t *curr_task = task_current();

  // 4.若当前任务为空闲任务，则判断就绪队列是否为空
  if (curr_task == &task_manager.empty_task) {
    if (list_is_empty(&task_manager.ready_list)) return;

    task_manager.empty_task.state = TASK_CREATED;

    task_switch();  // 就绪队列有任务，则直接切换任务
  }

  // 5.若当前任务为普通任务则，减小当前时间片数
  if (curr_task != &task_manager.empty_task && --curr_task->slice_curr == 0) {
    // 6.时间片数用完了，重置时间片并进行任务切换
    curr_task->slice_curr = curr_task->slice_max;
    task_set_unready(curr_task);
    task_set_ready(curr_task);
    task_switch();
  }
}

/**
 * @brief  使进程进入延时状态
 *
 * @param ms 延时的时间，以ms为单位
 */
void sys_sleep(uint32_t ms) {
  cpu_state_t state = task_enter_protection();  // TODO:加锁

  // 1.获取当前任务
  task_t *curr_task = task_current();

  // 2.将当前任务离开就绪队列
  task_set_unready(curr_task);

  // 3.计算出需要延时的时间片数，对时间片数向上取整，保证进程至少能延时指定时间
  uint32_t slice =
      (ms + (TASK_TIME_SLICE_DEFAULT - 1)) / TASK_TIME_SLICE_DEFAULT;

  // 4.将当前任务放入延时队列，并设置延时时间片数
  task_set_sleep(curr_task, slice);

  // 5.切换任务
  task_switch();

  task_leave_protection(state);  // TODO:解锁
}

/**
 * @brief 获取任务pid
 *
 * @return int pid
 */
int sys_getpid(void) { return task_current()->pid; }

/**
 * @brief  使当前正在运行的任务主动让出cpu
 */
void sys_yield(void) {
  cpu_state_t state = task_enter_protection();  // TODO:加锁

  // 1.判断当前就绪队列中是否有多个任务
  if (list_get_size(&task_manager.ready_list) > 1) {
    // 2.获取当前任务
    task_t *curr_task = task_current();

    // 3.将当前任务从就绪队列中取下
    task_set_unready(curr_task);

    // 4.将当前任务重新加入到就绪队列的队尾
    task_set_ready(curr_task);

    // 5.任务管理器运行下一个任务，从而释放cpu使用权
    task_switch();
  }

  task_leave_protection(state);  // TODO:解锁
}
