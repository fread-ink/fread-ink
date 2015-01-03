#!/bin/sh

export BUILDROOT_HOST_PATH=/home/juul/projects/fread.ink/fread-ink/system/buildroot-2014.02/output/host
export BUILDROOT_TARGET_PATH=/home/juul/projects/fread.ink/fread-ink/system/buildroot-2014.02/output/target
export PKG_CONFIG=${BUILDROOT_HOST_PATH}/usr/bin/pkgconf
export PKG_CONFIG_SYSROOT_DIR=""
export PKG_CONFIG_PATH=`readlink -f output/lib/pkgconfig`:`readlink -f output/share/pkgconfig`
export LD_LIBRARY_PATH=${BUILDROOT_TARGET_PATH}/lib:${BUILDROOT_TARGET_PATH}/usr/lib:`readlink -f ./output/lib`:`readlink -f ./output/usr/lib`
export C_INCLUDE_PATH=${BUILDROOT_TARGET_PATH}/include:${BUILDROOT_TARGET_PATH}/usr/include:`readlink -f ./output/include`:`readlink -f ./output/usr/include`
export CPLUS_INCLUDE_PATH=${BUILDROOT_TARGET_PATH}/include:${BUILDROOT_TARGET_PATH}/usr/include:`readlink -f ./output/include`:`readlink -f ./output/usr/include`
