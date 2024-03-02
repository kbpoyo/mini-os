/**
 * @file disk.c
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 磁盘
 * @version 0.1
 * @date 2023-08-08
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "core/disk.h"

#include "common/boot_info.h"
#include "common/cpu_instr.h"
#include "core/dev.h"
#include "core/irq.h"
#include "dev/nandflash.h"
#include "tools/klib.h"
#include "tools/log.h"

// 系统磁盘表
static disk_t disk_table[DISK_CNT]
    __attribute__((section(".data"), aligned(4))) = {{0}};
// 磁盘锁
static mutex_t mutex;

/**
 * @brief 从磁盘disk中sector_number扇区开始读取size个扇区的数据到buf中
 *
 * @param disk
 * @param sector_number 读取扇区号
 * @param buf
 * @param size  读取扇区数
 */
static int disk_read_data(disk_t *disk, uint32_t sector_number, void *buf,
                          int size) {
  return nand_read(sector_number, buf, size);
}

/**
 * @brief 从buf中向磁盘disk中的sector_number扇区开始写入size个扇区的数据
 *
 * @param disk
 * @param sector_number 写入扇区号
 * @param buf
 * @param size  读取扇区数
 */
static int disk_write_data(disk_t *disk, uint32_t sector_number, void *buf,
                           int size) {
  return nand_write(sector_number, buf, size);
}

/**
 * @brief 检测磁盘disk的分区表信息
 *
 * @param disk
 * @return int
 */
static int detect_part_info(disk_t *disk) {
  mbr_t mbr;
  // 读取0扇区的mbr
  int ret = nand_read(0, &mbr, 1);
  if (ret < 0) {
    log_error("read mbr failed!\n");
    return ret;
  }
  // 读取到disk的partinfo结构中
  part_item_t *item = mbr.part_item;
  partinfo_t *part_info = disk->partinfo + 1;
  for (int i = 1; i < MBR_PRIMARY_PART_NR; ++i, ++item, ++part_info) {
    part_info->type = item->system_id;
    if (part_info->type == FS_INVALID) {  // 无效分区，不使用
      part_info->total_sectors = 0;
      part_info->start_sector = 0;
      part_info->disk = (disk_t *)0;
    } else {  // 分区有效，记录分区信息
      kernel_sprintf(part_info->name, "%s%d", disk->name, i);
      part_info->disk = disk;
      part_info->start_sector = item->relative_sector;
      part_info->total_sectors = item->total_sectors;
    }
  }
}

/**
 * @brief 检测磁盘
 *
 * @param disk
 * @return int
 */
static int identify_disk(disk_t *disk) {
  //\保存了该磁盘的扇区总数量
  disk->sector_count = FLASH_SECTOR_COUNT;
  disk->sector_size = SECTOR_SIZE;

  // 初始化磁盘分区信息
  // 用partinfo将整个磁盘视为一个大分区
  partinfo_t *part_info = disk->partinfo + 0;
  part_info->disk = disk;
  kernel_sprintf(part_info->name, "%s%d", disk->name, 0);
  part_info->start_sector = 0;
  part_info->total_sectors = disk->sector_count;
  part_info->type = FS_INVALID;

  // 读取并检测磁盘的分区表信息
  detect_part_info(disk);

  return 0;
}

/**
 * @brief 打印磁盘相关信息
 *
 * @param disk
 */
static void print_disk_info(disk_t *disk) {
  log_printf("%s\n", disk->name);
  log_printf("\ttotal size: %d m\n",
             disk->sector_count * disk->sector_size / (1024 * 1024));

  for (int i = 0; i < DISK_PRIMARY_PART_CNT; ++i) {
    partinfo_t *part_info = disk->partinfo + i;
    if (part_info->type != FS_INVALID) {
      log_printf("\t%s: type: 0x%x, start sector: %d, sector count: %d\n",
                 part_info->name, part_info->type, part_info->start_sector,
                 part_info->total_sectors);
    }
  }
}

/**
 * @brief 初始化系统磁盘信息
 *
 */
void disk_init(void) {
  log_printf("disk init...\n");

  // 打开nandflash
  nand_open();

  kernel_memset(disk_table, 0, sizeof(disk_table));

  // 初始化磁盘锁与操作信号量
  mutex_init(&mutex);

  // 遍历并初始化化primary信道上的磁盘信息
  for (int i = 0; i < DISK_PER_CHANNEL; ++i) {
    disk_t *disk = disk_table + i;
    kernel_sprintf(disk->name, "sd%c", i + 'a');
    disk->mutex = &mutex;

    int err = identify_disk(disk);
    if (err == 0) {
      print_disk_info(disk);
    }
  }
}

