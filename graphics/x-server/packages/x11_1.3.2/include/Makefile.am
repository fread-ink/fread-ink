x11includedir=${includedir}/X11
x11include_HEADERS=\
	X11/XKBlib.h \
	X11/Xcms.h \
	X11/Xlib.h \
	X11/Xlibint.h \
	X11/Xlocale.h \
	X11/Xresource.h \
	X11/Xutil.h \
	X11/cursorfont.h \
	X11/Xregion.h \
	X11/ImUtil.h

nodist_x11include_HEADERS=\
	X11/XlibConf.h

EXTRA_DIST = \
	X11/XlibConf.h.in

if XCB
x11include_HEADERS += X11/Xlib-xcb.h
endif
