#include "dev/sd.h"

#include "common/os_config.h"
#include "common/register_addr.h"
#include "common/types.h"
#include "core/disk.h"
#include "tools/log.h"

#define INICLK 400000   // sd卡在初始化时的时钟频率为400khz
#define SDCLK 25000000  // 25mhz

#define POL 0
#define INT 1
#define DMA 2

static sd_info_t sd_info = {.bus_width = 0, .block_cnt = 0, .block_size = 512};

static void sdi_delay(uint32_t ms) {
  for (uint32_t i = 0; i < ms; i++) {
    for (uint32_t j = 0; j < 400; j++)
      ;
  }
}

int sd_init(void) {
  // 1.先配置SDLCK为400khz
  rSDIPRE = OS_PCLK / (INICLK)-1;  // 400KHz
  log_printf("SD Init. Frequency is %dHz\n", (OS_PCLK / (rSDIPRE + 1)));

  // 设置按A方式读取(小端字节序),并使能时钟
  // （按半字访问）D[15:8]→D[7:0]
  rSDICON = (0 << 4) | 1;

  // fifo复位
  SD_FIFO_RSET();

  // 设置SD卡块大小为512字节
  rSDIBSIZE = SD_BLOCK_SIZE;

  // 设置命令等待响应的超时时间
  rSDIDTIMER = 0x7fffff;

  sdi_delay(1000);

  if (sd_cmd0()) {
    log_printf("SD in idle.\n");
  }

  if (sd_cmd8()) {
    log_printf("SD in version 2.0.\n");
  }

  if (sd_check_ocr()) {
    log_printf("SD in ready.\n");
  } else {
    log_printf("SD Initialize fail\nNo Card assertion\n");
    return -1;
  }

  // 执行cmd2命令读取cid
  do {
    SD_CMD_ARG(0);
    SD_CMD(2, 1, 1);  // cmd2，需要等待长响应
  } while (!sd_check_cmd_end(2, 1));

  log_printf("SD read cid end.\n");

  // 执行cmd3命令读取RCA,并检测一并返回的状态寄存器csr的状态是否正确
  do {
    SD_CMD_ARG(0);
    SD_CMD(3, 1, 0);

  } while (!sd_check_cmd_end(3, 1) || (rSDIRSP0 & 0x1e00 != 0x600));

  sd_info.rca = (rSDIRSP0 & 0xffff0000) >> 16;
  log_printf("SD RCA=0x%x\n", sd_info.rca);

  // 已完成识别，将时钟设置为25mhz
  rSDIPRE = OS_PCLK / (SDCLK)-1;
  log_printf("SD Frequency is %dHz\n", (OS_PCLK / (rSDIPRE + 1)));

  // 读取sd卡信息
  sd_cmd9();

  // 选中sd卡
  if (!sd_sel_desel(1)) {
    log_error("sd select error!\n");
    return -1;
  }

  // 设置4位总线模式
  sd_set_4bit_bus();

  // 取消片选
  sd_sel_desel(0);

  return 1;
}

/**
 * @brief 将SD卡设置为idle状态
 *
 * @return int
 */
int sd_cmd0(void) {
  //-- Make card idle state
  SD_CMD_ARG(0);
  SD_CMD(0, 0, 0);  // CMD0(stuff bit)

  return sd_check_cmd_end(0, 0);
}

/**
 * @brief 检测sd卡是否支持sd2.0
 *
 * @return int
 */
int sd_cmd8(void) {
  SD_CMD_ARG(0x1aa);
  SD_CMD(8, 1, 0);

  return sd_check_cmd_end(8, 1);
}

/**
 * @brief 读取csd寄存器寄存器卡信息
 *
 * @return int
 */
int sd_cmd9(void) {
  do {
    SD_CMD_ARG(sd_info.rca << 16);
    SD_CMD(9, 1, 1);

  } while (!sd_check_cmd_end(9, 1));

  sd_info.block_size = 512;
  sd_info.block_cnt = ((((rSDIRSP1 & 0x3f) << 16) | (rSDIRSP2 >> 16)) + 1) *
                      512 * (1024 / sd_info.block_size);
  log_printf("SD capacity = %dGB\n",
             sd_info.block_cnt / (1024 * 1024 * 1024 / sd_info.block_size));
  return 1;
}

/**
 * @brief 执行ACMD命令之前需要执行cmd55命令
 *
 * @return int
 */
