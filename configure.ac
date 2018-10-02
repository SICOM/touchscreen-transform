dnl
dnl Configure script for the SICOM Systems Inc
dnl COPYRIGHT (C) 2018 SICOM Systems Inc
dnl

AC_INIT([touchscreen-transform],[1.0],[])
AC_DEFINE(VERSION_TITLE,["RELEASE"],[Release Title])
AC_CONFIG_SRCDIR(config.h.in)
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR(.)

dnl init automake
AM_INIT_AUTOMAKE()
AC_CONFIG_HEADERS(config.h)
AM_MAINTAINER_MODE

dnl checks for library functions.
dnl AC_CHECK_FUNCS(fchmod)

dnl check for progs/headers we want
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_PROG_CC_C_O
LT_INIT
LT_OUTPUT

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdio.h stdlib.h string.h malloc.h getopt.h])

AC_CONFIG_FILES([
	Makefile
])
AC_OUTPUT
