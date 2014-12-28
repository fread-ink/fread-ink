
# Kindle workings

## How the eips utility clears the screen:

```
open("/dev/fb0", O_RDWR)                = 3
ioctl(3, FBIOGET_VSCREENINFO, 0xbe9c5b48) = 0
mmap2(NULL, 480000, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_LOCKED, 3, 0) = 0x40127000
ioctl(3, 0x46e1, 0)                     = 0
close(3)                                = 0
```

It open the framebuffer in read-write mode, maps it to memory, then calls the ioctl FBIO_EINK_CLEAR_SCREEN. Not sure why the memory mapping is needed, maybe it just always does that and it's not actually needed for the clear.

## Which modules the kindle loads for eink

Here is the reverse module load order from the stop_eink() function from /etc/init.d/video

```    
rmmod eink_fb_shim 
rmmod eink_fb_hal_broads 
rmmod eink_fb_hal_fslepdc
rmmod eink_fb_hal_emu
rmmod mxc_epdc_fb
rmmod eink_fb_hal
rmmod eink_fb_waveform
rmmod emucolor_fb
```         

The module loading proceeds through load_eink_fb_1st() then load_controller() then load_eink_fb_2nd():

```
modprobe eink_fb_waveform
modprobe eink_fb_hal
modprobe mxc_epdc_fb dont_register_fb=1
modprobe eink_fb_hal_fslepdc bootstrap=0 waveform_to_use=/path/to/06_05_00dc_3c_156231_05_58_000010a2_85_07.wbf
modprobe eink_fb_shim
```

Runnning the above on the original kindle system after running:

```
/etc/init.d/video stop_all
```

results in a working display, which can be tested with:

```
eips lollerskater
```

Even omititng the waveform_to_use parameter still results in a working system.

It seems like the broads (broadsheet) and fslepdc (FreeScaLe Electronic Paper Display Controller) referred to in /etc/init.d/display are almost certainly different e-ink controllers, with the emu option being emulation (for when no e-ink controller/display is available).

# Cross compiling manually with buildroot

After compiling buildroot the cross compile environment will be in:

```
buildroot_dir/output/host/
```




# Cross compiling programs

Note that your cross compiler will need to be using the same version of libc (and any other libraries) as is present on the kindle so the process outlined here won't really work :(

Here is an example of how you can cross-compile code for you kindle. 

sudo aptitude install gcc-arm-linux-gnueabi g++-arm-linux-gnueabi  cpp-arm-linux-gnueabi binutils-arm-linux-gnueabi pkg-config-arm-linux-gnueabi

Here we are cross compiling strace, so better download and extract it:

```
wget http://downloads.sourceforge.net/project/strace/strace/4.9/strace-4.9.tar.xz
tar xvJf strace-4.9.tar.xz
```

Make some dirs we'll need for building
mkdir strace-build
mkdir strace-final
```

Now set some environment variables using the script included with fread (this script assumes your system is 64 bit x86):

```
. utilities/cross_compile_env.sh
```

Configure build and install strace:

```
cd strace-build
../strace-4.9/configure --prefix=/home/juul/projects/fread.ink/strace-final --host=$TARGETMACH
make
make install
```

You will find the resulting binaries in strace-final/bin and you can simply copy the strace binary to the kindle.

# kexec

A binary kexec for kindle 4 is included in kernel/kexec_kindle_4 and I recommend that you use it. Compiling a kexec that will work on kindle 4 is annoying since it requires a cross-compilation toolchain with an old libc and old kernel headers.

# Investigating the original kindle system

## ldd

There is no ldd on the kindle. You can instead use this script utilities/fake_ldd.sh on your host system:

## strace

A version of strace compiled for kindle 4 is available here utilities/strace_kindle_4

## strings

The strings utility is already present on the kindle.

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

## Kindle 4 NT

# Non-Kindle devices

## Nook Simple Touch (and Glowlight)

*Processor: 800 MHz TI OMAP 3621
*RAM: 256 MB
*HD: 2 GB + microSD
*OS: Android 2.1

