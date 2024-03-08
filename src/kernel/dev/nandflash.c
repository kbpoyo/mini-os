#include "dev/nandflash.h"

#include "tools/klib.h"
#include "tools/log.h"

// flash的缓存结构
typedef struct _nand_block_buff_t {
  // flash的块缓冲区
  char buff[FLASH_BLOCK_PAGE_COUNT * FLASH_PAGE_MAIN_SIZE / FLASH_SECTOR_SIZE]
           [FLASH_SECTOR_SIZE];

  // flash的块缓冲区记录的块的块号
  uint32_t number;
  // 缓冲区是否需要写回的标记
  uint8_t is_write_back;
} nand_block_buff_t;

static nand_block_buff_t nand_block_buff
    __attribute__((section(".data"), align(2 * 1024))) = {.number = 0xffffffff,
                                                          .is_write_back = 0};

/**
 * @brief 初始化nandflash
 *
 */
void nand_flash_init() {
  log_printf("nand_flash_init...\n");

  // 配置gpio引脚对应nandflash引脚
  rGPACON |= (0x3f << 17);  // 配置芯片引脚
  // 初始化配置寄存器
  rNFCONF |= NFCONF_FLASH_TIME_SER;
  // 初始化flash的控制寄存器
  rNFCONT = NFCONT_MODE | NFCONT_INITECC | NFCONT_MAINECC_LOCK |
            NFCONT_SPAREECC_LOCK | NFCONT_RnB_TRANS_MODE_UP_EDGE;

  log_printf("nand_flash_success..\n");
}

/**
 * @brief 发送页地址
 *
 * @param page_number
 */
static void send_addr(uint32_t page_number) {}

/**
 * @brief 复位flash
 *
 */
static void nand_reset() {
  NF_CE_OPEN();  // 使能片选

  NF_CLEAR_RB();  // 清除RnB的中断信号

  NF_CMD(CMD_RESET);  // 写入复位命令

  NF_DETECT_RB();  // 等待flash空闲

  NF_CE_CLOSE();  // 关闭片选
}

/**
 * @brief 读取一页数据
 *
 * @param page_number
 * @param buf
 * @return int
 */
int nand_read_page(uint32_t page_number, char *buf) {
  uint32_t mecc0, secc;

  NF_CE_OPEN();  // 打开nandflash片选

  NF_CLEAR_RB();  // 清RnB信号

  NF_RSTECC();  // 复位ECC

  NF_MECC_UnLock();  // 解锁main区ECC

  NF_CMD(CMD_READ1);  // 页读命令周期1

  // 写入5个地址周期
  NF_ADDR(0x00);  // 列地址A0~A7

  NF_ADDR(0x00);  // 列地址A8~A11

  NF_ADDR((page_number) & 0xff);  // 行地址A12~A19

  NF_ADDR((page_number >> 8) & 0xff);  // 行地址A20~A27

  NF_ADDR((page_number >> 16) & 0x1);  // 行地址A28

  NF_CMD(CMD_READ2);  // 页读命令周期2

  NF_DETECT_RB();  // 检测RnB信号上升沿跳变，即操作完成

  // 读取一页数据内容

  for (int i = 0; i < 2048; i++) {
    buf[i] = NF_RDDATA8();
  }

  NF_MECC_Lock();  // 锁定main区ECC值

  NF_SECC_UnLock();  // 解锁spare区ECC

  // 读spare区的前4个地址内容，即第2048~2051地址，这4个字节为main区的ECC
  mecc0 = NF_RDDATA();

  // 把读取到的main区的ECC校验码放入NFMECCD0/1的相应位置内
  rNFMECCD0 = ((mecc0 & 0xff00) << 8) | (mecc0 & 0xff);

  rNFMECCD1 = ((mecc0 & 0xff000000) >> 8) | ((mecc0 & 0xff0000) >> 16);

  NF_SECC_Lock();  // 锁定spare区的ECC值

  // 继续读spare区的4个地址内容，即第2052~2055地址，其中前2个字节为spare区的ECC值
  secc = NF_RDDATA();

  // 把读取到的spare区的ECC校验码放入NFSECCD的相应位置内
  rNFSECCD = ((secc & 0xff00) << 8) | (secc & 0xff);

  NF_CE_CLOSE();  // 关闭nandflash片选

  // 判断所读取到的数据是否正确
  if ((rNFESTAT0 & 0xf) == 0x0) {
    return 0;  // 正确
  } else {
    log_printf("read error: ECC err! page_number = %d.\n", page_number);
    // return -1;  // 错误
    return 0;  // TODO:以后做处理
  }
}

