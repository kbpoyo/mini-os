#ifndef SDI_H
#define SDI_H

#include "common/types.h"
#include "core/dev.h"
#include "core/disk.h"

// 定义sd信息结构
typedef struct _sd_info_t {
  uint8_t bus_width;
  // 获取sd卡的RCA地址
  volatile uint32_t rca;
  uint32_t block_cnt;
  uint32_t block_size;

} sd_info_t;

// Function prototypes
int sd_init(void);

int sd_check_data_end(void);
int sd_check_busy_end(void);
int sd_check_cmd_end(int cmd, int be_resp);
int sd_check_ocr(void);


int sd_sel_desel(char sel_desel);

int sd_cmd0(void);
int sd_cmd55(void);

int sd_cmd8(void);

int sd_read_blocks(uint32_t first_block_addr, uint8_t *buf,
                   uint32_t block_cnt);
int sd_write_blocks(uint32_t first_block_addr, uint8_t *buf,
                    uint32_t block_cnt);

void sd_set_bus(void);
void sd_set_4bit_bus(void);

// 定义数据块最多块数
#define SD_BLOCK_CNT_MAX 4095
#define SD_BLOCK_SIZE 512

// 定义SDI命令状态寄存器
#define SD_CMDSTA_CmdOn (1 << 8)     // 命令传输处理中
#define SD_CMDSTA_RspFin (1 << 9)    // 响应结束
#define SD_CMDSTA_CmdTout (1 << 10)  // 响应超时
#define SD_CMDSTA_CmdSent (1 << 11)  // 命令发送结束
#define SD_CMDSTA_RspCrc (1 << 12)   // 响应CRC错误

#define SD_OCR_HCS (1 << 30)       // 主机支持HC卡（大容量卡）
#define SD_OCR_Power_UP (1 << 31)  // SD卡已上电

#define SD_CMD_ARG(arg) (rSDICARG = (arg))
#define SD_CMD(index, is_resp, is_long) \
  (rSDICCON =                           \
       (((is_long) << 10) | (is_resp) << 9) | (1 << 8) | (0x40 + (index)))

// 定义SDI数据控制寄存器位
#define SD_DATCON_DatMode_Sent (3 << 12)  // 数据传输模式：发送
#define SD_DATCON_DatMode_Recv (2 << 12)  // 数据传输模式：接收
#define SD_DATCON_DatMode_Busy (1 << 12)  // 数据传输模式：只检查忙信号
#define SD_DATCON_DatTranStart (1 << 14)  // 数据传输开始
#define SD_DATCON_WideBus (1 << 16)       // 宽总线
#define SD_DATCON_BlockMode (1 << 17)     // 块模式
#define SD_DATCON_BACMD (1 << 18)         // busy接收在命令发送后开始
#define SD_DATCON_RACMD (1 << 19)  // 数据接收在命令发送后开始
#define SD_DATCON_TARSP (1 << 20)  // 数据传输在响应结束后开始
#define SD_DATCON_DataSize_Word (2 << 22)  // fifo的传输数据大小为字传输

// 重置数据控制寄存器
#define SD_DATCON_RESET() (rSDIDCON = rSDIDCON & (~(7 << 12)))
// 进行读操作
#define SD_DATCON_DO_READ(block_cnt)                                          \
  (rSDIDCON =                                                                 \
       (SD_DATCON_DataSize_Word | SD_DATCON_RACMD | SD_DATCON_BlockMode |     \
        SD_DATCON_WideBus | SD_DATCON_DatTranStart | SD_DATCON_DatMode_Recv | \
        (block_cnt << 0)))
// 进行写操作
#define SD_DATCON_DO_WRITE(block_cnt)                                         \
  (rSDIDCON =                                                                 \
       (SD_DATCON_DataSize_Word | SD_DATCON_TARSP | SD_DATCON_BlockMode |     \
        SD_DATCON_WideBus | SD_DATCON_DatTranStart | SD_DATCON_DatMode_Sent | \
        (block_cnt << 0)))
