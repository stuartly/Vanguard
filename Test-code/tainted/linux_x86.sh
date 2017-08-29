#! /bin/sh

export SOURCE_BASE=$(pwd)


rm -rf ./obj
mkdir ./obj
cp ./linux_x86.mk ./obj
cd ./obj

make -f linux_x86.mk clean

make -f linux_x86.mk


rm ./linux_x86.mk
cd ..

find $(pwd) -name "*.ast" > astList.txt
