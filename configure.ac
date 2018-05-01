#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)

AC_INIT([OpenGI], [2.1.1], [rauy@users.sourceforge.net])
AC_CONFIG_SRCDIR([src/gi_gl.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([-Wall -Werror])

AC_SUBST([LIBRARY_VERSION], [4:1:1])
AC_SUBST([LIBRARY_RELEASE], [2.1.1])

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AM_PROG_LIBTOOL
AM_PROG_CC_C_O

# Checks for libraries.
AC_SEARCH_LIBS([floor], [m])
AC_SEARCH_LIBS([glEnable], [GL], , AC_MSG_ERROR([No libGL found]))
AC_SEARCH_LIBS([clock_gettime], [rt])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([GLU], [gluPerspective], [LIBGLU_PRESENT=1], [LIBGLU_PRESENT=0])
AC_CHECK_LIB([glut], [glutInit], [LIBGLUT_PRESENT=1], [LIBGLUT_PRESENT=0])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([float.h limits.h stddef.h stdlib.h string.h stdio.h stdarg.h errno.h math.h], , 
                 AC_MSG_ERROR([Neccessary C header file missing]))
AC_CHECK_HEADER([GL/gl.h], , AC_MSG_ERROR([No GL/gl.h found]))
AC_CHECK_HEADERS([pthread.h])
AC_CHECK_HEADER([GL/glu.h], [GLU_H_PRESENT=1], [GLU_H_PRESENT=0])
AC_CHECK_HEADER([GL/glut.h], [GLUT_H_PRESENT=1], [GLUT_H_PRESENT=0])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT64_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([floor memset pow sqrt strchr strstr])

# Check if debug output enabled
AC_MSG_CHECKING([verbose feature])
AC_ARG_ENABLE([verbose], [  --enable-verbose        let OpenGI print informative messages during operation], 
              [VERBOSE_GIVEN=1], [VERBOSE_GIVEN=0])
if test $VERBOSE_GIVEN -eq 1 && test $enable_verbose != no;
then
   AC_DEFINE([OPENGI_DEBUG_OUTPUT], [1], [Define to 1 for various informations
             during OpenGI operations.])
   AC_MSG_RESULT([yes])
else
   AC_MSG_RESULT([no])
fi

# Check number of threads.
AC_MSG_CHECKING([maximum number of threads])
AC_ARG_ENABLE([max-threads], [  --enable-max-threads=ARG  set maximum number of threads to ARG], 
              [THREADS_GIVEN=1], [THREADS_GIVEN=0])
if test $THREADS_GIVEN -eq 1;
then
   if test $enable_max_threads != no;
   then
      NUM_THREADS=$enable_max_threads
   else
      NUM_THREADS=1
   fi
else
   NUM_THREADS=`cat /proc/cpuinfo | grep -c "processor"`
   if test $NUM_THREADS -eq 0;
   then
      NUM_THREADS=4
   fi
fi
AC_DEFINE_UNQUOTED([OPENGI_MAX_THREADS], [$NUM_THREADS], 
                   [Define to maximum number of simultaneous threads.])
AC_MSG_RESULT([$NUM_THREADS])

# Check SSE version.
AC_MSG_CHECKING([supported SSE intrustion set])
AC_ARG_ENABLE([sse], [  --enable-sse=ARG        set maximum supported SSE version to ARG (e.g. 2 for SSE2)], 
              [SSE_GIVEN=1], [SSE_GIVEN=0])
if test $SSE_GIVEN -eq 1;
then
   if test $enable_sse != no;
   then
      NUM_SSE=$enable_sse
   else
      NUM_SSE=0
   fi
else
   NUM_PROC=`cat /proc/cpuinfo | grep -c "processor"`
   if test $NUM_PROC -eq 0;
   then
      NUM_SSE=0
   else
      SSE1=`cat /proc/cpuinfo | grep "flags" | grep -ci "\<sse\>"`
      if test $SSE1 -lt $NUM_PROC;
      then
         NUM_SSE=0
      else
         SSE2=`cat /proc/cpuinfo | grep "flags" | grep -ci "\<sse2\>"`
         if test $SSE2 -lt $NUM_PROC;
         then
            NUM_SSE=1
         else
            SSE3=`cat /proc/cpuinfo | grep "flags" | grep -ci "\<pni\>"`
            if test $SSE3 -lt $NUM_PROC;
            then
               NUM_SSE=2
            else
               NUM_SSE=3
            fi
         fi
      fi
   fi
fi
AC_DEFINE_UNQUOTED([OPENGI_SSE], [$NUM_SSE], 
                   [Define to maximum supported SSE version.])
if test $NUM_SSE -ge 3;
then
   SSECFLAGS="-msse3"
else
   if test $NUM_SSE -ge 2;
   then
      SSECFLAGS="-msse2"
   else
      if test $NUM_SSE -ge 1;
      then
         SSECFLAGS="-msse"
      else
         SSECFLAGS=""
      fi
   fi
fi
AC_SUBST([SSECFLAGS])
AC_MSG_RESULT([$NUM_SSE])

# Check for GLU and GLUT.
AM_CONDITIONAL([BUILD_GIM], [test $LIBGLU_PRESENT -eq 1 && test $GLU_H_PRESENT -eq 1 && 
                             test $LIBGLUT_PRESENT -eq 1 && test $GLUT_H_PRESENT -eq 1])
if test $LIBGLU_PRESENT -eq 0 || test $GLU_H_PRESENT -eq 0 || 
   test $LIBGLUT_PRESENT -eq 0 || test $GLUT_H_PRESENT -eq 0;
then
   AC_MSG_WARN([No GLU or GLUT found -> examples/gim will NOT be built])
fi

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 include/Makefile
                 examples/gim/Makefile])
AC_OUTPUT