int sd_cmd55(void) {
  SD_CMD_ARG(sd_info.rca << 16);
  SD_CMD(55, 1, 0);

  return sd_check_cmd_end(55, 1);
}

/**
 * @brief 检测命令是否正常结束,若命令超时返回0
 *
 * @param cmd
 * @param be_resp
 * @return int
 */
int sd_check_cmd_end(int cmd, int be_resp) {  // 0: Timeout
  int finish0;

  if (!be_resp) {  // 命令不需要响应

    finish0 = rSDICSTA;
    while (!(finish0 & SD_CMDSTA_CmdSent)) {
      // 检测命令发送完毕
      finish0 = rSDICSTA;
    }

    // 清除命令结束状态
    rSDICSTA = finish0 & (~(uint32_t)0xff);
    return 1;
  } else {  // 命令需要响应

    finish0 = rSDICSTA;
    while (!((finish0 & SD_CMDSTA_RspFin) | (finish0 & SD_CMDSTA_CmdTout))) {
      // 等待命令响应结束或超时
      finish0 = rSDICSTA;
    }

    if (cmd == 1 | cmd == 41) {  // CRC no check, CMD9 is a long Resp. command.
      // cmd1和cmd41不需要检查CRC
      if (finish0 & SD_CMDSTA_CmdTout) {  // 命令超时
        rSDICSTA = finish0 & (~(uint32_t)0xff);
        return 0;
      }

    } else {  // 进行CRC检测
      if ((finish0 & SD_CMDSTA_RspCrc) || (finish0 & SD_CMDSTA_CmdTout)) {
        log_error("CMD%d:rSDICSTA=0x%x, rSDIRSP0=0x%x\n", cmd, rSDICSTA,
                  rSDIRSP0);

        if (finish0 & SD_CMDSTA_CmdTout) {
          rSDICSTA = finish0 & (~(uint32_t)0xff);
          return 0;
        }
      }
    }

    rSDICSTA = finish0;
    return 1;
  }
}

/**
 * @brief 选中或取消选中sd卡
 *
 * @param sel_desel
 */
int sd_sel_desel(char sel_desel) {
  // 选中sd卡
  if (sel_desel) {
    for (int i = 0; i < 50; i++) {
      SD_CMD_ARG(sd_info.rca << 16);  // CMD7(RCA,stuff bit)
      SD_CMD(7, 1, 0);

      if (!sd_check_cmd_end(7, 1) || rSDIRSP0 & 0x1e00 != 0x800) {
        continue;
      } else {
        return 1;
      }
    }
  } else {
    for (int i = 0; i < 50; i++) {
      SD_CMD_ARG(0);
      SD_CMD(7, 0, 0);

      if (!sd_check_cmd_end(7, 0)) {
        continue;
      } else {
        return 1;
      }
    }

    return 0;
  }
}

/**
 * @brief 检测sd卡的工作电压范围是否合适，并检测sd卡是否上电
 *
 * @return int
 */
int sd_check_ocr(void) {
  // 检测sd卡的ocr，判断sd是否上电，且是否可在主机提供的电压范围工作
  for (int i = 0; i < 50; i++) {
    // 准备发送特殊的ACMD
    sd_cmd55();

    // 设置HCS位, ACMD41(SD OCR:2.7V~3.6V)
    SD_CMD_ARG(0xff8000 | (1 << 30));
    SD_CMD(41, 1, 0);

    //-- Check end of ACMD41
    if (sd_check_cmd_end(41, 1)) {
      uint32_t rsp = rSDIRSP0;
      if (!(rsp & SD_OCR_Power_UP)) {
        sdi_delay(1000);
        continue;
      }

      if (rsp & SD_OCR_HCS) {
        log_printf("SD support SDHC, capacity > 2GB.\n");
      } else {
        log_printf("SD support SDSC, capacity < 2GB.\n");
      }

      return 1;  // Success
    }
  }
  return 0;  // Fail
}

/**
 * @brief 设置sd卡的总线宽度为4bit
 *
 */
void sd_set_4bit_bus(void) {
  sd_info.bus_width = 1;
  sd_set_bus();
}

/**
 * @brief 设置sd卡的总线宽度
 *
 */
void sd_set_bus(void) {
  do {
    sd_cmd55();
    SD_CMD_ARG(sd_info.bus_width << 1);
    SD_CMD(6, 1, 0);

  } while (!sd_check_cmd_end(6, 1));
}

