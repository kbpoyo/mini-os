/**
 * @file tty.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief tty设备
 * @version 0.1
 * @date 2023-07-05
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "core/tty.h"

#include "core/dev.h"
#include "core/irq.h"
#include "core/task.h"
#include "dev/uart.h"
#include "tools/log.h"

static tty_t tty_table[TTY_TABLE_SIZE];  // 全局tty设备表
static int curr_tty_index __attribute__((section(".data"), aligned(4))) =
    0;  // 系统当前使用tty设备索引

/**
 * @brief 根据dev结构获取到对应的tty设备结构
 *
 * @param dev
 * @return tty_t*
 */
static tty_t *get_tty(device_t *dev) {
  int tty_index = dev->dev_index;
  if (tty_index < 0 || tty_index >= TTY_TABLE_SIZE || !dev->open_count) {
    log_printf("tty is not opened. tty = %d\n", tty_index);
    return (tty_t *)0;
  }

  return tty_table + tty_index;
}

/**
 * @brief 初始化tty设备的缓冲队列
 *
 * @param fifo
 * @param buf
 * @param size
 */
static void tty_fifo_init(tty_fifo_t *fifo, char *buf, int size) {
  fifo->buf = buf;
  fifo->count = 0;
  fifo->size = size;
  fifo->read = fifo->write = 0;
}

/**
 * @brief 往缓冲队列fifo中写入字符c
 *
 * @param fifo
 * @param c
 * @return int
 */
int tty_fifo_put(tty_fifo_t *fifo, char c) {
  // TODO:加锁
  cpu_state_t state = task_enter_protection();

  // fifo已满，不能再写入
  if (fifo->count >= fifo->size) {
    // TODO:解锁
    task_leave_protection(state);
    return -1;
  }

  fifo->buf[fifo->write++] = c;     // 写入一个字符
  if (fifo->write >= fifo->size) {  // 循环队列的方式写入
    fifo->write = 0;
  }

  fifo->count++;

  // 开中断取消资源保护
  // TODO:解锁
  task_leave_protection(state);
  return 0;
}

/**
 * @brief 从缓冲队列fifo中读取一个字符放到c中
 *
 * @param fifo
 * @param c
 * @return int
 */
int tty_fifo_get(tty_fifo_t *fifo, char *c) {
  // TODO:加锁关中断进行资源保护
  cpu_state_t state = task_enter_protection();

  if (fifo->count <= 0) {
    // TODO:解锁
    task_leave_protection(state);
    return -1;
  }

  *c = fifo->buf[fifo->read++];    // 读取一个字符
  if (fifo->read >= fifo->size) {  // 循环队列的方式读取
    fifo->read = 0;
  }

  fifo->count--;

  // TODO:解锁
  task_leave_protection(state);
  return 0;
}

/**
 * @brief 打开tty设备
 *
 */
int tty_open(device_t *dev) {
  int index = dev->dev_index;
  if (index < 0 || index >= TTY_TABLE_SIZE) {
    log_printf("open tty failed. incorrect tty num = %d\n", index);
    return -1;
  }

  tty_t *tty = tty_table + index;
  // 初始化输入输出缓冲队列
  tty_fifo_init(&tty->out_fifo, tty->out_buf, TTY_OBUF_SIZE);
  tty_fifo_init(&tty->in_fifo, tty->in_buf, TTY_IBUF_SIZE);

  // 初始化缓冲区的信号量, 缓冲区的每一个字节都视为资源
  sem_init(&tty->out_sem,
           TTY_OBUF_SIZE);  // 输出缓冲区一开始有TTY_OBUF_SIZE大小的资源可写
  sem_init(&tty->in_sem, 0);  // 输入缓冲区一开始无资源可读

  // 为tty设备绑定输出终端
  tty->console_index = index;
  tty->oflags = TTY_OCRLF;  // 默认开启输出模式下'\n'转换为'\r\n'的模式
  tty->iflags =
      TTY_INCLR | TTY_IECHO;  // 默认开启输入模式下的换行转换和字符回显

  // 初始化tty设备需要的键盘与终端
  uart_init(index);
  return 0;
}

/**
 * @brief 写入tty设备
 *
 */
