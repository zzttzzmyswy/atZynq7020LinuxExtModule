###
 # @Description: 
 # @Version: 2.0
 # @Autor: ZZT
 # @Date: 2022-01-29 10:54:32
 # @LastEditors: ZZT
 # @LastEditTime: 2022-01-29 16:19:41
### 

env_file=/home/zzt/gcc-for-arm/gcc-linaro-11.2.1-2022.01-x86_64_arm-linux-gnueabihf.sh

source ${env_file}

export KERNELDIR=/home/zzt/xilinx/project/petalinux/kernel_no1/src/linux-xlnx

if [ $SHELL == "/bin/zsh" ]; then
    zsh;
elif [ $SHELL == "/bin/csh" ]; then
    csh;
else
    bash;
fi