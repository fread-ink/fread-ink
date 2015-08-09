################################################################################
#
# libxdg-basedir
#
################################################################################

LIBXDG_BASEDIR_VERSION = 8aa82b1d5cf4458df2866b015eb9ff885dcd3905 # 1.2.0
LIBXDG_BASEDIR_SITE = git://github.com/devnev/libxdg-basedir.git
LIBXDG_BASEDIR_LICENSE_FILES = COPYING
LIBXDG_BASEDIR_AUTORECONF = YES

LIBXDG_BASEDIR_INSTALL_STAGING = YES

$(eval $(autotools-package))