/**
 * @brief 随机写一字节数据
 *
 * @param page_number
 * @param add
 * @param dat
 * @return uint8_t
 */
uint8_t nand_random_write(uint32_t page_number, uint32_t add, uint8_t dat) {
  uint8_t stat;

  NF_CE_OPEN();  // 打开nandflash片选

  NF_CLEAR_RB();  // 清RnB信号

  NF_CMD(CMD_WRITE1);  // 页写命令周期1

  // 写入5个地址周期

  NF_ADDR(0x00);  // 列地址A0~A7

  NF_ADDR(0x00);  // 列地址A8~A11

  NF_ADDR((page_number) & 0xff);  // 行地址A12~A19

  NF_ADDR((page_number >> 8) & 0xff);  // 行地址A20~A27

  NF_ADDR((page_number >> 16) & 0x1);  // 行地址A28

  NF_CMD(CMD_RANDOMWRITE);  // 随意写命令

  // 页内地址
  NF_ADDR((char)(add & 0xff));  // 列地址A0~A7

  NF_ADDR((char)((add >> 8) & 0x0f));  // 列地址A8~A11

  NF_WRDATA8(dat);  // 写入数据

  NF_CMD(CMD_WRITE2);  // 页写命令周期2

  NF_DETECT_RB();

  NF_CMD(CMD_STATUS);  // 读状态命令

  // 判断状态值的第6位是否为1，即是否在忙，该语句的作用与NF_DETECT_RB();相同

  do {
    stat = NF_RDDATA8();

  } while (NF_STATE_IS_BUSY(stat));

  NF_CE_CLOSE();  // 关闭nandflash片选

  // 判断状态值的第0位是否为0，为0则写操作正确，否则错误

  if (NF_STATE_IS_OK(stat)) {
    return 0;  // 成功
  } else {
    log_printf("random write error!\n");
    return -1;  // 失败
  }
}

/**
 * @brief 随机读一字节数据
 *
 * @param page_number
 * @param add
 * @return uint8_t
 */
uint8_t nand_random_read(uint32_t page_number, uint32_t add) {
  NF_CE_OPEN();  // 打开nandflash片选

  NF_CLEAR_RB();  // 清RnB信号

  NF_CMD(CMD_READ1);  // 页读命令周期1

  // 写入5个地址周期

  NF_ADDR(0x00);  // 列地址A0~A7

  NF_ADDR(0x00);  // 列地址A8~A11

  NF_ADDR((page_number) & 0xff);  // 行地址A12~A19

  NF_ADDR((page_number >> 8) & 0xff);  // 行地址A20~A27

  NF_ADDR((page_number >> 16) & 0x1);  // 行地址A28

  NF_CMD(CMD_READ2);  // 页读命令周期2

  NF_DETECT_RB();  // 等待RnB信号变高，即不忙

  NF_CMD(CMD_RANDOMREAD1);  // 随意读命令周期1

  // 页内地址

  NF_ADDR((char)(add & 0xff));  // 列地址A0~A7

  NF_ADDR((char)((add >> 8) & 0x0f));  // 列地址A8~A11

  NF_CMD(CMD_RANDOMREAD2);  // 随意读命令周期2

  return NF_RDDATA8();  // 读取数据
}

uint8_t nand_is_bad_block(uint32_t block) {
  return nand_random_read(block * 64, 2054);
}

uint8_t nand_mark_bad_block(uint32_t block) {
  return nand_random_write(block * 64, 2054, MARK_BAD_BLOCK);
}

static uint8_t ecc_buf[4] __attribute__((section(".data")));

/**
 * @brief 写入一页
 *
 * @param page_number 页号
 * @return uint8_t
 */
