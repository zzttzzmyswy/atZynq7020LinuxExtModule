###
 # @Description: 
 # @Version: 2.0
 # @Autor: ZZT
 # @Date: 2022-01-29 10:54:32
 # @LastEditors: ZZT
 # @LastEditTime: 2022-01-29 11:45:46
### 

env_file=/home/zzt/gcc-for-arm/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf.sh

source ${env_file}

export CROSS_COMPILE=arm-none-linux-gnueabihf-
export ARCH=arm
export CC=${CROSS_COMPILE}gcc
export LD=${CROSS_COMPILE}ld 
export KERNELDIR=/home/zzt/xilinx/project/petalinux/kernel_no1/src/linux-xlnx

if [ $SHELL == "/bin/zsh" ]; then
    zsh;
elif [ $SHELL == "/bin/csh" ]; then
    csh;
else
    bash;
fi