/**
 * @brief 检测数据是否成功传输完毕
 *
 * @return int
 */
int sd_check_data_end(void) {
  while (!(SD_DATSTA_IS_DatFin() | SD_DATSTA_IS_DatTout()))
    ;
  // 检测数据传输结束或超时
  // 数据fifo在一定时间未被读取后，会将数据持续计数寄存器置0
  // 也会将数据状态寄存器的[4]位置1，代表数据读取结束

  // 检测数据结束异常
  if (SD_DATSTA_IS_DatTout() || SD_DATSTA_IS_DatCrc() ||
      SD_DATSTA_IS_CrcSta()) {
    log_printf("DATA:rSDIDSTA=0x%x\n", rSDIDSTA);
    SD_DATSTA_RESET();
    return 0;
  }

  // 清除数据传输结束标志位
  SD_DATSTA_CLR_DatFin();
  return 1;
}

/**
 * @brief 检测busy信号是否结束
 *
 * @return int
 */
int sd_check_busy_end(void) {
  // 检测忙信号结束和超时
  while (!(SD_DATSTA_IS_BusyFin() | SD_DATSTA_IS_DatTout()))
    ;

  // 检测忙信号结束异常
  if (SD_DATSTA_IS_DatTout() || SD_DATSTA_IS_DatCrc() ||
      SD_DATSTA_IS_CrcSta()) {
    log_error("DATA: rSDIDSTA=0x%x\n", rSDIDSTA);
    SD_DATSTA_RESET();
    return 0;
  }

  // 清除忙结束标志位
  SD_DATSTA_CLR_BusyFin();
  return 1;
}

/**
 * @brief sd卡读取块数据
 *
 */
int sd_read_blocks(uint32_t first_block_addr, uint8_t *buf,
                   uint32_t block_cnt) {
  if (block_cnt < 0 || block_cnt > SD_BLOCK_CNT_MAX) {
    log_error("SD block count error!");
    return -1;
  }

  // 重置fifo
  SD_FIFO_RSET();

  // 进行读操作
  SD_DATCON_DO_READ(block_cnt);

  // CMD17/18(addr), 读取的第一块地址
  SD_CMD_ARG(first_block_addr);

  uint32_t read_cnt = 0;
  switch (POL) {
    case POL:
      for (int i = 0; i < 50; ++i) {
        if (block_cnt == 1) {  // 单块读取
          // CMD17命令执行单块读取
          SD_CMD(17, 1, 0);
          if (!sd_check_cmd_end(17, 1)) {
            continue;
          }
          break;
        } else {  // 多块读取
          // CMD18命令执行多块读取
          SD_CMD(18, 1, 0);
          if (!sd_check_cmd_end(18, 1)) {
            continue;
          }
          break;
        }
      }

      while (read_cnt < sd_info.block_size *
                            block_cnt) {  // 读取block个块的数据即block*128个字
        if (SD_DATSTA_IS_DatTout()) {  // 检测数据传输超时
          SD_DATSTA_CLR_DatTout();     // 清除标志位
          break;
        }
        if (SD_FIFO_CAN_READ()) {  // 检测FIFO中有数据
          *(buf++) = rSDIDAT8;
          read_cnt++;
        }
      }
      break;
    default:
      break;
  }

  // 检测数据是否传输结束
  if (!sd_check_data_end()) {
    log_error("sd read dat error!");
  }

  // 清除数据控制寄存器各标志位
  SD_DATCON_RESET();
  // 清除Rx fifo 最后数据就绪位标志
  SD_FIFO_CLEAR_LAST();

  if (block_cnt > 1) {  // 多块读取需要CMD12指令结束
    do {
      SD_CMD_ARG(0);
      SD_CMD(12, 1, 0);
    } while (!sd_check_cmd_end(12, 1));
  }

  return read_cnt / sd_info.block_size;
}

/**
 * @brief sd卡写入块数据
 *
 */
