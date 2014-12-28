

# GOT HERE

An error occurs when running "make" 

  ~/data/build/cross-compiler-mine/gcc-obj


For more info read:

~/data/build/cross-compiler-mine/gcc-obj/arm-linux-gnueabihf/libgcc/config.log

Especially:

  xgcc: error: unrecognized option '-V'
  xgcc: fatal error: no input files

  xgcc: error: unrecognized option '-qversion'
  xgcc: fatal error: no input files

  conftest.c:1:0: sorry, unimplemented: -mfloat-abi=hard and VFP

# Getting a working cross-compiler

The only cross-compiler capable of generating a bootable kindle kernel is from Ub

## From old emdebian sources 

Here's how to get the only pre-compiled cross-compiler capable of generating a bootable kernel for Kindle (that I have found). Other cross-compilers generate a kernels that don't work (no idea why, I get no output after attempting to boot them with kexec).

First install Ubuntu 12.04 32 bit (e.g. in a virtual machine). Then add the following two lines to /etc/apt/sources.list:

```
deb http://ftp.uk.debian.org/emdebian/toolchains testing main
deb http://ftp.uk.debian.org/emdebian/toolchains unstable main
```

Now run:

```
sudo apt-get update
sudo apt-get install binutils-arm-linux-gnueabihf cpp-4.6-arm-linux-gnueabihf gcc-4.6-arm-linux-gnueabihf cpio uboot-mkimage
cd /usr/bin
sudo ln -s arm-linux-gnueabihf-gcc-4.6 arm-linux-gnueabihf-gcc
sudo ln -s arm-linux-gnueabihf-cpp-4.6 arm-linux-gnueabihf-cpp
sudo ln -s arm-linux-gnueabihf-gcov-4.6 arm-linux-gnueabihf-gcov
```

Now you simply run the build.sh script to build the kernel.

## Building your own kernel

Links: 

* http://cowlark.com/2009-07-04-building-gcc/
* http://www.linux.com/community/blogs/127-personal/468920-cross-compiling-for-arm
* http://www.emdebian.org/tools/crossdev.html

First get some sources:

