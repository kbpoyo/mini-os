ENTRY(_exception_handler)

SECTIONS {/*SECTIONS必须大写*/
    . = 0x30000000;

    /*记录只读段的起始地址*/
    PROVIDE(s_text = .);
     .text : {   /*冒号前必须有空格 */
      /* EXCLUDE_FILE(文件名) 表示排除该文件*/
         *(EXCLUDE_FILE(*first_task* *lib_syscall*) .text)
       . = ALIGN(4);
    }

    .rodata : {
       *(EXCLUDE_FILE(*first_task* *lib_syscall*) .rodata)
       . = ALIGN(4);
    }
    /* 记录只读段的结束地址 */
    /* 段结束地址为真正结束位置的下一个字节 */
    PROVIDE(e_text = .);


    /* 记录可读写段的起始地址 */
    . = ALIGN(1024); /*让当前地址按1kb对齐*/

    PROVIDE(s_data = .);
    .data : {    
         *(EXCLUDE_FILE(*first_task* *lib_syscall*) .data)
       . = ALIGN(4);
    }

    .bss : {
        *(EXCLUDE_FILE(*first_task* *lib_syscall*) .bss)
    }
    . = ALIGN(1024);
    PROVIDE(e_data = .);

    /*将第一个进程的各个段放到虚拟地址的0x80000000之后，物理地址紧挨着内核的四个段*/
    /*将进程的各个结放到一个大段 .first_task中*/
    .first_task 0x80000000 : AT(e_data) {
        /*格式为： 文件名(段名) ，*为通配符，方便查找到文件 */
        /*将以下待重定向文件的各个段放在0x80000000开始的地址空间中*/
        *first_task_entry*(.text .rodata .data .bss)
        *first_task*(.text .rodata .data .bss)
        *lib_syscall*(.text .rodata .data .bss)
    }
    /*LOADADDR(段名)：获取该段所在的物理地址paddr*/
    PROVIDE(s_first_task = LOADADDR(.first_task));

    /*SIZEOF(段名): 获取该段的大小size*/
    PROVIDE(e_first_task = LOADADDR(.first_task) + SIZEOF(.first_task)); /*获取在物理地址上紧邻.first_task段的地址*/

   /*记录内核四个段后的紧邻的空间地址, 
    该变量只被链接器认为地址为当前地址 . 
    但并不占用内存，即并不会改变当前地址 . 的值*/
    PROVIDE(mem_kernel_end = e_first_task);
 
 }

