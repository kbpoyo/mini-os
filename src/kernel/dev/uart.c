#include "dev/uart.h"

#include <stdarg.h>

#include "common/types.h"
#include "core/irq.h"
#include "tools/assert.h"
#include "tools/klib.h"

static uart_t uart_table[UART_COUNT]
    __attribute__((section(".data"), aligned(4)));

static int curr_uart_index __attribute__((section(".data"))) = 0;

void irq_handler_for_uartRX0();

/**串口初始化*/
void uart_init(int uart_inedx) {
  rUFCON0 = 0x0;  // UART channel 0 FIFO control register, FIFO disable
  rUFCON1 = 0x0;  // UART channel 1 FIFO control register, FIFO disable
  rUFCON2 = 0x0;  // UART channel 2 FIFO control register, FIFO disable
  rUMCON0 = 0x0;  // UART chaneel 0 MODEM control register, AFC disable
  rUMCON1 = 0x0;  // UART chaneel 1 MODEM control register, AFC disable

  switch (uart_inedx) {
    case 0: {
      // UART0
      rULCON0 =
          0x3;  // Line control register : Normal,No parity,1 stop,8 bits
                //     [10]       [9]     [8]        [7]        [6]      [5]
                //     [4]           [3:2]        [1:0]
                //  Clock Sel,  Tx Int,  Rx Int, Rx Time Out, Rx err,
                //  Loop-back, Send break,  Transmit Mode, Receive Mode
                //      0          1       0    ,     0          1        0 0
                //      ,       01          01
                //    PCLK       Level    Pulse    Disable    Generate  Normal
                //    Normal        Interrupt or Polling
      rUCON0 = 0x245;  // Control register
      rUBRDIV0 = ((int)(OS_PCLK / (16 * BAUDRATE)) -
                  1);  // Baud rate divisior register 0

      // 使能串口接收中断
      // TODO:这里只使能0串口
      irq_handler_register(INT_RXD0_PRIM,
                           (irq_handler_t)irq_handler_for_uartRX0);
      irq_enable(INT_RXD0_PRIM, INT_RXD0_SUB);

      // 初始化串口设备表
      uart_table[0].in_addr = (volatile unsigned *)URXH0;
      uart_table[0].out_addr = (volatile unsigned *)UTXH0;
      uart_table[0].state_addr = (volatile unsigned *)UTRSTAT0;
      mutex_init(&uart_table[0].mutex);
    } break;

    case 1: {
      // UART1
      rULCON1 = 0x3;
      rUCON1 = 0x245;
      rUBRDIV1 = ((int)(OS_PCLK / (16 * BAUDRATE)) - 1);
      uart_table[1].in_addr = (volatile unsigned *)URXH1;
      uart_table[1].out_addr = (volatile unsigned *)UTXH1;
      uart_table[1].state_addr = (volatile unsigned *)UTRSTAT1;
      mutex_init(&uart_table[1].mutex);
    } break;

    case 2: {
      // UART2
      rULCON2 = 0x3;
      rUCON2 = 0x245;
      rUBRDIV2 = ((int)(OS_PCLK / (16 * BAUDRATE)) - 1);

      uart_table[2].in_addr = (volatile unsigned *)URXH2;
      uart_table[2].out_addr = (volatile unsigned *)UTXH2;
      uart_table[2].state_addr = (volatile unsigned *)UTRSTAT2;
      mutex_init(&uart_table[2].mutex);
    } break;

    default:
      break;
  }
}

/**
 * @brief 选择输出串口
 *
 * @param uart_number
 */
void uart_select(int uart_index) {
  if (uart_index >= 0 && uart_index < UART_COUNT) {
    curr_uart_index = uart_index;
  }
}

/**
 * @brief 获取当前输出串口
 *
 * @return int
 */
static int uart_get() { return curr_uart_index; }

// /**
//  * @brief 串口发送字节
//  *
//  * @param data
//  */
// void uart_send_byte(uint8_t data) {
//   if (curr_uart_index == 0) {
//     if (data == '\n') {
//       while (!(rUTRSTAT0 & STATE_TRA_BUFF_ISEMPTY))
//         ;
//       WrUTXH0('\r');
//     }

//     while (!(rUTRSTAT0 & STATE_TRA_BUFF_ISEMPTY))
//       ;
//     WrUTXH0(data);

//   } else if (curr_uart_index == 1) {
//     if (data == '\n') {
//       while (!(rUTRSTAT1 & STATE_TRA_BUFF_ISEMPTY))
//         ;
//       WrUTXH1('\r');
//     }

//     while (!(rUTRSTAT1 & STATE_TRA_BUFF_ISEMPTY))
//       ;
//     WrUTXH1(data);

//   } else if (curr_uart_index == 2) {
//     if (data == '\n') {
//       while (!(rUTRSTAT2 & STATE_TRA_BUFF_ISEMPTY))
//         ;
//       WrUTXH2('\r');
//     }

//     while (!(rUTRSTAT2 & STATE_TRA_BUFF_ISEMPTY))
//       ;
//     WrUTXH2(data);
//   }
// }

/**
 * @brief 串口发送字节
 *
 * @param data
 */
void uart_send_byte(uart_t *uart, uint8_t data) {
  while (!((*(uart->state_addr)) & STATE_TRA_BUFF_ISEMPTY))
    ;
  *(uart->out_addr) = data;
}

/**
 * @brief 串口发送字符串
 *
 * @param str
 */
void uart_send_str(uart_t *uart, const char *str) {
  const char *temp = str;
  while (*temp) {
    uart_send_byte(uart, *(temp++));
  }
}

/**
 * @brief 串口格式化输出
 *
 * @param fmt
 * @param ...
 */
void uart_printf(uart_t *uart, char *fmt, ...) {
  char string[256];

  kernel_memset(string, 0, sizeof(string));

  va_list ap;
  va_start(ap, fmt);

  kernel_vsprintf(string, fmt, ap);
  va_end(ap);

  uart_send_str(uart, string);
}

/**
 * @brief 写入uart设备
 *
 */
int uart_write(tty_t *tty) {
  // 获取需要需要写入的终端
  uart_t *uart = uart_table + tty->console_index;
  int len = 0;

  // TODO:加锁
  mutex_lock(&uart->mutex);

  // 在tty的缓冲队列中读取一个字符写入终端
  do {
    char c;
    int err = tty_fifo_get(&tty->out_fifo, &c);
    if (err < 0) {
      break;
    }

    // 成功消耗掉缓冲区一个字节资源后，对资源进行释放
    sem_notify(&tty->out_sem);

    // 将该字节传输给串口
    uart_send_byte(uart, c);

    len++;
  } while (1);

  // TODO:解锁
  mutex_unlock(&uart->mutex);

  return len;
}

/**
 * @brief 向uart设备发送控制指令
 *
 */
int uart_control(int cmd, int arg0, int arg1) {}

void irq_handler_for_uartRX0() {
  ASSERT((rINTOFFSET == INT_UART0));
  // 清除中断位
  irq_clear(INT_RXD0_PRIM, INT_RXD0_SUB);

  uart_t *uart = uart_table + curr_uart_index;

  // 判断状态寄存器接收缓冲区位，是否有数据可读
  if (*(uart->state_addr) & STATE_REC_BUFF_ISREADY) {
    tty_in(*(uart->in_addr));
  }
}

/**
 * @brief 关闭uart设备
 *
 */
void uart_close() {}