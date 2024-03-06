/**
 * @file crt0.S
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief c运行时库,加载程序前的初始化工作汇编部分
 * @version 0.1
 * @date 2023-05-12
 * 
 * @copyright Copyright (c) 2023
 **/


    .text
    .global _start
    .extern cstart
_start:
    //因为cs寄存器已由操作系统分配，所以直接跳转即可
	b cstart
