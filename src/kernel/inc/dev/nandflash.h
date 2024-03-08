#ifndef NANDFLASH_H
#define NANDFLASH_H
#include "common/register_addr.h"
#include "common/types.h"
#include "core/disk.h"
/**
 * @brief
 *
 * tCLS = tALS = 12ns   命令(地址)使能建立时间
 *  twp = 12ns  写使能脉冲宽度（pulse width），即写使能等待有效时间
 * tclh = talh = 5ns    命令(地址)使能持续时间
 *
 * TACLS >= max(tCLS,tALS) - tWP;
 * TWRPH0 >= tWP;
 * TWRPH1 >= max(tCLH,tALH);
 *
 * 读取NFCONF低4位可知该flash的配置：
 *      总线宽度：8
 *      地址周期：5
 *      页大小：2kb
 *
 * 列地址：A0~A10 用来确定0~2047这2kb业内地址
 * A11: 0表示页内地址， 1表示页oob地址(2048~2111)64B地址
 * 行地址：A12~A28 用来确定页号
 *
 *  !这里将flash每一页抽象为4个512字节大小的分区
 *  扇区数量：2048(块)*64(页)*4(扇区)
 *
 */

#define FLASH_PAGE_MAIN_SIZE (2 * 1024)
#define FLASH_PAGE_SPARE_SIZE 64
#define FLASH_BLOCK_MAIN_SIZE (128 * 1024)
#define FLASH_BLOCK_SPARE_SIZE (4 * 1024)
#define FLASH_MAIN_SIZE (256 * 1024 * 1024)
#define FLASH_SPARE_SIZE (8 * 1024 * 1024)
#define FLASH_PAGE_COUNT (FLASH_MAIN_SIZE / FLASH_PAGE_MAIN_SIZE)
#define FLASH_BLOCK_COUNT (FLASH_MAIN_SIZE / FLASH_BLOCK_MAIN_SIZE)
#define FLASH_BLOCK_PAGE_COUNT (FLASH_BLOCK_MAIN_SIZE / FLASH_PAGE_MAIN_SIZE)
#define FLASH_SECTOR_SIZE 512
#define FLASH_BLOCK_SECTOR_COUNT (FLASH_BLOCK_MAIN_SIZE / FLASH_SECTOR_SIZE)
#define FLASH_SECTOR_COUNT (FLASH_MAIN_SIZE / FLASH_SECTOR_SIZE)

#define FLASH_MAIN_ECC_ADDR 2048
#define FLASH_SPARE_ECC_ADDR (FLASH_MAIN_ECC_ADDR + 4)

/*
0x32F018 = 0011 0010 1111 0000 0001 1000；
由此可得到实际发送的数据
A7  - A0  =  0001 1000               0xFF&col
A10  - A8  =  000                    0x07 & (col >> 8)
A11 = 0 访问存储区，A11=1访问oob；
A19  - A12 =  0101 1110             0xFF & page
A27  - A20 =  0000 0110             0xFF & (page >> 8)
A28 = 0                                        0x01 & (page >> 16)
其中：地址 = 64*2048*块号 + 2048 * 页号 + 页内地址；
 列地址 col = A10 - A0 = 地址 & （页大小-1）= 0x32F018 & 7FF= 0x018 = 000  0001
1000，列地址表示的是某页内的2k地址； 页地址 page = A28 - A12 = 地址/页大小 =
0x32F018 / 0x800 = 0x65E =  0 0000 0110 0101
1110，页地址表示对应的某一页，或者说为行地址；
*/

#define TACLS 0   // tacls = TACLS * tHCLK = 0
#define TWRPH0 1  // twrph0 = (TWRPH0 + 1) * tHCLK = 20ns > twp = 12ns
#define TWRPH1 \
  0  // twrph1 = (TWRPH1 + 1) * tHCLK = 10ns > max(tclh, talh) =
     // 5ns

// 初始化flash的配置寄存器寄存器
/// 设置nandflash时序
#define NFCONF_FLASH_TIME_SER ((TACLS << 12) | (TWRPH0 << 8) | (TWRPH1 << 4))

