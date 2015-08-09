################################################################################
#
# awesome
#
################################################################################

AWESOME_VERSION = 4933028c5ab7f1c3c5f2c08db7286b185c5d385d
AWESOME_SITE = git://github.com/fread-ink/awesome-3.4.11-kindle.git
AWESOME_LICENSE_FILES = LICENSE
AWESOME_DEPENDENCIES = xserver_xorg-server xproto_xproto cairo pango libxcb xcb-proto xcb-util xcb-util-image xcb-util-keysyms xcb-util-wm imlib2 lua startup-notification libxdg-basedir libev

#AWESOME_CONF_OPTS = "-DCMAKE_TOOLCHAIN_FILE=../../host/usr/share/buildroot/toolchainfile.cmake"

AWESOME_CONF_OPTS = "-DCMAKE_TOOLCHAIN_FILE=$(HOST_DIR)/usr/share/buildroot/toolchainfile.cmake"

#AWESOME_CONF_OPTS = "-DPKG_CONFIG_PATH=/home/juul/data/build/kindle/buildroot-2015.05/output/target/usr/lib/pkgconfig"

#AWESOME_CONF_ENV = "./envs"

$(eval $(cmake-package))
