## Process this file with automake to produce Makefile.in

if BUILD_GIM
  bin_PROGRAMS = gim
  gim_SOURCES = gim.c
  gim_CPPFLAGS = -I$(srcdir)/../../include
  gim_LDADD = -L$(builddir)/../../src/.libs -lGI -lGLU -lglut
endif

dist_noinst_DATA = head.ply2
