# Notes about open source

There are some binary blobs in the original linux-2.6.31 amazon release in:

```
linux-2.6.31/firmware
```

A few of these end up in:

```
linux-2.6.31/tar-install/lib/firmware/imx
```

and becomes part of the final linux-2.6.31-rt11-lab126.tar.gz file, and I assume also in the uImage (should check).
  
Looking at these in a hex editor they look very odd. Possibly they are monochrome images (they seem to be composed mostly of the values 0, 1 and 2).

# Pre-requisites

You need some basics:

```
sudo aptitude install build-essential git bison
```

and some cross-compilation tools:

```
sudo aptitude install gcc-arm-linux-gnueabihf cpp-arm-linux-gnueabihf g++-arm-linux-gnueabihf binutils-arm-linux-gnueabihf pkg-config-arm-linux-gnueabihf
```

# Compiling the fread kernel

To see changed files, grep for "juul" in this dir.

First run:

  sudo aptitude install u-boot-tools

If that doesn't work try this instead:

  sudo aptitude install uboot-mkimage

Then run:

  ./build.sh

## kernel parameters

The kernel parameters are set in the .config option "CONFIG_CMDLINE". Note that kexec will ignore any and all kernel parameters. Kernel parameters must be hard-coded using CONFIG_CMDLINE in the kernel .config file.

consoleblank=0 rootwait ro ip=off root=/dev/ram0 debug eink=fslepdc video=mxcepdcfb:E60,bpp=8 console=ttymxc0,115200

Setting init= in the command line does nothing for the initrd filesystem. The file "/init" is always executed as the first and only file.

For explanation of /dev/ram0 see: https://www.kernel.org/doc/Documentation/initrd.txt 

# Using kexec to boot a kernel from usb storage

Passing the exact contents of /proc/cmdline into command-line-options is the safest way to ensure that correct values are passed to the rebooting kernel.

kexec4 \
 -l uImage \
 --type=uImage 
kexec4 -e

# Compiling the fread intiial ram disk (initrd) 

The initial ram disk is actually just minimal busybox system contained in a .cpio file which bootstraps the file system containing the full system using the init script contained in /init in the cpio filesystem.

For debugging purposes to stop booting after loading the initial ram disk and drop into a shell. The following /init will accomplish this:

```
#!/bin/busybox sh

mount -t proc proc /proc
mount -t sysfs sysfs /sys

exec /bin/sh
```

# Compiling the fread operating system

The fread operating system is compiled using buildroot with the kernel headers from the fread kernel and the X driver added in.

## Using custom kernel source

In order to use a custom linux kernel source (instead of downloading the default linux kernel) the following settings were needed:

Added buildroot-2014.02/local.mk with the line:

LINUX_OVERRIDE_SRCDIR = /home/juul/projects/kindle/toolchain/linux-2.6.31

and in menuconfig set:

BR2_PACKAGE_OVERRIDE_FILE="$(TOPDIR)/local.mk"

## Adding the kindle X driver

To package/x11r7/Config.in added the line:

	source package/x11r7/xdriver_xf86-video-imx/Config.in

Added the dir and contents:

  package/x11r7/xdriver_xf86-video-imx

Which cause the kindle X driver to be pulled from juul's github.

## TODO

need to create correct versions of:

buildroot-2014.02/system/device_table*.txt


# Hardware info

The Kindles use the Freescale i.MX series of low-power system-on-chip micropocessors. The i.MX processors used in the Kindle 4th generation and later includes an ElectroPhoretic Display Controller (EPDC).

First, second and third generation Kindles used the i.MX31 and i.MX35 processors while later versions used the i.MX50 which includes the EPDC. The paperwhite uses the i.MX 6SoloLite which has a higher performance EPDC (presumably to allow for higher resolutions).

Hardware for various Kindle versions:

*Original: Marvell Xscale PXA255 400 MHz, 64 MB RAM, 256 MB HD,
*Kindle 2: Freescale i.MX31 532 MHz, 32 MB RAM, 2 GB HD,
*Kindle 3: Freescale i.MX35 532 MHz, 128 MB RAM, 4 GB HD
*Kindle 4, 5, Touch, Paperwhite (1st gen): Freescale i.MX50 800 MHz, 256 MB RAM, 2 GB HD (4 GB for touch)
*Kindle Paperwhite (2nd gen): Freescale i.MX50 1 GHz, 256 MB RAM, 2 GB HD

https://en.wikipedia.org/wiki/I.MX

http://cache.freescale.com/files/industrial/doc/white_paper/KNDLPWCS.pdf

https://community.freescale.com/docs/DOC-93622

# Non-Kindle devices

## Nook Simple Touch (and Glowlight)

*Processor: 800 MHz TI OMAP 3621
*RAM: 256 MB
*HD: 2 GB + microSD
*OS: Android 2.1

