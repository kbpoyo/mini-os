/**
 * @file link.lds
 * @author kbpoyo (kbpoyo@qq.com)
 * @brief 外部程序的链接脚本
 * @version 0.1
 * @date 2023-05-12
 * 
 * @copyright Copyright (c) 2023
 **/


/*指定程序入口为_start, 即在链接的applib库中*/
ENTRY(_start)

SECTIONS {

    . = 0x80000000;

    .text : {
        *(*.text)
    }

    .rodata : {
        *(*.rodata)
    }

    .data : {
        *(*.data)
    }

    .bss : {
        /*程序bss段的起始地址*/
        PROVIDE(__bss_start__ = .);
        *(*.bss)
        PROVIDE(__bss_end__ = .);
    }


}