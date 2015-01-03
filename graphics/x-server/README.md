
# Cross-compiling X

This requires that buildroot is compiled.

Install host dependencies for building X stuff:

```
sudo aptitude install groff xmlto
```

Set the basic cross-compile environment variables:

```
cd ../../system
. buildroot_cross_compile_env.sh
cd ../graphics/x-server
```

Set some more env vars for X. Edit BUILDROOT_PATH in build_env_extra.sh to the absolute path to your buildroot-2014.02 dir, then run:

```
. build_env_extra.sh
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

Create output directory:

```
mkdir output
```

Now compile everything. This process worked for all packages unless otherwise noted:

```
mkdir -p build/xdamage-build
cd build/xdamage-build
../../packages/xdamage_1.1.2/configure --build=${BUILDMACH} --host=${TARGETMACH} --prefix=`readlink -f ../../output`
make
make install
```

Here's the build order and notes about non-standard configure and make options:

```
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
  add configure option: --enable-malloc0returnsnull
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
  add configure options: 
    --disable-dri --disable-dri2 --disable-glx --disable-aiglx --disable-glx --disable-xinerama
  non-standard make command: make LIBS="-lfreetype -lpthread"
xf86-video-imx
  if problems occur, see these patches:
    https://github.com/Freescale/meta-fsl-arm/tree/master/recipes-graphics/xorg-driver/xf86-video-imxfb
xf86-input-evdev
xf86-video-fbdev
```

Multitouch cross-compile is currently not working: No configure script present. Cross compile will be annoying.

# ToDo

## Save space

* Remove locales other than en_US
* Remove all documentation
* Remove all headers
* Strip all binaries of debugging symbols
* Remove unneeded parts of Xorg
** Keyboard layouts
** Probably lots of unneeded libraries like /lib/libX*

## Ideas

Try the kdrive from buildroot.

Download pre-compiled anchient arm stuff: http://archive.debian.net/woody/arm/x11/