*[binutils 2.22](http://mirror.clarkson.edu/gnu/binutils/binutils-2.22.tar.bz2)
*[gcc 4.6.3](http://mirrors.kernel.org/gnu/gcc/gcc-4.6.3/gcc-4.6.3.tar.bz2)

Make a directory for building binutils, then configure and build:

```

```

# Configure flags from working cross-compiling gcc

here is its configure flags:

./arm-linux-gnueabihf-gcc-4.6 -v
Using built-in specs.
COLLECT_GCC=./arm-linux-gnueabihf-gcc-4.6
COLLECT_LTO_WRAPPER=/usr/lib/gcc/arm-linux-gnueabihf/4.6/lto-wrapper
Target: arm-linux-gnueabihf
Configured with: ../src/configure -v --with-pkgversion='Debian 4.6.3-15' --with-bugurl=file:///usr/share/doc/gcc-4.6/README.Bugs --enable-languages=c,c++,fortran,objc,obj-c++ --prefix=/usr --program-suffix=-4.6 --enable-shared --enable-linker-build-id --with-system-zlib --libexecdir=/usr/lib --without-included-gettext --enable-threads=posix --with-gxx-include-dir=/usr/arm-linux-gnueabihf/include/c++/4.6.3 --libdir=/usr/lib --enable-nls --enable-clocale=gnu --enable-libstdcxx-debug --enable-libstdcxx-time=yes --enable-gnu-unique-object --enable-plugin --enable-objc-gc --disable-sjlj-exceptions --with-arch=armv7-a --with-fpu=vfpv3-d16 --with-float=hard --with-mode=thumb --enable-checking=release --build=i486-linux-gnu --host=i486-linux-gnu --target=arm-linux-gnueabihf --program-prefix=arm-linux-gnueabihf- --includedir=/usr/arm-linux-gnueabihf/include --with-headers=/usr/arm-linux-gnueabihf/include --with-libs=/usr/arm-linux-gnueabihf/lib
Thread model: posix
gcc version 4.6.3 (Debian 4.6.3-15)


My conf command:

../gcc-4.6.3/configure --enable-languages=c,c++,fortran,objc,obj-c++ --prefix=/usr/local --program-suffix=-4.6 --enable-shared --enable-linker-build-id --with-system-zlib --libexecdir=/usr/local/lib --without-included-gettext --enable-threads=posix --with-gxx-include-dir=/usr/local/arm-linux-gnueabihf/include/c++/4.6.3 --libdir=/usr/local/lib --enable-nls --enable-clocale=gnu --enable-libstdcxx-debug --enable-libstdcxx-time=yes --enable-gnu-unique-object --enable-plugin --enable-objc-gc --disable-sjlj-exceptions --with-arch=armv7-a --with-fpu=vfpv3-d16 --with-float=hard --with-mode=thumb --enable-checking=release --target=arm-linux-gnueabihf --program-prefix=arm-linux-gnueabihf- --includedir=/usr/local/arm-linux-gnueabihf/include --with-headers=/usr/local/arm-linux-gnueabihf/include --with-libs=/usr/local/arm-linux-gnueabihf/lib

Take two:

../gcc-4.6.3/configure --enable-languages=c,c++,fortran,objc,obj-c++ --prefix=$TOOLCHAINPATH --program-suffix=-4.6 --enable-shared --enable-linker-build-id --with-system-zlib --libexecdir=${TOOLCHAINPATH}/lib --without-included-gettext --enable-threads=posix --with-gxx-include-dir=${TOOLCHAINPATH}/arm-linux-gnueabihf/include/c++/4.6.3 --libdir=${TOOLCHAINPATH}/lib --enable-nls --enable-clocale=gnu --enable-libstdcxx-debug --enable-libstdcxx-time=yes --enable-gnu-unique-object --enable-plugin --enable-objc-gc --disable-sjlj-exceptions --with-arch=armv7-a --with-fpu=vfpv3-d16 --with-float=hard --with-mode=thumb --enable-checking=release --target=arm-linux-gnueabihf --program-prefix=arm-linux-gnueabihf- --includedir=${TOOLCHAINPATH}/arm-linux-gnueabihf/include --with-headers=${TOOLCHAINPATH}/arm-linux-gnueabihf/include --with-libs=${TOOLCHAINPATH}/arm-linux-gnueabihf/lib 

Take three:

Based on this guide http://preshing.com/20141119/how-to-build-a-gcc-cross-compiler/ 
but used the gcc ./contrib/download_prerequisites instead of manually downloading

Used gcc 4.6.3

export TOOLCHAINPATH=/home/juul/data/build/cross-compiler-mine/toolchain
export PATH=${TOOLCHAINPATH}/bin:${TOOLCHAINPATH}/usr/bin:$PATH

cd /home/juul/projects/kindle/toolchain/linux-2.6.31
make clean
make ARCH=arm INSTALL_HDR_PATH=${TOOLCHAINPATH}/arm-linux-gnueabihf headers_install


../gcc-4.9.2/configure --enable-languages=c,c++,fortran,objc,obj-c++ --prefix=$TOOLCHAINPATH --program-suffix=-4.9 --enable-shared --enable-linker-build-id --with-system-zlib --without-included-gettext --enable-threads=posix --enable-nls --enable-clocale=gnu --enable-libstdcxx-debug --enable-libstdcxx-time=yes --enable-gnu-unique-object --enable-plugin --enable-objc-gc --disable-sjlj-exceptions --with-arch=armv7-a --with-fpu=vfpv3-d16 --with-float=hard --with-mode=thumb --enable-checking=release --target=arm-linux-gnueabihf --program-prefix=arm-linux-gnueabihf- --host=x86_64-pc-linux-gnu --build=x86_64-pc-linux-gnu

Take four (no threads):

../gcc-4.6.3/configure --enable-languages=c,c++,fortran,objc,obj-c++ --prefix=$TOOLCHAINPATH --program-suffix=-4.6 --enable-shared --enable-linker-build-id --with-system-zlib --without-included-gettext --enable-nls --enable-clocale=gnu --enable-libs tdcxx-debug --enable-libstdcxx-time=yes --enable-gnu-unique-object --enable-plugin --enable-objc-gc --disable-sjlj-exceptions --with-arch=armv7-a --with-fpu=vfpv3-d16 --with-float=hard --with-mode=thumb --enable-checking=release --target=arm-linux-gnueabihf --program-prefix=arm-linux-gnueabihf- --host=x86_64-pc-linux-gnu --build=x86_64-pc-linux-gnu

Take five (with threads, no thumb (if this doesn't compile a working kernel, then try without hardfloat but with thumb)):

../gcc-4.6.3/configure --enable-languages=c,c++,fortran,objc,obj-c++ --prefix=$TOOLCHAINPATH --program-suffix=-4.6 --enable-shared --enable-linker-build-id --with-system-zlib --without-included-gettext --enable-threads=posix --enable-nls --enable-clocale=gnu --enable-libs tdcxx-debug --enable-libstdcxx-time=yes --enable-gnu-unique-object --enable-plugin --enable-objc-gc --disable-sjlj-exceptions --with-arch=armv7-a --with-fpu=vfpv3-d16 --with-float=hard --enable-checking=release --target=arm-linux-gnueabihf --program-prefix=arm-linux-gnueabihf- --host=x86_64-pc-linux-gnu --build=x86_64-pc-linux-gnu

Take six (neon instead of vfpv3-d16):

../gcc-4.6.3/configure --enable-languages=c,c++,fortran,objc,obj-c++ --prefix=$TOOLCHAINPATH --program-suffix=-4.6 --enable-shared --enable-linker-build-id --with-system-zlib --without-included-gettext --enable-threads=posix --enable-nls --enable-clocale=gnu --enable-libs tdcxx-debug --enable-libstdcxx-time=yes --enable-gnu-unique-object --enable-plugin --enable-objc-gc --disable-sjlj-exceptions --with-arch=armv7-a --with-fpu=neon --with-float=hard --with-mode=thumb --enable-checking=release --target=arm-linux-gnueabihf --program-prefix=arm-linux-gnueabihf- --host=x86_64-pc-linux-gnu --build=x86_64-pc-linux-gnu

Take seven (softfloat) THIS ONE WORKS but unknown if it compiles a working kernel:

../gcc-4.6.3/configure --enable-languages=c,c++,fortran,objc,obj-c++ --prefix=$TOOLCHAINPATH --program-suffix=-4.6 --enable-shared --enable-linker-build-id --with-system-zlib --without-included-gettext --enable-threads=posix --enable-nls --enable-clocale=gnu --enable-libs tdcxx-debug --enable-libstdcxx-time=yes --enable-gnu-unique-object --enable-plugin --enable-objc-gc --disable-sjlj-exceptions --with-arch=armv7-a --with-fpu=vfpv3-d16 --with-float=soft --with-mode=thumb --enable-checking=release --target=arm-linux-gnueabi --program-prefix=arm-linux-gnueabi- --host=x86_64-pc-linux-gnu --build=x86_64-pc-linux-gnu

Take eight (no --with-fpu) UNTESTED:

../gcc-4.6.3/configure --enable-languages=c,c++,fortran,objc,obj-c++ --prefix=$TOOLCHAINPATH --program-suffix=-4.6 --enable-shared --enable-linker-build-id --with-system-zlib --without-included-gettext --enable-threads=posix --enable-nls --enable-clocale=gnu --enable-libs tdcxx-debug --enable-libstdcxx-time=yes --enable-gnu-unique-object --enable-plugin --enable-objc-gc --disable-sjlj-exceptions --with-arch=armv7-a --with-float=hard --with-mode=thumb --enable-checking=release --target=arm-linux-gnueabihf --program-prefix=arm-linux-gnueabihf- --host=x86_64-pc-linux-gnu --build=x86_64-pc-linux-gnu

---------

export TOOLCHAINPATH=/home/juul/data/build/cross-compiler-mine/toolchain
export PATH=${TOOLCHAINPATH}/bin:${TOOLCHAINPATH}/usr/bin:$PATH

cd /home/juul/projects/kindle/toolchain/linux-2.6.31
make clean
make ARCH=arm INSTALL_HDR_PATH=${TOOLCHAINPATH}/arm-linux-gnueabi headers_install

../binutils-2.22/configure --target=arm-linux-gnueabi --prefix=/home/juul/data/build/cross-compiler-mine/toolchain --disable-multilib

make CFLAGS="-w" -j4
make install

../gcc-4.6.3/configure --build=x86_64-pc-linux-gnu --host=x86_64-pc-linux-gnu --target=arm-linux-gnueabi --enable-languages=c,c++ --prefix=$TOOLCHAINPATH --program-suffix=-4.6 --enable-shared --enable-linker-build-id --with-system-zlib --without-included-gettext --enable-threads=posix --enable-nls --enable-clocale=gnu --enable-libs tdcxx-debug --enable-libstdcxx-time=yes --enable-gnu-unique-object --enable-plugin --enable-objc-gc --disable-sjlj-exceptions --with-arch=armv7-a --with-fpu=neon --with-tune=cortex-a8 --with-float=softfp --with-mode=thumb --disable-multilib --enable-checking=release --program-prefix=arm-linux-gnueabi- 

make CFLAGS="-w" -j4 all-gcc
make install-gcc

../glibc-2.18/configure --host=arm-linux-gnueabi --build=$MACHTYPE --target=arm-linux-gnueabi --prefix=/home/juul/data/build/cross-compiler-mine/toolchain --with-headers=${TOOLCHAINPATH}/arm-linux-gnueabi/include --disable-multilib lib_cv_forced_unwind=yes


--------

make CFLAGS="-w" all-gcc
make install-gcc

The correct glibc version to use is 2.18

../glibc-2.18/configure --host=arm-linux-gnueabihf --build=$MACHTYPE --target=arm-linux-gnueabihf --prefix=/home/juul/data/build/cross-compiler-mine/toolchain --with-headers=${TOOLCHAINPATH}/arm-linux-gnueabihf/include libc_cv_forced_unwind=yes

make install-bootstrap-headers=yes install-headers
make csu/subdir_lib
install csu/crt1.o csu/crti.o csu/crtn.o ${TOOLCHAINPATH}/arm-linux-gnueabihf/lib


= Other notes =

Notes on CFLAGS for Kindle 4 (search for "full set of used CFLAGS"):

http://www.mobileread.com/forums/archive/index.php/t-170213.html

Long version:

```
CFLAGS="-mlittle-endian -march=armv7-a -mcpu=cortex-a8 -mtune=cortex-a8 -mfpu=neon -mfloat-abi=softfp"
```

Short version:

```
CFLAGS="-mfpu=neon -mfloat-abi=softfp"
```

You can find this info by copying a binary from the kindle to your computer and doing:

```
readelf -A eips
```