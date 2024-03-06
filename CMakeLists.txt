# 顶层cmakelist文件，以下的设置将被子工程所继承使用
cmake_minimum_required(VERSION 3.0.0)

# 保存一些通用的配置
set(CMAKE_VERBOSE_MAKEFILE on)   # 开启输出编译详细过程的提示
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   #生成编译命令json文件

# gcc工具链前缀，根据实际情况做不同的设置
set(TOOL_PREFIX "arm-linux-")


# C编译器与参数配置
set(CMAKE_C_COMPILER "${TOOL_PREFIX}gcc") 
set(CMAKE_C_FLAGS " -O0 -Wall -fno-pie -fno-stack-protector -std=c99")

#-g参数用于生成调试信息，-c参数表示只编译不链接
#-O0参数表示关闭优化，-m32参数表示生成32位代码，
#-fno-pie参数表示不生成位置无关代码，
#位置无关代码是指代码无论被加载到哪个地址上都可以正常执行
#-fno-stack-protector参数表示不生成栈保护代码，
#栈保护代码是一种保护机制，用于检测和防止栈缓冲区溢出攻击。
#-fno-asynchronous-unwind-tables参数表示不生成异步取消表.

# 汇编器与参数配置
set(CMAKE_ASM_COMPILER "${TOOL_PREFIX}gcc")
# set(CMAKE_ASM_FLAGS "-g -c")
# set(CMAKE_ASM_SOURCE_FILE_EXTENSIONS "asm")

# 链接器工具
set(LINKER_TOOL "${TOOL_PREFIX}ld")

# 其它工具
set(OBJCOPY_TOOL "${TOOL_PREFIX}objcopy")
set(OBJDUMP_TOOL "${TOOL_PREFIX}objdump")
set(READELF_TOOL "${TOOL_PREFIX}readelf")

# 工程，启用C语言和汇编语言
project(mini-os LANGUAGES C)  
enable_language(ASM)

# 头文件搜索路径
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/inc/
    ${PROJECT_SOURCE_DIR}/src/kernel/inc
    ${PROJECT_SOURCE_DIR}/newlib/arm-myos/include
)

# # 底层的若干子项目：含内核及应用程序
add_subdirectory(./src/applib)
add_subdirectory(./src/shell)
add_subdirectory(./src/snake)
add_subdirectory(./src/collapse)
add_subdirectory(./src/loop)
add_subdirectory(./src/kernel)
add_subdirectory(./src/init)

# # 添加编译依赖，先生成app库，再生成kernel和shell
# # 不加则cmake则可能先编译shell和kernel，而缺少libapp，导致编译错误
add_dependencies(collapse app)       
add_dependencies(shell app)
add_dependencies(snake app)
# add_dependencies(kernel app)
add_dependencies(loop app)
