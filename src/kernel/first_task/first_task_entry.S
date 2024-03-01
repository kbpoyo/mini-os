/**
 * @file first_task.S
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 第一个任务进程，实现与操作系统的代码隔离
 * @version 0.1
 * @date 2023-04-08
 * 
 * @copyright Copyright (c) 2023
 * 
 */

	.text
	.global first_task_entry
    .extern first_main

//第一个任务进程入口函数
first_task_entry:
	mov r0, #0
	mov r1, r0
	mov r2, r0
	mov r3, r0
	mov r4, r0
	mov r5, r0
	mov r6, r0
	mov r7, r0
	mov r8, r0
	mov r9, r0
	mov r10, r0
	mov r11, r0
	mov r12, r0
	mov r14, r0
	

	
    //因为cs寄存器已由操作系统分配，所以直接跳转即可
	b first_main