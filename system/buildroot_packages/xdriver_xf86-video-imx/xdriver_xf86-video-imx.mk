################################################################################
#
# xdriver_xf86-video-imx
#
################################################################################

XDRIVER_XF86_VIDEO_IMX_VERSION = fed2b5c67cf6899ce2332c171083e1d35010a8a7
XDRIVER_XF86_VIDEO_IMX_SITE = git://github.com/fread-ink/xf86-video-imx-fread.git
XDRIVER_XF86_VIDEO_IMX_LICENSE_FILES = COPYING
XDRIVER_XF86_VIDEO_IMX_AUTORECONF = YES

XDRIVER_XF86_VIDEO_IMX_DEPENDENCIES = xserver_xorg-server xproto_fontsproto xproto_randrproto xproto_renderproto xproto_videoproto xproto_xproto linux

# This driver needs access to imx-specific kernel headers
XDRIVER_XF86_VIDEO_IMX_INCLUDE = \
		-I$(LINUX_DIR)/include
XDRIVER_XF86_VIDEO_IMX_CONF_ENV += CFLAGS="$(TARGET_CFLAGS) -I$(LINUX_DIR)/include"

$(eval $(autotools-package))
