#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([MTC GLib],[0.0.0],[],[mtc-glib],[https://github.com/akashrawal/mtc-glib])
AC_CONFIG_SRCDIR([mtc0-glib/common.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror silent-rules])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])
AM_MAINTAINER_MODE
AM_PROG_AR
LT_INIT

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for libraries.
PKG_CHECK_MODULES([MTC], [mtc0 >= 0.0.0])
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.12])

AC_CONFIG_FILES([Makefile
                 data/Makefile
                 data/mtc0-glib.pc
                 mtc0-glib/Makefile
                 doc/Makefile
                 ])
AC_OUTPUT