int sd_write_blocks(uint32_t first_block_addr, uint8_t *buf,
                    uint32_t block_cnt) {
  if (block_cnt < 0 || block_cnt > SD_BLOCK_CNT_MAX) {
    log_error("SD block count error!");
    return -1;
  }

  // 重置fifo
  SD_FIFO_RSET();

  // 进行写操作
  SD_DATCON_DO_WRITE(block_cnt);

  // CMD24/25(addr),写入的第一块地址
  SD_CMD_ARG(first_block_addr);

  uint32_t write_cnt = 0;
  switch (POL) {
    case POL:
      for (int i = 0; i < 50; ++i) {
        if (block_cnt == 1) {  // 单块写入
          // CMD24
          SD_CMD(24, 1, 0);
          if (!sd_check_cmd_end(24, 1)) {
            continue;
          }
          break;
        } else {  // 多块写入
          SD_CMD(25, 1, 0);
          if (!sd_check_cmd_end(25, 1)) {
            continue;
          }
          break;
        }
      }
      while (write_cnt < sd_info.block_size * block_cnt) {
        if (SD_DATSTA_IS_DatTout()) {  // 检测数据传输超时
          SD_DATSTA_CLR_DatTout();     // 清除标志位
          break;
        }
        if (SD_FIFO_CAN_WRITE()) {  // 发现fifo未满
          rSDIDAT8 = *(buf++);
          write_cnt++;
        }
      }
      break;
    default:
      break;
  }

  // 检测数据是否传输结束
  if (!sd_check_data_end()) {
    log_error("sd write dat error!");
  }

  // 清除数据控制寄存器各标志位
  SD_DATCON_RESET();

  if (block_cnt > 1) {
    do {
      // 在发送命令后接收忙信号，检测忙结束
      SD_DATCON_DO_DetectBusy(block_cnt);

      // 执行CMD12命令,结束多块写入
      SD_CMD_ARG(0);
      SD_CMD(12, 1, 0);
    } while (!sd_check_cmd_end(12, 1));

    // 检测忙结束，判断是否执行完写操作
    if (!sd_check_busy_end()) {
      log_error("sd write dat error!");
    }
  }

  return write_cnt / sd_info.block_size;
}

// 定义驱动接口
int sd_open(disk_t *disk) {
  int ret = sd_init();

  disk->sector_count = sd_info.block_cnt;
  disk->sector_size = sd_info.block_size;
  return ret;
}

void sd_close(disk_t *disk) {}

/**
 * @brief sd卡外部读接口
 *
 * @param block_addr
 * @param buff
 * @param block_cnt
 * @return int
 */
int sd_read(disk_t *disk, uint32_t block_addr, char *buff, uint32_t block_cnt) {
  // 进行片选
  if (!sd_sel_desel(1)) {
    log_error("sd read error: select sd card failed!\n");
    return -1;
  }

  uint8_t *temp_buff = (uint8_t *)buff;
  int read_cnt = 0;
  for (int i = 0; i < block_cnt / SD_BLOCK_CNT_MAX; ++i) {
    read_cnt += sd_read_blocks(block_addr, temp_buff, SD_BLOCK_CNT_MAX);
    block_addr += SD_BLOCK_CNT_MAX;
    temp_buff += SD_BLOCK_CNT_MAX * sd_info.block_size / 4;
  }

  read_cnt +=
      sd_read_blocks(block_addr, temp_buff, block_cnt % SD_BLOCK_CNT_MAX);

  // 取消片选
  sd_sel_desel(0);

  return read_cnt;
}

/**
 * @brief sd卡外部写接口
 *
 * @param block_addr
 * @param buff
 * @param block_cnt
 * @return int
 */
int sd_write(disk_t *disk, uint32_t block_addr, char *buff,
             uint32_t block_cnt) {
  // 进行片选
  if (!sd_sel_desel(1)) {
    log_error("sd write error: select sd card failed!\n");
    return -1;
  }

  uint8_t *temp_buff = (uint8_t *)buff;
  int write_cnt = 0;
  for (int i = 0; i < block_cnt / SD_BLOCK_CNT_MAX; ++i) {
    write_cnt += sd_write_blocks(block_addr, temp_buff, SD_BLOCK_CNT_MAX);
    block_addr += SD_BLOCK_CNT_MAX;
    temp_buff += SD_BLOCK_CNT_MAX * sd_info.block_size / 4;
  }

  write_cnt +=
      sd_write_blocks(block_addr, temp_buff, block_cnt % SD_BLOCK_CNT_MAX);

  // 取消片选
  sd_sel_desel(0);

  return write_cnt;
}
int sd_control(disk_t *disk, int cmd, int arg0, int arg1) { return 0; }

disk_opt_t sd_opt = {
    .open = sd_open,
    .close = sd_close,
    .read = sd_read,
    .write = sd_write,
    .control = sd_control,
};