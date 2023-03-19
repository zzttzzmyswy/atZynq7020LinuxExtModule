###
 # @Description: 
 # @Version: 2.0
 # @Autor: ZZT
 # @Date: 2022-01-29 10:54:32
 # @LastEditors: ZZT
 # @LastEditTime: 2022-03-26 22:30:57
### 

env_dir=/home/zzt/gcc-for-arm
# env_file=gcc-linaro-11.2.1-2022.01-x86_64_arm-linux-gnueabihf.sh
env_file=gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf.sh

source ${env_dir}/${env_file}

export KERNELDIR=/home/zzt/xilinx/project/petalinux/kernel/src/kernel/linux-xlnx-xlnx_rebase_v5.10_2021.2

if [[ $SHELL =~ "/bin/zsh" ]]; then
    zsh;
elif [[ $SHELL =~ "/bin/csh" ]]; then
    csh;
else
    bash;
fi