lib_LTLIBRARIES = libmtc0-glib.la

mtc_glib_c = gevent.c

mtc_glib_h = common.h mtc-glib.h gevent.h

libmtc0_glib_la_SOURCES = $(mtc_glib_c) $(mtc_glib_h)
                          
libmtc0_glib_la_CFLAGS = -Wall $(GLIB_CFLAGS)
libmtc0_glib_la_LIBADD  = $(MTC_LIBS) $(GLIB_LIBS)

mtcincludedir = $(includedir)/mtc0-glib
mtcinclude_HEADERS = $(mtc_glib_h)