uint8_t nand_write_page(uint32_t page_number, const char *buf) {
  uint32_t mecc0, secc;

  uint8_t stat, temp;

  temp = nand_is_bad_block(page_number >> 6);  // 判断该块是否为坏块

  if (temp == MARK_BAD_BLOCK) {
    log_printf("write error: this is a bad block! page_number = %d.\n",
               page_number);
    return -1;  // 是坏块，返回-1
  }

  NF_CE_OPEN();  // 打开nandflash片选

  NF_CLEAR_RB();  // 清RnB信号

  NF_RSTECC();  // 复位ECC

  NF_MECC_UnLock();  // 解锁main区的ECC

  NF_CMD(CMD_WRITE1);  // 页写命令周期1

  // 写入5个地址周期

  NF_ADDR(0x00);  // 列地址A0~A7

  NF_ADDR(0x00);  // 列地址A8~A11

  NF_ADDR((page_number) & 0xff);  // 行地址A12~A19

  NF_ADDR((page_number >> 8) & 0xff);  // 行地址A20~A27

  NF_ADDR((page_number >> 16) & 0x1);  // 行地址A28

  // 写入一页数据
  for (int i = 0; i < 2048; i++) {
    NF_WRDATA8(buf[i]);
  }

  NF_MECC_Lock();  // 锁定main区的ECC值

  mecc0 = rNFMECC0;  // 读取main区的ECC校验码

  // 把ECC校验码由字型转换为字节型，并保存到全局变量数组ecc_buf中

  ecc_buf[0] = (uint8_t)(mecc0 & 0xff);

  ecc_buf[1] = (uint8_t)((mecc0 >> 8) & 0xff);

  ecc_buf[2] = (uint8_t)((mecc0 >> 16) & 0xff);

  ecc_buf[3] = (uint8_t)((mecc0 >> 24) & 0xff);

  NF_SECC_UnLock();  // 解锁spare区的ECC

  // 把main区的ECC值写入到spare区的前4个字节地址内，即第2048~2051地址

  for (int i = 0; i < 4; i++) {
    NF_WRDATA8(ecc_buf[i]);
  }

  NF_SECC_Lock();  // 锁定spare区的ECC值

  secc = rNFSECC;  // 读取spare区的ECC校验码

  // 把ECC校验码保存到全局变量数组ecc_buf中
  ecc_buf[0] = (uint8_t)(secc & 0xff);
  ecc_buf[1] = (uint8_t)((secc >> 8) & 0xff);

  // 把spare区的ECC值继续写入到spare区的第2052~2053地址内
  for (int i = 0; i < 2; i++) {
    NF_WRDATA8(ecc_buf[i]);
  }

  NF_CMD(CMD_WRITE2);  // 页写命令周期2

  NF_DETECT_RB();  // 等待写入完成

  NF_CMD(CMD_STATUS);  // 读状态命令

  // 判断状态值的第6位是否为1，即是否在忙，该语句的作用与NF_DETECT_RB();相同
  do {
    stat = NF_RDDATA8();

  } while (NF_STATE_IS_BUSY(stat));

  NF_CE_CLOSE();  // 关闭nandflash片选

  // 判断状态值的第0位是否为0，为0则写操作正确，否则错误
  if (!NF_STATE_IS_OK(stat)) {
    log_printf("write error: this is a bad block! page_number = %d.\n",
               page_number);
    temp = nand_mark_bad_block(page_number >> 6);  // 标注该页所在的块为坏块

    if (temp == -1) {
      log_printf("write error: mark bad block failed! page_number = %d.\n",
                 page_number);
      return ERR_MARK_BAD_BLOCK;  // 标注坏块失败
    } else {
      return ERR_WRITE;  // 写操作失败
    }
  } else {
    return 0;  // 写操作成功
  }
}

/**
 * @brief 进行块擦除
 *
 * @param page_number
 * @return int
 */
int nand_block_erase(uint32_t page_number) {
  // 使能片选
  NF_CE_OPEN();

  NF_CLEAR_RB();  // 清RnB信号

  // 擦除命令1
  NF_CMD(CMD_ERASE1);

  // 写入5个地址周期

  NF_ADDR((page_number) & 0xff);  // 行地址A12~A19

  NF_ADDR((page_number >> 8) & 0xff);  // 行地址A20~A27

  NF_ADDR((page_number >> 16) & 0x1);  // 行地址A28

  // 擦除命令2
  NF_CMD(CMD_ERASE2);

  NF_DETECT_RB();  // 等待擦除完成

  NF_CMD(CMD_STATUS);  // 获取flash状态

  uint8_t stat = 0;

  do {
    stat = NF_RDDATA8();
  } while (NF_STATE_IS_BUSY(stat));

  // 关闭片选
  NF_CE_CLOSE();

  if (NF_STATE_IS_OK(stat)) {
    return 0;
  } else {
    log_error("erase error: erase block failed! page_number = %d.\n",
              page_number);
    return -1;
  }
}

