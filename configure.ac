#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([minibar], [1.0], [eanderton@github.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/utils.cpp])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([crypt], [crypt], [ns1],[
    echo "libcrypt is required"
    exit -1])
AC_CHECK_LIB([dl], [dlopen], [ns1],[
    echo "libdl is required"
    exit -1])
AC_CHECK_LIB([fcgi], [FCGI_Accept], [ns1],[
    echo "libfcgi is required"
    exit -1])
#AC_CHECK_LIB([gtest], [gtest_version_cstr], [ns1],[
#    echo "libgtest is required"
#    exit -1])
AC_CHECK_LIB([pthread], [pthread_create], [ns1],[
    echo "libpthread is required"
    exit -1])
AC_CHECK_LIB([sqlite3], [sqlite3_open], [ns1],[
    echo "libsqlite3 is required"
    exit -1])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h malloc.h stddef.h stdint.h stdlib.h string.h sys/file.h sys/ioctl.h sys/mount.h sys/param.h sys/statvfs.h sys/time.h unistd.h utime.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([clock_gettime fdatasync gettimeofday localtime_r memmove memset munmap strchr strerror strpbrk strstr utime])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