int tty_write(device_t *dev, int addr, char *buf, int size) {
  if (size < 0) {
    return -1;
  }

  // 根据dev结构获取到对应的tty设备结构
  tty_t *tty = get_tty(dev);
  if (!tty) {
    return -1;
  }

  int len = 0;
  while (size) {
    // 获取待写入字符
    char c = *(buf++);

    // 当前输出为"\r\n"换行模式，
    if (c == '\n' && (tty->oflags & TTY_OCRLF)) {
      sem_wait(&tty->out_sem);
      int err = tty_fifo_put(&tty->out_fifo, '\r');
      if (err < 0) {
        break;
      }
    }

    if (c == 0x7f) {  // 进行退格处理
      char *str = "\x1b[1D \x1b[1";
      for (int i = 0; i < 8; ++i) {
        sem_wait(&tty->out_sem);
        int err = tty_fifo_put(&tty->out_fifo, str[i]);
        if (err < 0) {
          break;
        }
      }
      c = 'D';
    }

    // 先获取到访问缓冲区一个字节资源的资格
    // 若缓冲区写满就阻塞住，等待中断程序将缓冲区消耗掉再写
    sem_wait(&tty->out_sem);

    int err = tty_fifo_put(&tty->out_fifo, c);
    if (err < 0) {
      break;
    }

    len++;
    size--;

    // 显示器直接写显存，不需要写io端口
    // 所以不需要交给中断处理程序，即当前进程自己往缓冲区写入并读取
    // 此处是为了模仿当前进程对缓冲区写入的同时中断处理程序读取缓冲区
    // 此处肯定不会阻塞在信号量中，但利用中断处理程序就会阻塞
    uart_write(tty);
  }

  return len;
}

/**
 * @brief 读取读取设备
 *
 */
int tty_read(device_t *dev, int addr, char *buf, int size) {
  if (size < 0) {
    return -1;
  }

  // 1.获取操作的tty设备
  tty_t *tty = get_tty(dev);

  char *pbuf = buf;
  int len = 0;

  // 2.从输入缓冲队列中读取字符到缓冲区buf中
  while (len < size) {
    // 2.1等待资源就绪
    sem_wait(&tty->in_sem);

    // 2.2资源已就绪，读取一个字符
    char ch;
    tty_fifo_get(&tty->in_fifo, &ch);
    switch (ch) {
      case 0x7f:  // 退格键不读取并删除buf中上一个读取到的字符
        if (len == 0) {
          continue;
        } else {
          len--;
          *(--pbuf) = '\0';
        }
        break;
      case '\n':
        if ((tty->iflags & TTY_INCLR) && len < size - 1) {
          // 开启了换行转换
          *(pbuf++) = '\r';
          len++;
        }
        *(pbuf++) = '\n';
        len++;
        break;
      case '\r':
        if ((tty->iflags & TTY_INCLR) && len < size - 1) {
          // 开启了换行转换
          *(pbuf++) = '\r';
          len++;
          ch = '\n';
        }
        *(pbuf++) = '\n';
        len++;
        break;
      default:
        *(pbuf++) = ch;
        len++;
        break;
    }

    // 若tty设备开启了回显模式，则将输入回显到设备上
    if (tty->iflags & TTY_IECHO) {
      tty_write(dev, 0, &ch, 1);
    }

    // 若输入回车或者换行则直接停止读取
    if (ch == '\n' || ch == '\r') {
      break;
    }
  }

  return len;
}

/**
 * @brief 向tty设备发送控制指令
 *
 */
int tty_control(device_t *dev, int cmd, int arg0, int arg1) {
  tty_t *tty = get_tty(dev);
  switch (cmd) {
    case TTY_CMD_ECHO:  // 对tty回显进行设置
      if (arg0) {
        tty->iflags |= TTY_IECHO;
      } else {
        tty->iflags &= ~TTY_IECHO;
      }
      break;
    case TTY_CMD_IN_COUNT:  // 获取tty输入缓冲区的字符个数
      if (arg0) {
        *(int *)arg0 = sem_count(&tty->in_sem);
      }
      break;
    default:
      break;
  }
  return 0;
}

/**
 * @brief 关闭tty设备
 *
 */
void tty_close(device_t *dev) {}

/**
 * @brief 将字符放入对应索引的tty设备的输入缓冲队列中
 *
 * @param dev_index
 * @param ch
 */
void tty_in(char ch) {
  // 1.获取tty设备
  tty_t *tty = tty_table + curr_tty_index;

  // 2.判断输入缓冲区资源是否已准备满
  if (sem_count(&tty->in_sem) >= TTY_IBUF_SIZE) {
    // 输入缓冲区已写满，放弃写入
    return;
  }

  // 3.将字符写入输入缓冲队列
  tty_fifo_put(&tty->in_fifo, ch);

  // 4.准备好一份可读资源，唤醒等待的进程或添加可获取资源
  sem_notify(&tty->in_sem);
}

/**
 * @brief 通过索引号更改当前系统使用的tty设备
 *
 * @param tty_index
 */
void tty_select(int tty_index) {
  if (tty_index != curr_tty_index) {
    // 选择对应的终端设备
    uart_select(tty_index);
    curr_tty_index = tty_index;
  }
}

// 操作tty结构的函数表
dev_desc_t dev_tty_desc = {.dev_name = "tty",
                           .open = tty_open,
                           .read = tty_read,
                           .write = tty_write,
                           .control = tty_control,
                           .close = tty_close};