/**
 * @brief 将block缓冲区固化到flash中
 *
 * @param block_number
 * @return int
 */
static int nand_write_buff_to_block() {
  // 判断当前缓冲区块是否存在，且缓冲区是否需要写回
  if (nand_block_buff.number == 0xffffffff ||
      nand_block_buff.is_write_back == 0) {
    return 0;
  }

  // 先进行块擦除
  int ret = nand_block_erase(nand_block_buff.number * FLASH_BLOCK_PAGE_COUNT);
  if (ret == -1) {
    return -1;
  }

  // 再进行块写
  uint32_t page_number = 0;
  for (int i = 0; i < FLASH_BLOCK_PAGE_COUNT; ++i) {
    page_number = nand_block_buff.number * FLASH_BLOCK_PAGE_COUNT + i;
    ret = nand_write_page(
        page_number,
        nand_block_buff.buff[i * FLASH_PAGE_MAIN_SIZE / FLASH_SECTOR_SIZE]);
    if (ret == -1) {
      return -1;
    }
  }
  // 更新缓冲区标志
  nand_block_buff.is_write_back = 0;

  return 0;
}

/**
 * @brief 将块内容读取到block缓冲区中
 *
 * @param block_number
 * @return int
 */
static int nand_read_block_to_buff(uint32_t block_number) {
  if (block_number < 0 || block_number >= FLASH_BLOCK_COUNT) {
    return -1;
  }

  // 当前需要读取的块在缓冲区,不需要再次读取
  if (block_number == nand_block_buff.number) {
    return 0;
  }

  // 当前需要读取的块不在缓冲区，需要再次读取
  // 先将缓冲区写回
  if (nand_block_buff.is_write_back) {
    int ret = nand_write_buff_to_block();
    if (ret ==
        -1) {  // TODO:之后可做错误处理，分配新的健康块进行写入，并且将该块标记为坏块
      log_error("nand block buff write back error! block number = %d\n",
                nand_block_buff.number);
      return -1;
    }
  }

  // 再读取新块到缓冲区
  uint32_t page_number = 0;
  for (int i = 0; i < FLASH_BLOCK_PAGE_COUNT; ++i) {
    page_number = block_number * FLASH_BLOCK_PAGE_COUNT + i;
    int ret = nand_read_page(
        page_number,
        nand_block_buff.buff[i * FLASH_PAGE_MAIN_SIZE / FLASH_SECTOR_SIZE]);
    if (ret == -1) {
      log_error("nand block read buff error! block number = %d\n",
                block_number);
      return -1;
    }
  }

  // 更新缓冲块号和标记
  nand_block_buff.number = block_number;
  nand_block_buff.is_write_back = 0;

  return 0;
}

/**
 * @brief 参数校验
 *
 * @param addr
 * @param buf
 * @param size
 * @return int
 */
static int param_is_ok(int addr, char *buf, int size) {
  if (buf == 0) {
    return -1;
  }
  if (addr < 0 || size < 0 || (addr + size) >= FLASH_SECTOR_COUNT) {
    return -1;
  }

  return 0;
}

int nand_open() {
  nand_flash_init();

  return 0;
}

/**
 * @brief 读nand
 *
 * @param addr 读取的扇区号
 * @param buf 读取缓冲区
 * @param size 读取扇区数
 * @return * int
 */
