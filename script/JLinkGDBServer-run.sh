#!/bin/bash
# 定义JLinkGDBServerCL的安装目录
JLINK_HOME=/mnt/f/software/common_tools/Jlink/JLink_V794a
# JLinkGDBServerCL启动项
GDB_SERVER_ARG="-select USB -device S3C2440A -endian little -if JTAG -speed auto -noir -noLocalhostOnly -nologtofile -port 2331 -SWOPort 2332 -TelnetPort 2333"
# 设备类型为STM32F103RB，接口类型为SWD，速度为4000kHz，端口号为2331
$JLINK_HOME/JLinkGDBServerCL.exe $GDB_SERVER_ARG
