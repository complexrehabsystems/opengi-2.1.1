## Process this file with automake to produce Makefile.in

SUBDIRS = src include examples/gim
EXTRA_DIST = bin lib projects

doc_DATA = AUTHORS README NEWS ChangeLog
dist_doc_DATA = doc/OpenGI-manual.pdf
dist_noinst_DATA = config_win32.exe Doxyfile doc/logo.png

dist-hook:
	rm -rf $(top_distdir)/bin/*
	rm -rf $(top_distdir)/bin/.svn
	rm -rf $(top_distdir)/lib/*
	rm -rf $(top_distdir)/lib/.svn
	rm -rf $(top_distdir)/projects/.svn
	rm -f $(top_distdir)/projects/vs*/*.user
	rm -f $(top_distdir)/projects/vs*/*.suo
	rm -f $(top_distdir)/projects/vs*/*.ncb
	rm -rf $(top_distdir)/projects/vs*/Debug
	rm -rf $(top_distdir)/projects/vs*/Release
	rm -rf $(top_distdir)/projects/vs*/.svn