// 进行检测忙操作
#define SD_DATCON_DO_DetectBusy(block_cnt)                                 \
  (rSDIDCON = (SD_DATCON_BACMD | SD_DATCON_BlockMode | SD_DATCON_WideBus | \
               SD_DATCON_DatTranStart | SD_DATCON_DatMode_Busy | block_cnt))

// 定义数据状态寄存器
#define SD_DATSTA_BusyFin (1 << 3)  // 忙结束
#define SD_DATSTA_DatFin (1 << 4)   // 数据传输结束
#define SD_DATSTA_DatTout (1 << 5)  // 数据传输超时
#define SD_DATSTA_DatCrc (1 << 6)   // 数据接收crc失败
#define SD_DATSTA_CrcSta (1 << 7)   // crc状态失败

// 重置数据状态寄存器
#define SD_DATSTA_RESET()                                                 \
  (rSDIDSTA = (SD_DATSTA_BusyFin | SD_DATSTA_DatFin | SD_DATSTA_DatTout | \
               SD_DATSTA_DatCrc | SD_DATSTA_CrcSta))
// 是否接收到忙结束
#define SD_DATSTA_IS_BusyFin() (rSDIDSTA & SD_DATSTA_BusyFin)
// 清除忙结束标志
#define SD_DATSTA_CLR_BusyFin() (rSDIDSTA = SD_DATSTA_BusyFin)
// 是否数据传输结束
#define SD_DATSTA_IS_DatFin() (rSDIDSTA & SD_DATSTA_DatFin)
// 清除数据传输结束标志
#define SD_DATSTA_CLR_DatFin() (rSDIDSTA = SD_DATSTA_DatFin)
// 是否数据传输超时
#define SD_DATSTA_IS_DatTout() (rSDIDSTA & SD_DATSTA_DatTout)
// 清除数据传输超时标志
#define SD_DATSTA_CLR_DatTout() (rSDIDSTA = SD_DATSTA_DatTout)
// 是否接收数据crc失败
#define SD_DATSTA_IS_DatCrc() (rSDIDSTA & SD_DATSTA_DatCrc)
// 清除接收数据crc失败标志
#define SD_DATSTA_CLR_DatCrc() (rSDIDSTA = SD_DATSTA_DatCrc)
// 是否crc状态失败
#define SD_DATSTA_IS_CrcSta() (rSDIDSTA & SD_DATSTA_CrcSta)
// 清除crc状态失败标志
#define SD_DATSTA_CLR_CrcSta() (rSDIDSTA = SD_DATSTA_CrcSta)

// 定义fifo状态寄存器位
#define SD_FIFOSTA_RFLast (1 << 9)  // fifo中的数据是最后一个数据
#define SD_FIFOSTA_RxOK (1 << 12)   // fifo中有数据可接收
#define SD_FIFOSTA_TxOK (1 << 13)   // fifo中有空间可发送
#define SD_FIFOSTA_FFail (1 << 14)  // fifo中有错误
#define SD_FIFOSTA_Rset (1 << 16)   // fifo中的数据已被重置

#define SD_FIFO_RSET() (rSDIFSTA = (SD_FIFOSTA_Rset | SD_FIFOSTA_FFail))
#define SD_FIFO_CAN_READ() (rSDIFSTA & SD_FIFOSTA_RxOK)
#define SD_FIFO_CAN_WRITE() (rSDIFSTA & SD_FIFOSTA_TxOK)
#define SD_FIFO_READ_LAST() (rSDIFSTA & SD_FIFOSTA_RFLast)
#define SD_FIFO_CLEAR_LAST() (rSDIFSTA = SD_FIFOSTA_RFLast)

int sd_open(disk_t *disk);
void sd_close(disk_t *disk);
int sd_read(disk_t *disk, uint32_t block_addr, char *buff,
            uint32_t block_cnt);
int sd_write(disk_t *disk, uint32_t block_addr, char *buff,
             uint32_t block_cnt);
int sd_control(disk_t *disk, int cmd, int arg0, int arg1);

#endif /*__SD_H___*/
