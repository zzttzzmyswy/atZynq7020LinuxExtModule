###
 # @Description: 
 # @Version: 2.0
 # @Autor: ZZT
 # @Date: 2022-01-29 10:54:32
 # @LastEditors: ZZT
 # @LastEditTime: 2022-01-29 11:03:01
### 

env_file=/home/zzt/gcc-for-arm/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf.sh

source ${env_file}

if [ $SHELL == "/bin/zsh" ]; then
    zsh;
elif [ $SHELL == "/bin/csh" ]; then
    csh;
else
    bash;
fi