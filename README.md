
# Save space

* Remove locales other than en_US
* Remove all documentation
* Remove all headers
* Strip all binaries of debugging symbols
* Remove unneeded parts of Xorg (e.g. keyboard layouts)

# Ideas

Try the kdrive from buildroot.
It looks like kindle 5 OS does use the full X server though.

Download pre-compiled anchient arm stuff: http://archive.debian.net/woody/arm/x11/

# X build order

LOOK INTO config.site: https://stackoverflow.com/questions/7561509/how-to-add-include-and-lib-paths-to-configure-make-cycle

I had to manually download ( from http://ftp.x.org/pub/individual/ ):

damageproto-1.2.0
fontsproto-2.1.0
xproto-7.0.17
xtrans-1.2.5
libfontenc-1.0.5
fixesproto-4.1.1
xextproto-7.1.1
libpthread-stubs-0.1
kbproto-1.0.4
inputproto-1.9.99.902
libSM-1.1.1
libICE-1.0.6
renderproto-0.11
xineramaproto-1.2
randrproto-1.3.1
recordproto-1.13.99.1
videoproto-2.3.0
xcmiscproto-1.2.0
scrnsaverproto-1.2.0
bigreqsproto-1.1.0
resourceproto-1.1.0
compositeproto-0.4.1
libpciaccess-0.11.0
xf86-video-fbdev-0.4.1

I downloaded the versions that were released immediately before xserver-1.8.2 (July 1st 2010).

This process worked for a bunch of packages:

```
# run the buildroot_cross_compile_env.sh script
mkdir output
mkdir xdamage-build
cd xdamage-build
../xdamage_1.1.2/./configure --build=${BUILDMACH} --host=${TARGETMACH} --prefix=`readlink -f ../output`
make
make install
```

Host dependencies for building X stuff

```
sudo aptitude install groff xmlto
```

Build env for kindle X stuff:

```
export BUILDROOT_PATH=/home/juul/projects/fread.ink/fread-ink/system/buildroot-2014.02

export BUILDROOT_HOST_PATH=${BUILDROOT_PATH}/output/host
export BUILDROOT_TARGET_PATH=${BUILDROOT_PATH}/output/target
export BUILDROOT_SYSROOT_PATH=${BUILDROOT_HOST_PATH}/usr/arm-buildroot-linux-gnueabi/sysroot
export PKG_CONFIG=${BUILDROOT_HOST_PATH}/usr/bin/pkgconf
export PKG_CONFIG_SYSROOT_DIR=""
export PKG_CONFIG_PATH=`readlink -f output/lib/pkgconfig`:`readlink -f output/share/pkgconfig`
export LD_LIBRARY_PATH=${BUILDROOT_TARGET_PATH}/lib:${BUILDROOT_TARGET_PATH}/usr/lib:`readlink -f ./output/lib`:`readlink -f ./output/usr/lib`
export C_INCLUDE_PATH=${BUILDROOT_TARGET_PATH}/include:${BUILDROOT_TARGET_PATH}/usr/include:`readlink -f ./output/include`:`readlink -f ./output/usr/include`
export CPLUS_INCLUDE_PATH=${BUILDROOT_TARGET_PATH}/include:${BUILDROOT_TARGET_PATH}/usr/include:`readlink -f ./output/include`:`readlink -f ./output/usr/include`
export FREETYPE_LIBS=${BUILDROOT_TARGET_PATH}/usr/lib
export FREETYPE_CFLAGS="-I${BUILDROOT_SYSROOT_PATH}/usr/include -I${BUILDROOT_SYSROOT_PATH}/usr/include/freetype2"
export FONTCONFIG_LIBS=${BUILDROOT_TARGET_PATH}/usr/lib
export FONTCONFIG_CFLAGS="-I${BUILDROOT_SYSROOT_PATH}/usr/include -I${BUILDROOT_SYSROOT_PATH}/usr/include/fontconfig"
```

Add missing .pc files:

```
mkdir -p output/lib/pkgconfig
cp buildroot_missing_dotpc/* output/lib/pkgconfig/
```

Add missing kernel headers:

```
cp -a ../imx_gfx_kernel_headers/linux output/include/
```

My build order for kindle X stuff:

damageproto (make install only)
xproto (make install only)
fixesproto (make install only)
xextproto (make install only)
xtrans (make install only)
xcb-proto
xau
libpthread-stubs
xcb
kbproto (make install only)
inputproto (make install only)
xdmcp
xcb-util
x11
  changed line 25456 of configure script from:
    dir=`pkg-config --variable=includedir xproto`
  to:
    dir=`$PKG_CONFIG --variable=includedir xproto`
  used different configure command:
    ../x11_1.3.2/./configure --build=${BUILDMACH} --host=${TARGETMACH} --prefix=`readlink -f ../output` --enable-malloc0returnsnull
xext
  add configure option: --enable-malloc0returnsnull
libICE
libSM
xt
  add configure option: --enable-malloc0returnsnull
xmu
xpm
xaw7
xfixes
renderproto (make install only)
xrender
  add configure option: --enable-malloc0returnsnull
xdamage
fontsproto (make install only)
libfontenc
freetype # NO! make is use the buildroot freetype 
xfont
xft
xi
  add configure option: --enable-malloc0returnsnull
xineramaproto (make install only)
xinerama
  add configure option: --enable-malloc0returnsnull
xkbfile
xkeyboard-config
randrproto (make install only)
xrandr
  add configure option: --enable-malloc0returnsnull
recordproto (make install only)
xtst
videoproto (make install only)
xv
  add configure option: --enable-malloc0returnsnull
xev
  non-standard make command: make LIBS="-lxcb -lXau"
xkbcomp
  non-standard make command: make LIBS="-lxcb -lXau"
xload
  non-standard make command: make LIBS="-lxcb -lXau -lXext -lXpm"
xorg-macros (make install only)
xclock:
  non-standard make command: make LIBS="-lxcb -lXau -lXext -lXpm -lm"
xcmiscproto (make install only)
scrnsaverproto (make install only)
bigreqsproto (make install only)
resourceproto (make install only)
compositeproto (make install only)
libpciaccess
xserver
  add configure option: 
    --disable-dri --disable-dri2 --disable-glx --disable-aiglx --disable-glx --disable-xinerama
  non-standard make command: make LIBS="-lfreetype -lpthread"

xf86-video-imx
  if problems occur, see these patches:
    https://github.com/Freescale/meta-fsl-arm/tree/master/recipes-graphics/xorg-driver/xf86-video-imxfb
xf86-input-evdev
xf86-video-fbdev

Maybe download the fbdev driver?




multitouch NOT WORKING
  no configure script present
  cross compile will be annoying





Based on modern Xorg build.sh script:

    build proto damageproto
    build proto fixesproto
    build proto fontsproto
    build proto kbproto ?
    build proto randrproto ?
    build proto renderproto ?
    build proto x11proto ?
    build proto xextproto ?
    build proto xineramaproto ?
    build xcb proto ?
    build lib libXau
    build lib libXdmcp

    build xcb libxcb
    build xcb util
    build xcb util-image
    build xcb util-keysyms
    build xcb util-renderutil
    build xcb util-wm
    build lib libX11
    build lib libXext
    build lib libXt
    build lib libXmu
    build lib libXpm
    build lib libXaw
    build lib libXfixes
    build lib libXrender
    build lib libXdamage
    build lib libXfont
    build lib libXft
    build lib libXi
    build lib libXinerama
    build lib libxkbfile
    build lib libXrandr
    build lib libXtst
    build lib libXv


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