// 初始化flash的控制寄存器
/// 设置flash模式
#define NFCONT_MODE (1 << 0)           // 使能flash
#define NFCONT_INITECC (1 << 4)        // 初始化ECC编码器和译码器
#define NFCONT_MAINECC_LOCK (1 << 5)   // 锁定主数据区ECC生成
#define NFCONT_SPAREECC_LOCK (1 << 6)  // 锁定备份区ECC生成
#define NFCONT_RnB_TRANS_MODE_UP_EDGE (0 << 8)    // RnB传输检测上升沿
#define NFCONT_RnB_TRANS_MODE_DOWN_EDGE (0 << 8)  // RnB传输检测下降沿

#define NFCONT_EnbRnBINT (1 << 9)  // 使能RnB中断,即数据准备好时产生中断
#define NFCONT_Enbillegal_ACCINT (1 << 10)  // 使能非法访问中断
#define NFCONT_SOFT_LOCK (1 << 12)          // 使能软件上锁
#define NFCONT_LOCK_TIGHT (1 << 13)         // 使能紧锁

// 定义nandflash命令
#define CMD_READ1 0x00  // 页读命令周期1
#define CMD_READ2 0x30  // 页读命令周期2

#define CMD_READID 0x90  // 读ID命令

#define CMD_WRITE1 0x80  // 页写命令周期1
#define CMD_WRITE2 0x10  // 页写命令周期2

#define CMD_ERASE1 0x60  // 块擦除命令周期1
#define CMD_ERASE2 0xd0  // 块擦除命令周期2

#define CMD_STATUS 0x70  // 读状态命令

#define CMD_RESET 0xff  // 复位

#define CMD_RANDOMREAD1 0x05  // 随意读命令周期1
#define CMD_RANDOMREAD2 0xE0  // 随意读命令周期2

#define CMD_RANDOMWRITE 0x85  // 随意写命令

// 定义具体的nandflash状态标志
#define NF_STATE_IS_OK(state) (!(state & 0x1))
#define NF_STATE_IS_BUSY(state) (!(state & (1 << 6)))

// 坏块标记
#define MARK_BAD_BLOCK 0x33

// 定义错误状态标志
#define ERR_MARK_BAD_BLOCK 0x10
#define ERR_WRITE 0x11
#define ERR_READ 0x12

// 定义相关指令
#define NF_CMD(data) \
  { rNFCMD = (data); }  // 传输命令

#define NF_ADDR(addr) \
  { rNFADDR = (addr); }  // 传输地址

#define NF_RDDATA() (rNFDATA)  // 读32位数据

#define NF_RDDATA8() (rNFDATA8)  // 读8位数据

#define NF_WRDATA(data) \
  { rNFDATA = (data); }  // 写32位数据

#define NF_WRDATA8(data) \
  { rNFDATA8 = (data); }  // 写8位数据

#define NF_CE_OPEN() \
  { rNFCONT &= ~(1 << 1); }  // 打开nandflash片选

#define NF_CE_CLOSE() \
  { rNFCONT |= (1 << 1); }  // 关闭nandflash片选

#define NF_RSTECC() \
  { rNFCONT |= (1 << 4); }  // 复位ECC

#define NF_MECC_UnLock() \
  { rNFCONT &= ~(1 << 5); }  // 解锁main区ECC

#define NF_MECC_Lock() \
  { rNFCONT |= (1 << 5); }  // 锁定main区ECC

#define NF_SECC_UnLock() \
  { rNFCONT &= ~(1 << 6); }  // 解锁spare区ECC

#define NF_SECC_Lock() \
  { rNFCONT |= (1 << 6); }  // 锁定spare区ECC

#define NF_WAITRB()               \
  {                               \
    while (!(rNFSTAT & (1 << 0))) \
      ;                           \
  }  // 等待nandflash不忙

#define NF_DETECT_RB()            \
  {                               \
    while (!(rNFSTAT & (1 << 2))) \
      ;                           \
  }

#define NF_CLEAR_RB() \
  { rNFSTAT |= (1 << 2); }  // 清除RnB信号

// 由总扇区号计算块号
#define NF_BLOCK_NUMBER(sector) (sector >> 8)
// 由总扇区号计算块内扇区号
#define NF_BLOCK_IN_SECTOR_NUMBER(sector) (sector & 0xff)

// 定义控制指令
#define NF_CMD_WRITE_BACK DISK_CMD_WRITE_BACK

int nand_open();

int nand_read(int addr, char* buf, int size);
int nand_write(int addr, char* buf, int size);

void nand_close();
int nand_control(int cmd, int arg0, int arg1);
#endif