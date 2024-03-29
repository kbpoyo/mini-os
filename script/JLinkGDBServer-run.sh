#!/bin/bash

#!/bin/bash
# 用法: ./killbyname.sh <任务名称>
# 例如: ./killbyname.sh firefox

# 获取任务名称作为参数
# PS="JLinkGDBServerCL"

# # 检查参数是否为空
# if [ -z "$PS" ]; then
#   echo "请输入任务名称"
#   exit 1
# fi

# # 用ps和grep命令查找匹配的进程id
# # 用-v选项排除grep自身的进程
# # 用awk命令提取第二列的进程id
# PIDS=$(ps -ef | grep "$PS" | grep -v grep | awk '{print $2}')

# # 检查是否找到了进程id
# if [ -z "$PIDS" ]; then
#   echo "没有找到匹配的进程"
#   exit 2
# fi

# # 用kill命令结束进程
# # 用-9选项强制结束进程
# # 用xargs命令将进程id作为参数传递给kill命令
# echo "正在结束以下进程:"
# echo "$PIDS"
# echo "$PIDS" | xargs kill -9

# # 检查是否成功结束进程
# if [ $? -eq 0 ]; then
#   echo "成功结束进程"
#   exit 0
# else
#   echo "结束进程失败"
#   exit 3
# fi


# 定义JLinkGDBServerCL的安装目录
JLINK_HOME=/mnt/f/software/common_tools/Jlink/JLink
# JLinkGDBServerCL启动项
GDB_SERVER_ARG="-select USB -device S3C2440A -endian little -if JTAG -speed auto -noir -noLocalhostOnly -nologtofile -port 2331 -SWOPort 2332 -TelnetPort 2333"
# 设备类型为STM32F103RB，接口类型为SWD，速度为4000kHz，端口号为2331
$JLINK_HOME/JLinkGDBServerCL.exe $GDB_SERVER_ARG
