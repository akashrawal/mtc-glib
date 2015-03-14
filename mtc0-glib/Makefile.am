lib_LTLIBRARIES = libmtc0-glib.la

libmtc0_glib_la_SOURCES = common.h                      \
                          mtc-glib.h                    \
                          gsource.c      gsource.h      \
                          gsimple.c      gsimple.h      
                          
libmtc0_glib_la_CFLAGS = -Wall $(GLIB_CFLAGS)
libmtc0_glib_la_LIBADD  = $(MTC_LIBS) $(GLIB_LIBS)

mtcincludedir = $(includedir)/mtc0-glib
mtcinclude_HEADERS = common.h \
                     mtc-glib.h \
                     gsource.h  \
                     gsimple.h
