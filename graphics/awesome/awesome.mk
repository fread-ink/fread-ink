################################################################################
#
# awesome
#
################################################################################

AWESOME_VERSION = fe4bcab927e306bd95699ce6fdda29865ca28612
AWESOME_SITE = git://github.com/fread-ink/awesome-3.4.11-kindle.git
AWESOME_LICENSE_FILES = LICENSE
AWESOME_DEPENDENCIES = xserver_xorg-server xproto_xproto cairo pango libxcb xcb-proto xcb-util xcb-util-image xcb-util-keysyms xcb-util-wm imlib2 lua startup-notification libxdg-basedir libev

AWESOME_CONF_OPTS = -DCMAKE_TOOLCHAIN_FILE=$(HOST_DIR)/usr/share/buildroot/toolchainfile.cmake 

# Enable Lab126 specific code
AWESOME_CONF_OPTS += -DCMAKE_C_FLAGS='-DLAB126 ${CMAKE_C_FLAGS}'

$(eval $(cmake-package))
