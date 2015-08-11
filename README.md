
This is a work-in-progress attempt at a free-as-in-freedom linux distribution for electronic paper devices. 

WARNING: This is a big mess right now and graphics are not yet fully working. You should come back later if you're expecting something usable.

This project is based on buildroot and the GPL sources released by Lab126. The operating system is called [fread](http://fread.ink) and while it will run native X apps, most apps will run as full-screen web apps. The focus is on making it super easy to write apps. Performance is less important since the refresh delay of electronic paper hides a lot of the delay. The plan is to have an app and media store called the [free store](freestore.cc) where the app store will simply be a GUI front-end to packages published via npm (and carrying a tag saying that they are fread apps). The media store will be a front-end where people can publish, rate and review any [free culture](http://freedomdefined.org/Definition) media content. The free store will only support ebooks in its first iteration and will simply use archive.org and wikimedia as the storage backend.

I have set myself a very tentative deadline of a 0.1 alpha release for Chaos Communications Congress 2015 but I work on [several](http://peoplesopen.net/) [other](https://sudoroom.org/) [important](http://counterculturelabs.org/) [projects](http://realvegancheese.org/) + I have to actually pay rent somehow, so don't be too surprised if I don't make that deadline. You are of course welcome to help out, but I have limited time for helping new developers get up to speed right now. I hang out in #sudoroom on freenode IRC if you want to reach me (I am juul).

This readme is currently full of a bunch of helpful assorted notes for developers but it is by no means complete.

# Status

The current status of the project is that I can compile and boot a working system and have been able to flip some pixels on the e-ink display by loading the appropriate kernel modules and sending ioctl commands but I do not have real graphics support. Investigating the Lab126 GPL source shows that they, at least on later Kindle devices, are using a full X server with their own fork of the awesome window manager. Their fork has been modified to extend the lua API to allow lua scripts to receive XDamage events with information about which portions of the screen need updating. It can be assumed that lua scripts are then being used to listen for XDamage events and trigger the appropriate screen updates (possibly using ioctl calls through a compiled lua module). These last parts were not included in the GPL code release from Lab126. The major challenges have been getting everything to compile since the code release was partial (even some kernel headers were missing) and many packages were fairly old. Luckily the actual epdc (electronic paper display controller) drivers were merged into buildroot recently so we don't have to maintain our own version of those anymore! Currently everything compiles on latest stable buildroot, including an updated version of the forked awesome. The next step is testing and writing the lua scripts and lua module to enable graphics updating. Then the last major steps before a 0.1 release is getting button input and wifi working (neither should post major challenges).

In the future it would be interesting to look at the modifications Lab126 did to webkit. Their webkit is ancient and their changes seem extensive but I have not looked into them too deeply. Two things would be interesting:

* Giving developers some level of hardware access from the client side javascript (real sockets, battery status, wifi info, etc.)
* Potentially giving developers the option of controlling screen updates on a per-html-element basis such that no updates are performed automatically.

and having an up to date webkit seems important.

# TODO (for the developers)

When running make for buildroot, awesome cmake isn't finding lua. This is despite the fact that the following commands _do_ detect lua:

```
cd output/build/awesome-*
rm CMakeCache.txt
rm .stamp_configured
make clean
cmake -DCMAKE_TOOLCHAIN_FILE=../../host/usr/share/buildroot/toolchainfile.cmake .
```

The minimum kernel version needed for the latest buildroot is 2.6.32 but the kindle kernel we've been using is 2.6.31. Currently I'm trying a dirty hack where I'm just setting the kernel version in buildroot to 2.6.32 but using the 2.6.31 kernel.

I had to drop imx-lib since it was depending on linux kernel headers that were not present. Possibly imx-lib is only for i.mx6?

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

I have not been able to compile a bootable kernel on anything except using a cross compiler on Ubuntu 12.04 using the old emdebian cross compilation toolchain. I have no idea why nothing else works. I have tried a bunch of different toolchains and while they all compile the kernel without error, loading with kexec just results in no output on the serial console and having to hard reboot. I do not know if it could be an incompatibility between kexec and the newer kernels. If anyone has any ideas I am very interested! Until I figure this out I plan to make a virtualbox image available of the Ubuntu setup that successfully compiles the kernel.

To see files I changed just grep for "juul" in the kernel source dir.

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

# Compiling the fread initial ram disk (initrd) 

The initial ram disk is actually just minimal busybox system contained in a .cpio file which bootstraps the file system containing the full system using the init script contained in /init in the cpio filesystem.

For debugging purposes to stop booting after loading the initial ram disk and drop into a shell. The following /init will accomplish this:

```
#!/bin/busybox sh

mount -t proc proc /proc
mount -t sysfs sysfs /sys

exec /bin/sh
```

# Compiling the fread operating system

TODO: The kernel headers used are 2.6.32 but the kindle kernel is 2.6.31 which may cause problems. glibc in the latest buildroot needs at least 2.6.32 so maybe we should patch the kindle kernel to be version 2.6.32? Let's see if it works first.

The fread operating system is compiled using buildroot with the kernel headers from the fread kernel and the X driver added in.

Get the latest buildroot from http://buildroot.uclibc.org/download.html

I used buildroot-2015.05

Use make menuconfig and load the buildroot config from this repo.

## Using custom kernel source

In order to use a custom linux kernel source (instead of downloading the default linux kernel) the following settings were needed:

Added buildroot-2015.05/local.mk with the line:

LINUX_OVERRIDE_SRCDIR = /home/juul/projects/kindle/toolchain/linux-2.6.31

and in menuconfig set:

BR2_PACKAGE_OVERRIDE_FILE="$(TOPDIR)/local.mk"

## Adding the libxdg-basedir package

Copy the awesome buildroot files from this repo:

```
cp -a fread-ink/system/buildroot_packages/libxdg-basedir  buildroot-2015.05/package/
```

Then add the following line to buildroot-2015.05/package/Config.in

```
source "package/libxdg-basedir/Config.in"
```

Add it right after the line:

```
source "package/tzdata/Config.in"
```

Then run "make menuconfig", enable the libxdg-basedir option in "Target packages"-->"Libraries"-->"Other" at the bottom of the list.

Exit menuconfig saving the configuration.

## Adding the awesome window manager

Copy the awesome buildroot files from this repo:

```
cp -a fread-ink/system/buildroot_packages/awesome  buildroot-2015.05/package/
```

Then add the following line to buildroot-2015.05/package/Config.in

```
source "package/awesome/Config.in"
```

Add it right after the line:
    
```
comment "X window managers"
```

Then run "make menuconfig", enable the "awesome" option in "Target packages"--> TODO

Exit menuconfig saving the configuration.



## Upgrading the version of startup-notification

awesome needs startup-notification 0.10 or later but buildroot is using 0.9 so edit the file:

```
buildroot-2015.05/package/startup-notification/startup-notification.mk
```

Setting the version to 0.12 (since 0.10 and 0.11 are broken):

```
STARTUP_NOTIFICATION_VERSION = 0.12
```

## Settting kernel paths

Start menuconfig:

```
make menuconfig
```

Set "Kernel" --> "Kernel version" to "Local directory" then set the option below to the absolute path where you have you compiled kindle kernel. E.g:

```
/home/juul/projects/kindle/toolchain/linux-2.6.31
```

Set "Kernel" --> "Kernel configuration" to "Using custom (def)config file" then set the option below to the absolute path of the kernel config, e.g:


```
/home/juul/projects/fread-ink/kernel/CONFIG
```

## Compiling

Simply run:

```
make
```

## Disabling webkit webgl support

The webgl support is broken and we don't really need it anyway.

Edit the file buildroot-2015.05/package/webkit/webkit.mk and  comment out all lines beginning with WEBKIT_CONF_OPTS or WEBKIT_DEPENDENCIES that occur after the line:

```
ifeq ($(BR2_PACKAGE_HAS_LIBEGL)$(BR2_PACKAGE_HAS_LIBGLES),yy)
```

then right before the line:

```
$(eval $(autotools-package))
```

add this:

```
WEBKIT_CONF_OPTS += --disable-gles2 --disable-egl --disable-glx --with-acceleration-backend=none --disable-webgl --disable-accelerated-compositing
```

## Fixing webkit errors

You may get some errors while compiling webkit that look like this:

```
In file included from ./Source/WTF/wtf/UnusedParam.h:24:0,
                 from Source/JavaScriptCore/API/tests/JSNode.c:33:
./Source/WTF/wtf/Platform.h:301:1: error: expected identifier or ‘(’ before ‘/’ token
 // All NEON intrinsics usage can be disabled by this macro.
```

Simply edit the Platform.h file changing the problematic lines from comments like this:

```
// All NEON intrinsics usage can be disabled by this macro.
```

To comments like this:

```
/* All NEON intrinsics usage can be disabled by this macro. */
```

There should be two lines in Platform.h with the wrong "//" comments.

Then re-run make to resume compilation.

If you get errors related to GL then edit the file:

```
/buildroot-2015.05/output/build/webkit-1.11.5/Source/WebCore/platform/graphics/egl/GLContextEGL.cpp
```

Changing the line near the top from:

```
#if USE(EGL)
```

to:

```
#if 0
```

and re-run make to resume compilation.

## TODO

need to create correct versions of:

buildroot-2015.05/system/device_table*.txt


# Hardware info

The Kindles use the Freescale i.MX series of low-power system-on-chip micropocessors. The i.MX processors used in the Kindle 4th generation and later includes an ElectroPhoretic Display Controller (EPDC).

First, second and third generation Kindles used the i.MX31 and i.MX35 processors while later versions used the i.MX50 which includes the EPDC. The paperwhite uses the i.MX 6SoloLite which has a higher performance EPDC (presumably to allow for higher resolutions).

Hardware for various Kindle versions:

* Original: Marvell Xscale PXA255 400 MHz, 64 MB RAM, 256 MB HD,
* Kindle 2: Freescale i.MX31 532 MHz, 32 MB RAM, 2 GB HD,
* Kindle 3: Freescale i.MX35 532 MHz, 128 MB RAM, 4 GB HD
* Kindle 4, 5, Touch, Paperwhite (1st gen): Freescale i.MX50 800 MHz, 256 MB RAM, 2 GB HD (4 GB for touch)
* Kindle Paperwhite (2nd gen): Freescale i.MX50 (really?) 1 GHz, 256 MB RAM, 2 GB HD
* Kindle Voyage: Freescale i.MX6, 512 MB RAM, 4GB HD

https://en.wikipedia.org/wiki/I.MX

http://cache.freescale.com/files/industrial/doc/white_paper/KNDLPWCS.pdf

https://community.freescale.com/docs/DOC-93622

## Kindle 4 NT

# Non-Kindle devices

## Nook Simple Touch (and Glowlight)

* Processor: 800 MHz TI OMAP 3621
* RAM: 256 MB
* HD: 2 GB + microSD
* OS: Android 2.1


# Buildroot tips

## Directory layout

* package/ - Buildroot makefiles and config for packages
* dl/ where packages are downloaded
* output/build/ - Where source code for packages is extracted and built
* output/target/ - Where packages are first installed
* output/images/ - Completed built images

## How to re-build, re-configure, re-download single packages

To force a re-build of a package cd into the directory of the package and run make clean, delete the .stamp_built file (if it exists) and then re-run make:

```
cd output/build/awesome-4933028c5ab7f1c3c5f2c08db7286b185c5d385d/
make clean
rm .stamp_built
cd ../../../
make
```

To re-configure a package, do the same but delete both .stamp_built and .stamp_configured

```
cd output/build/awesome-4933028c5ab7f1c3c5f2c08db7286b185c5d385d/
make clean
rm .stamp_configured
rm .stamp_built
cd ../../../
make
```
 
If you changed something in the package but want to re-configure and re-build from the original unmodified package then just delete the package directory:

```
rm -rf output/build/awesome-4933028c5ab7f1c3c5f2c08db7286b185c5d385d/
make
```

If you want to re-download (because you changed the buildroot makefile for the package to point to a new source file):

```
rm -rf output/build/awesome-4933028c5ab7f1c3c5f2c08db7286b185c5d385d/
rm dl/awesome-4933028c5ab7f1c3c5f2c08db7286b185c5d385d.tar.gz
make
```

Also look at the buildroot documentation section [Understanding when a full rebuild is necessary](http://buildroot.uclibc.org/downloads/manual/manual.html#full-rebuild).

# Jailbreaking

## Jailbreaking Kindle Voyage

You need a serial console for this jailbreak. On the kindle, go to settings, then go to "about kindle" to get your kindle's serial number. Use [NiLuJe's version of KindleTool](https://github.com/NiLuJe/KindleTool/) to calculate the recovery root password. The normal root password won't work on the kindle voyage. 

Download KindleTool:

```
git clone https://github.com/NiLuJe/KindleTool.git
```

To compile KindleTool first install dependencies:

```
sudo aptitude install libgmp-dev libgmp3-dev nettle-dev build-essential
```

Then compile:

```
cd KindleTool
make
```

Run KindleTool:

```
./KindleTool/Release/kindletool CAFEC0DEF00D00

```

Save the diagnostics password. The other password won't work.

On the serial console, hit enter during the uboot prompt at bootup. Then run the printenv command and look for a line like:

```
bootcmd_diags=bootm 0xE41000
```

Run the part after the '=' as a command:

```
bootm 0xE41000
```

Wait for the kindle to boot into diagnostics mode, then touch the "Exit or reboot" option, and then the "exit to login shell" option. Wait for a few seconds, then hit enter on the serial console and you should get a login shell.

Now log in as root using the diagnostics password you got from KindleTool.

Mount the first root partition:

```
mkdir /tmp/root
mount /dev/mmcblk0p1 /tmp/root
```

On your own computer, generate a password hash:

```
mkpasswd --method=md5 mypassword
```

On the kindle, edit the shadow file:

```
vi /tmp/root/etc/shadow
```

Replace exclamation mark on the first line:

```
root:!:10933:0:99999:7:::
```

With the output from mkpasswd:

```
root:$1$Fmgi9feV$J268xLEjfPm6w6puduyNh/:10933:0:99999:7:::
```

Save the file, then unmount the root partition and reboot:

```
umount /tmp/root
reboot
```

Now you should be able to log in with the root password you gave to mkpasswd.


# Kindle workings

## How the eips utility clears the screen:

```
open("/dev/fb0", O_RDWR)                = 3
ioctl(3, FBIOGET_VSCREENINFO, 0xbe9c5b48) = 0
mmap2(NULL, 480000, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_LOCKED, 3, 0) = 0x40127000
ioctl(3, 0x46e1, 0)                     = 0
close(3)                                = 0
```

It opens the framebuffer in read-write mode, maps it to memory, then calls the ioctl FBIO_EINK_CLEAR_SCREEN. Not sure why the memory mapping is needed, maybe it just always does that and it's not actually needed for the clear.

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

Even omitting the waveform_to_use parameter still results in a working system.

The broads (broadsheet) and fslepdc (FreeScaLe Electronic Paper Display Controller) referred to in various places are different e-ink controllers, with the emu option being emulation (for when no e-ink controller/display is available). This is because some of the early kindles had a separate e-ink controller, the Epson Broadsheet, while newer kindles use versions of the Freescale i.mx SoC containing an integrated EPDC.

# Cross compiling manually with buildroot

After compiling buildroot the cross compile environment will be in:

```
buildroot_dir/output/host/
```


# Cross compiling programs

Note that your cross compiler will need to be using the same version of libc (and any other libraries) as is present on the kindle so the process outlined here won't really work for running stuff on the original kindle OS :(

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
  
Looking at these in a hex editor they look very odd. Possibly they are eips waveforms or monochrome images with an alpha channel (they seem to be composed mostly of the values 0, 1 and 2).
