#!/bin/sh


export PATH=`readlink -f ./buildroot-2014.02/output/host/usr/bin`:$PATH
export TARGETMACH=arm-buildroot-linux-uclibcgnueabi
export BUILDMACH=x86_64-pc-linux-gnu
export CROSS=arm-linux
export CC=${CROSS}-gcc
export LD=${CROSS}-ld
export AS=${CROSS}-as
export CXX=${CROSS}-g++
