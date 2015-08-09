################################################################################
#
# awesome
#
################################################################################

AWESOME_VERSION = ec7cf5bedbccefea629ef300af20b9b164f3526b #
AWESOME_SITE = git://github.com/fread-ink/awesome-3.4.3-kindle.git
AWESOME_LICENSE_FILES = LICENSE
AWESOME_AUTORECONF = YES

AWESOME_DEPENDENCIES = xserver_xorg-server xproto_xproto cairo pango libxcb xcb-proto xcb-util xcb-util-image imlib2

$(eval $(autotools-package))