int nand_read(int addr, char *buf, int size) {
  if (param_is_ok(addr, buf, size) == -1) {
    return -1;
  }

  // 记录读取扇区数
  int cnt = 0;
  //  计算块内扇区号
  int block_in_sector = NF_BLOCK_IN_SECTOR_NUMBER(addr);
  // 计算该块内还有多少扇区可读
  int remain_sector_count = FLASH_BLOCK_SECTOR_COUNT - block_in_sector;

  // 将addr所属块读取到缓冲区中
  int ret = nand_read_block_to_buff(NF_BLOCK_NUMBER(addr));
  if (ret == -1) {
    log_error("nand read error! addr = %d\n", addr);
    return -1;
  }

  // 处理当前页
  int sector_count = size <= remain_sector_count ? size : remain_sector_count;
  kernel_memcpy(buf, nand_block_buff.buff[block_in_sector],
                sector_count * FLASH_SECTOR_SIZE);

  buf += sector_count * FLASH_SECTOR_SIZE;
  cnt += sector_count;
  size -= sector_count;

  if (size > 0) {  // 处理后续块
    // 计算待处理扇区的起始地址
    addr += sector_count;
    // 计算后续还有多少个完整块
    int remain_block_count = size / FLASH_BLOCK_SECTOR_COUNT;
    // 对涉及到的每一个块进行处理
    for (int i = 0; i < remain_block_count; ++i) {
      int block_number = NF_BLOCK_NUMBER(addr);
      nand_read_block_to_buff(block_number);

      kernel_memcpy(buf, nand_block_buff.buff[0], FLASH_BLOCK_MAIN_SIZE);

      addr += FLASH_BLOCK_SECTOR_COUNT;
      buf += FLASH_BLOCK_MAIN_SIZE;
      cnt += FLASH_BLOCK_COUNT;
    }

    // 对最后不完整块进行处理
    remain_sector_count = size % FLASH_BLOCK_SECTOR_COUNT;
    nand_read_block_to_buff(NF_BLOCK_NUMBER(addr));
    kernel_memcpy(buf, nand_block_buff.buff[0],
                  remain_sector_count * FLASH_SECTOR_SIZE);
    cnt += remain_sector_count;
  }

  return cnt;
}

/**
 * @brief 写nand
 *
 * @param dev
 * @param addr
 * @param buf
 * @param size 页数量
 * @return int
 */
int nand_write(int addr, char *buf, int size) {
  if (param_is_ok(addr, buf, size) == -1) {
    return -1;
  }

  // 记录读取扇区数
  int cnt = 0;
  //  计算块内扇区号
  int block_in_sector = NF_BLOCK_IN_SECTOR_NUMBER(addr);
  // 计算该块内还有多少扇区可读
  int remain_sector_count = FLASH_BLOCK_SECTOR_COUNT - block_in_sector;

  // 将addr所属块读取到缓冲区中
  int ret = nand_read_block_to_buff(NF_BLOCK_NUMBER(addr));
  if (ret == -1) {
    log_error("nand read error! addr = %d\n", addr);
    return -1;
  }

  // 处理当前块
  int sector_count = size <= remain_sector_count ? size : remain_sector_count;
  kernel_memcpy(nand_block_buff.buff[block_in_sector], buf,
                sector_count * FLASH_SECTOR_SIZE);
  // 标记该块需要写回
  nand_block_buff.is_write_back = 1;

  buf += sector_count * FLASH_SECTOR_SIZE;
  cnt += sector_count;
  size -= sector_count;

  if (size > 0) {  // 处理后续块
    // 计算待处理扇区的起始地址
    addr += sector_count;
    // 计算后续还有多少个完整块
    int remain_block_count = size / FLASH_BLOCK_SECTOR_COUNT;
    // 对涉及到的每一个块进行处理
    for (int i = 0; i < remain_block_count; ++i) {
      int block_number = NF_BLOCK_NUMBER(addr);
      nand_read_block_to_buff(block_number);

      kernel_memcpy(nand_block_buff.buff[0], buf, FLASH_BLOCK_MAIN_SIZE);
      nand_block_buff.is_write_back = 1;

      addr += FLASH_BLOCK_SECTOR_COUNT;
      buf += FLASH_BLOCK_MAIN_SIZE;
      cnt += FLASH_BLOCK_COUNT;
    }

    // 对最后不完整块进行处理
    remain_sector_count = size % FLASH_BLOCK_SECTOR_COUNT;
    nand_read_block_to_buff(NF_BLOCK_NUMBER(addr));
    kernel_memcpy(nand_block_buff.buff[0], buf,
                  remain_sector_count * FLASH_SECTOR_SIZE);
    nand_block_buff.is_write_back = 1;
    cnt += remain_sector_count;
  }

  return cnt;
}

/**
 * @brief 向nand发送控制指令
 *
 * @param dev
 * @param cmd
 * @param arg0
 * @param arg1
 * @return int
 */
int nand_control(int cmd, int arg0, int arg1) {
  switch (cmd) {
    case NF_CMD_WRITE_BACK:
      nand_write_buff_to_block();
      break;

    default:
      break;
  }
  return 0;
}

/**
 * @brief 关闭磁盘
 *
 * @param dev
 */
void nand_close() { nand_write_buff_to_block(); }
