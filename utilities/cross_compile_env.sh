#!/bin/sh

export TARGETMACH=arm-linux-gnueabi
export BUILDMACH=x86_64-pc-linux-gnu
export CROSS=arm-linux-gnueabi
export CC=${CROSS}-gcc
export LD=${CROSS}-ld
export AS=${CROSS}-as
export CXX=${CROSS}-g++
