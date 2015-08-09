################################################################################
#
# libxdg-basedir
#
################################################################################

LIBXDG_BASEDIR_VERSION = fed2b5c67cf6899ce2332c171083e1d35010a8a7 # 1.2.0
LIBXDG_BASEDIR_SITE = git://github.com/devnev/libxdg-basedir.git
LIBXDG_BASEDIR_LICENSE_FILES = COPYING
LIBXDG_BASEDIR_AUTORECONF = YES

#LIBXDG_BASEDIR_DEPENDENCIES = 

$(eval $(autotools-package))