/**
 * @brief 打开磁盘设备
 *
 * @param dev
 * @return int
 */
int disk_open(device_t *dev) {
  // 对磁盘的编号为 a , b
  // 对扇区的编号为0, 1, 2, 3, 4, 0分区包含整个磁盘
  // 设备索引编号0xa0表示 a磁盘上的0分区

  // 获取磁盘在系统磁盘表中的索引
  int disk_index = (dev->dev_index >> 4) - 0xa;
  // 获取分区的索引
  int part_index = dev->dev_index & 0xf;

  if (disk_index >= DISK_CNT || part_index >= DISK_PRIMARY_PART_CNT) {
    log_printf("device index error: %d\n", dev->dev_index);
    return -1;
  }

  // 获取磁盘对象
  disk_t *disk = disk_table + disk_index;
  if (disk->sector_count == 0) {
    log_printf("disk not exist, device: sd%d", dev->dev_index);
    return -1;
  }

  // 获取分区对象
  partinfo_t *part_info = disk->partinfo + part_index;
  if (part_info->total_sectors == 0) {
    log_printf("part not exist\n");
    return -1;
  }

  // 记录分区信息
  dev->data = (void *)part_info;
  return 0;
}
/**
 * @brief 读磁盘
 *
 * @param dev 设备对象，记录了磁盘分区信息
 * @param addr 读取的起始扇区相对于dev指定分区的偏移量
 * @param buf 读取缓冲区
 * @param size 读取扇区数
 * @return * int
 */
int disk_read(device_t *dev, int addr, char *buf, int size) {
  // 获取要读取的分区信息
  partinfo_t *part_info = (partinfo_t *)dev->data;
  if (!part_info) {
    log_printf("Get part info failed. devce: %d\n", dev->dev_index);
    return -1;
  }

  // 获取磁盘对象
  disk_t *disk = part_info->disk;
  if (disk == (disk_t *)0) {
    log_printf("No disk, device: %d\n", dev->dev_index);
    return -1;
  }

  // TODO:加锁
  mutex_lock(disk->mutex);  // 确保磁盘io操作的原子性

  int cnt = disk_read_data(disk, part_info->start_sector + addr, buf, size);
  if (cnt < 0) {
    log_error("disk[%s] read error: start sector %d, count: %d", disk->name,
              addr, size);
    cnt = -1;
  }

  // TODO:解锁
  mutex_unlock(disk->mutex);

  return cnt;
}

/**
 * @brief 写磁盘
 *
 * @param dev
 * @param addr
 * @param buf
 * @param size
 * @return int
 */
int disk_write(device_t *dev, int addr, char *buf, int size) {
  // 获取要读取的分区信息
  partinfo_t *part_info = (partinfo_t *)dev->data;
  if (!part_info) {
    log_printf("Get part info failed. devce: %d\n", dev->dev_index);
    return -1;
  }

  // 获取磁盘对象
  disk_t *disk = part_info->disk;
  if (disk == (disk_t *)0) {
    log_printf("No disk, device: %d\n", dev->dev_index);
    return -1;
  }

  // TODO:加锁
  mutex_lock(disk->mutex);  // 确保磁盘io操作的原子性
  //
  int cnt = disk_write_data(disk, part_info->start_sector + addr, buf, size);

  if (cnt < 0) {
    log_error("disk[%s] read error: start sector %d, count: %d", disk->name,
              addr, size);
    cnt = -1;
  }

  // TODO:解锁
  mutex_unlock(disk->mutex);

  return cnt;
}

/**
 * @brief 向磁盘发送控制指令
 *
 * @param dev
 * @param cmd
 * @param arg0
 * @param arg1
 * @return int
 */
int disk_control(device_t *dev, int cmd, int arg0, int arg1) { return -1; }

/**
 * @brief 关闭磁盘
 *
 * @param dev
 */
void disk_close(device_t *dev) {}

// 操作disk结构的函数表
dev_desc_t dev_disk_desc = {.dev_name = "disk",
                            .open = disk_open,
                            .read = disk_read,
                            .write = disk_write,
                            .control = disk_control,
                            .close = disk_close};