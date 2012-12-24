AC_INIT([RFSWITCH], [0.1], [wnielson@github], [rfswitch], [http://github.com/wnielson/])
AC_PREREQ([2.58])
AM_INIT_AUTOMAKE([1.09 no-define foreign])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CXX

AC_CHECK_HEADERS([portaudio.h])
AC_CHECK_LIB(portaudio, Pa_Initialize)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
