ENTRY(_start)

SECTIONS {/*SECTIONS必须大写*/
    . = 0;

    /*记录只读段的起始地址*/
    PROVIDE(s_text = .);
     .text : {   /*冒号前必须有空格 */
        _start = .;
       *(.text) 
       . = ALIGN(4);
    }

    .rodata : {
        *(.rodata)
       . = ALIGN(4);
    }
    /* 记录只读段的结束地址 */
    /* 段结束地址为真正结束位置的下一个字节 */
    PROVIDE(e_text = .);


    PROVIDE(s_data = .);
    .data : {    
        *(.data) 
       . = ALIGN(4);
    }

    .bss : {
        *(.bss)
       . = ALIGN(4);
    }
    PROVIDE(e_data = .);
 
 }

