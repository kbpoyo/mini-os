
project(execpt LANGUAGES C)


# 加入相应的库
set(LIBS_FLAGS "-L ${CMAKE_SOURCE_DIR}/newlib/arm-myos/lib -lm -lc -L /home/kbpoyo/opt/FriendlyARM/toolschain/4.4.3/lib/gcc/arm-none-linux-gnueabi/4.4.3/ -lgcc")
set(CMAKE_EXE_LINKER_FLAGS "-T ${PROJECT_SOURCE_DIR}/link.lds ${LIBS_FLAGS}")
set(CMAKE_C_LINK_EXECUTABLE "${LINKER_TOOL} <OBJECTS> ${CMAKE_EXE_LINKER_FLAGS} -o ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf")

include_directories(
    ${PROJECT_SOURCE_DIR}/../applib/
)

# 将所有的汇编、C文件加入工程
# 注意保证start.asm在最前头
file(GLOB C_LIST  "*.S" "*.c" "*.h" "../applib/*.S" "../applib/*.c" "../applib/*.h")
add_executable(${PROJECT_NAME} ${C_LIST})

# 不带调试信息的elf生成，何种更小，写入到image目录下
add_custom_command(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${OBJCOPY_TOOL} -S ${PROJECT_NAME}.elf ${CMAKE_SOURCE_DIR}/image/${PROJECT_NAME}.elf
                   COMMAND ${OBJDUMP_TOOL} -x -d -S ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf > ${PROJECT_NAME}_dis.txt
                   COMMAND ${READELF_TOOL} -a ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf > ${PROJECT_NAME}_elf.txt
)