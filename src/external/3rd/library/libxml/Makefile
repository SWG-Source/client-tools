# Makefile.in generated automatically by automake 1.4-p6 from Makefile.am

# Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

# AUTOMAKE_ OPTIONS=no-dependencies


SHELL = /bin/sh

srcdir = .
top_srcdir = .

prefix = /usr/local
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
sbindir = ${exec_prefix}/sbin
libexecdir = ${exec_prefix}/libexec
datadir = ${prefix}/share
sysconfdir = ${prefix}/etc
sharedstatedir = ${prefix}/com
localstatedir = ${prefix}/var
libdir = ${exec_prefix}/lib
infodir = ${prefix}/info
mandir = ${prefix}/man
includedir = ${prefix}/include
oldincludedir = /usr/include

DESTDIR =

pkgdatadir = $(datadir)/libxml2
pkglibdir = $(libdir)/libxml2
pkgincludedir = $(includedir)/libxml2

top_builddir = .

ACLOCAL = aclocal-1.4
AUTOCONF = autoconf
AUTOMAKE = automake-1.4
AUTOHEADER = autoheader

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL} $(AM_INSTALL_PROGRAM_FLAGS)
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_SCRIPT = ${INSTALL}
transform = s,x,x,

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :
host_alias = 
host_triplet = i686-pc-linux-gnu
AS = @AS@
C14N_OBJ = c14n.c
CATALOG_OBJ = catalog.o
CC = gcc
CFLAGS = -g -O2 -Wall
CPP = gcc -E
DEBUG_OBJ = debugXML.o
DLLTOOL = @DLLTOOL@
DOCB_OBJ = DOCBparser.o
ECHO = echo
EXEEXT = 
FTP_OBJ = nanoftp.o
HAVE_ISINF = 
HAVE_ISNAN = 
HTML_DIR = $(prefix)/doc
HTML_OBJ = HTMLparser.o HTMLtree.o
HTTP_OBJ = nanohttp.o
ICONV_LIBS = 
LIBTOOL = $(SHELL) $(top_builddir)/libtool
LIBXML_MAJOR_VERSION = 2
LIBXML_MICRO_VERSION = 28
LIBXML_MINOR_VERSION = 4
LIBXML_VERSION = 2.4.28
LIBXML_VERSION_INFO = 6:28:4
LIBXML_VERSION_NUMBER = 20428
LN_S = ln -s
MAINT = #
MAKEINFO = makeinfo
MV = /bin/mv
M_LIBS = -lm
OBJDUMP = @OBJDUMP@
OBJEXT = o
PACKAGE = libxml2
PYTHON = /usr/bin/python
PYTHON_INCLUDES = /usr/include/python1.5
PYTHON_SITE_PACKAGES = /usr/lib/python1.5/site-packages
PYTHON_SUBDIR = python
PYTHON_VERSION = 1.5
RANLIB = ranlib
RDL_LIBS = 
RM = /bin/rm
STRIP = strip
TAR = /bin/tar
TEST_REGEXPS = Regexptests Automatatests
TEST_SCHEMAS = 
TEST_THREADS = 
THREAD_CFLAGS = 
THREAD_LIBS = 
U = 
VERSION = 2.4.28
WITH_C14N = 1
WITH_CATALOG = 1
WITH_DEBUG = 1
WITH_DOCB = 1
WITH_FTP = 1
WITH_HTML = 1
WITH_HTTP = 1
WITH_ICONV = 1
WITH_MEM_DEBUG = 0
WITH_REGEXPS = 1
WITH_SCHEMAS = 0
WITH_THREADS = 0
WITH_TRIO = 0
WITH_XINCLUDE = 1
WITH_XPATH = 1
WITH_XPTR = 1
XINCLUDE_OBJ = xinclude.o
XML_CFLAGS = 
XML_INCLUDEDIR = -I${includedir}/libxml2
XML_LIBDIR = -L${libdir}
XML_LIBS = -lxml2 -lz   -lm 
XPATH_OBJ = xpath.o
XPTR_OBJ = xpointer.o
Z_CFLAGS = 
Z_LIBS = -lz

SUBDIRS = include . doc example python

DIST_SUBDIRS = include . doc example python

INCLUDES = -I$(top_builddir)/include -I./include   

noinst_PROGRAMS = testSchemas testSAX testHTML testXPath testURI testDocbook \
                testThreads testC14N testAutomata testRegexp


bin_PROGRAMS = xmllint xmlcatalog

bin_SCRIPTS = xml2-config

lib_LTLIBRARIES = libxml2.la
libxml2_la_LIBADD =  -lz $(ICONV_LIBS) -lm

libxml2_la_LDFLAGS = -version-info 6:28:4
#libxml2_la_SOURCES = #SAX.c entities.c encoding.c error.c parserInternals.c  \
#		parser.c tree.c hash.c list.c xmlIO.c xmlmemory.c uri.c  \
#		valid.c xlink.c HTMLparser.c HTMLtree.c debugXML.c xpath.c  \
#		xpointer.c xinclude.c nanohttp.c nanoftp.c DOCBparser.c \
#		catalog.c globals.c threads.c c14n.c \
#		xmlregexp.c xmlschemas.c xmlschemastypes.c xmlunicode.c \
#		triostr.c trio.c
libxml2_la_SOURCES = SAX.c entities.c encoding.c error.c parserInternals.c  \
		parser.c tree.c hash.c list.c xmlIO.c xmlmemory.c uri.c  \
		valid.c xlink.c HTMLparser.c HTMLtree.c debugXML.c xpath.c  \
		xpointer.c xinclude.c nanohttp.c nanoftp.c DOCBparser.c \
		catalog.c globals.c threads.c c14n.c \
		xmlregexp.c xmlschemas.c xmlschemastypes.c xmlunicode.c

DEPS = $(top_builddir)/libxml2.la
LDADDS = $(top_builddir)/libxml2.la  -lz $(ICONV_LIBS) -lm

man_MANS = xml2-config.1 libxml.3

m4datadir = $(datadir)/aclocal
m4data_DATA = libxml.m4

xmllint_SOURCES = xmllint.c
xmllint_LDFLAGS = 
xmllint_DEPENDENCIES = $(DEPS)
xmllint_LDADD =  $(LDADDS)

testSAX_SOURCES = testSAX.c
testSAX_LDFLAGS = 
testSAX_DEPENDENCIES = $(DEPS)
testSAX_LDADD = $(LDADDS)

testHTML_SOURCES = testHTML.c
testHTML_LDFLAGS = 
testHTML_DEPENDENCIES = $(DEPS)
testHTML_LDADD = $(LDADDS)

testDocbook_SOURCES = testDocbook.c
testDocbook_LDFLAGS = 
testDocbook_DEPENDENCIES = $(DEPS)
testDocbook_LDADD = $(LDADDS)

xmlcatalog_SOURCES = xmlcatalog.c
xmlcatalog_LDFLAGS = 
xmlcatalog_DEPENDENCIES = $(DEPS)
xmlcatalog_LDADD =  $(LDADDS)

testXPath_SOURCES = testXPath.c
testXPath_LDFLAGS = 
testXPath_DEPENDENCIES = $(DEPS)
testXPath_LDADD = $(LDADDS)

testC14N_SOURCES = testC14N.c
testC14N_LDFLAGS = 
testC14N_DEPENDENCIES = $(DEPS)
testC14N_LDADD = $(LDADDS)

testThreads_SOURCES = testThreads.c
testThreads_LDFLAGS = 
testThreads_DEPENDENCIES = $(DEPS)
testThreads_LDADD = $(LDADDS)

testURI_SOURCES = testURI.c
testURI_LDFLAGS = 
testURI_DEPENDENCIES = $(DEPS)
testURI_LDADD = $(LDADDS)

testRegexp_SOURCES = testRegexp.c
testRegexp_LDFLAGS = 
testRegexp_DEPENDENCIES = $(DEPS)
testRegexp_LDADD = $(LDADDS)

testAutomata_SOURCES = testAutomata.c
testAutomata_LDFLAGS = 
testAutomata_DEPENDENCIES = $(DEPS)
testAutomata_LDADD = $(LDADDS)

testSchemas_SOURCES = testSchemas.c
testSchemas_LDFLAGS = 
testSchemas_DEPENDENCIES = $(DEPS)
testSchemas_LDADD = $(LDADDS)

CLEANFILES = xml2Conf.sh

confexecdir = $(libdir)
confexec_DATA = xml2Conf.sh
EXTRA_DIST = xml2-config.in xml2Conf.sh.in libxml.spec.in libxml.spec \
             libxml.m4 Copyright check-xml-test-suite.py \
             example/Makefile.am example/gjobread.c example/gjobs.xml \
	     $(man_MANS) libxml-2.0.pc.in \
	     trionan.c trionan.h triostr.c triostr.h trio.c trio.h \
	     triop.h triodef.h libxml.h \
	     testThreadsWin32.c genUnicode.py TODO_SCHEMAS


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libxml-2.0.pc

#
# Instll the tests program sources as examples 
#
BASE_DIR = $(datadir)/doc
DOC_MODULE = libxml2-$(VERSION)
EXAMPLES_DIR = $(BASE_DIR)/$(DOC_MODULE)/examples
ACLOCAL_M4 = $(top_srcdir)/aclocal.m4
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = config.h
CONFIG_CLEAN_FILES =  libxml.spec xml2-config libxml-2.0.pc xml2Conf.sh
LTLIBRARIES =  $(lib_LTLIBRARIES)


DEFS = -DHAVE_CONFIG_H -I. -I$(srcdir) -I.
CPPFLAGS = 
LDFLAGS = 
LIBS = 
libxml2_la_DEPENDENCIES = 
libxml2_la_OBJECTS =  SAX.lo entities.lo \
encoding.lo error.lo parserInternals.lo \
parser.lo tree.lo hash.lo list.lo xmlIO.lo \
xmlmemory.lo uri.lo valid.lo xlink.lo \
HTMLparser.lo HTMLtree.lo debugXML.lo xpath.lo \
xpointer.lo xinclude.lo nanohttp.lo nanoftp.lo \
DOCBparser.lo catalog.lo globals.lo threads.lo \
c14n.lo xmlregexp.lo xmlschemas.lo \
xmlschemastypes.lo xmlunicode.lo
#libxml2_la_OBJECTS =  SAX.lo entities.lo \
#encoding.lo error.lo parserInternals.lo \
#parser.lo tree.lo hash.lo list.lo xmlIO.lo \
#xmlmemory.lo uri.lo valid.lo xlink.lo \
#HTMLparser.lo HTMLtree.lo debugXML.lo xpath.lo \
#xpointer.lo xinclude.lo nanohttp.lo nanoftp.lo \
#DOCBparser.lo catalog.lo globals.lo threads.lo \
#c14n.lo xmlregexp.lo xmlschemas.lo \
#xmlschemastypes.lo xmlunicode.lo triostr.lo \
#trio.lo
bin_PROGRAMS =  xmllint$(EXEEXT) xmlcatalog$(EXEEXT)
noinst_PROGRAMS =  testSchemas$(EXEEXT) testSAX$(EXEEXT) \
testHTML$(EXEEXT) testXPath$(EXEEXT) testURI$(EXEEXT) \
testDocbook$(EXEEXT) testThreads$(EXEEXT) testC14N$(EXEEXT) \
testAutomata$(EXEEXT) testRegexp$(EXEEXT)
PROGRAMS =  $(bin_PROGRAMS) $(noinst_PROGRAMS)

xmllint_OBJECTS =  xmllint.$(OBJEXT)
xmlcatalog_OBJECTS =  xmlcatalog.$(OBJEXT)
testSchemas_OBJECTS =  testSchemas.$(OBJEXT)
testSAX_OBJECTS =  testSAX.$(OBJEXT)
testHTML_OBJECTS =  testHTML.$(OBJEXT)
testXPath_OBJECTS =  testXPath.$(OBJEXT)
testURI_OBJECTS =  testURI.$(OBJEXT)
testDocbook_OBJECTS =  testDocbook.$(OBJEXT)
testThreads_OBJECTS =  testThreads.$(OBJEXT)
testC14N_OBJECTS =  testC14N.$(OBJEXT)
testAutomata_OBJECTS =  testAutomata.$(OBJEXT)
testRegexp_OBJECTS =  testRegexp.$(OBJEXT)
SCRIPTS =  $(bin_SCRIPTS)

COMPILE = $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
LTCOMPILE = $(LIBTOOL) --mode=compile $(CC) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS)
CCLD = $(CC)
LINK = $(LIBTOOL) --mode=link $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@
man1dir = $(mandir)/man1
man3dir = $(mandir)/man3
MANS = $(man_MANS)

NROFF = nroff
DATA =  $(confexec_DATA) $(m4data_DATA) $(pkgconfig_DATA)

DIST_COMMON =  README ./stamp-h.in AUTHORS COPYING ChangeLog INSTALL \
Makefile.am Makefile.in NEWS TODO acconfig.h acinclude.m4 aclocal.m4 \
config.guess config.h.in config.sub configure configure.in install-sh \
libxml-2.0.pc.in libxml.spec.in ltconfig ltmain.sh missing \
mkinstalldirs xml2-config.in xml2Conf.sh.in


DISTFILES = $(DIST_COMMON) $(SOURCES) $(HEADERS) $(TEXINFOS) $(EXTRA_DIST)

GZIP_ENV = --best
SOURCES = $(libxml2_la_SOURCES) $(xmllint_SOURCES) $(xmlcatalog_SOURCES) $(testSchemas_SOURCES) $(testSAX_SOURCES) $(testHTML_SOURCES) $(testXPath_SOURCES) $(testURI_SOURCES) $(testDocbook_SOURCES) $(testThreads_SOURCES) $(testC14N_SOURCES) $(testAutomata_SOURCES) $(testRegexp_SOURCES)
OBJECTS = $(libxml2_la_OBJECTS) $(xmllint_OBJECTS) $(xmlcatalog_OBJECTS) $(testSchemas_OBJECTS) $(testSAX_OBJECTS) $(testHTML_OBJECTS) $(testXPath_OBJECTS) $(testURI_OBJECTS) $(testDocbook_OBJECTS) $(testThreads_OBJECTS) $(testC14N_OBJECTS) $(testAutomata_OBJECTS) $(testRegexp_OBJECTS)

all: all-redirect
.SUFFIXES:
.SUFFIXES: .S .c .lo .o .obj .s
$(srcdir)/Makefile.in: # Makefile.am $(top_srcdir)/configure.in $(ACLOCAL_M4) 
	cd $(top_srcdir) && $(AUTOMAKE) --gnu --include-deps Makefile

Makefile: $(srcdir)/Makefile.in  $(top_builddir)/config.status
	cd $(top_builddir) \
	  && CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

$(ACLOCAL_M4): # configure.in  acinclude.m4
	cd $(srcdir) && $(ACLOCAL)

config.status: $(srcdir)/configure $(CONFIG_STATUS_DEPENDENCIES)
	$(SHELL) ./config.status --recheck
$(srcdir)/configure: #$(srcdir)/configure.in $(ACLOCAL_M4) $(CONFIGURE_DEPENDENCIES)
	cd $(srcdir) && $(AUTOCONF)

config.h: stamp-h
	@if test ! -f $@; then \
		rm -f stamp-h; \
		$(MAKE) stamp-h; \
	else :; fi
stamp-h: $(srcdir)/config.h.in $(top_builddir)/config.status
	cd $(top_builddir) \
	  && CONFIG_FILES= CONFIG_HEADERS=config.h \
	     $(SHELL) ./config.status
	@echo timestamp > stamp-h 2> /dev/null
$(srcdir)/config.h.in: #$(srcdir)/stamp-h.in
	@if test ! -f $@; then \
		rm -f $(srcdir)/stamp-h.in; \
		$(MAKE) $(srcdir)/stamp-h.in; \
	else :; fi
$(srcdir)/stamp-h.in: $(top_srcdir)/configure.in $(ACLOCAL_M4) acconfig.h
	cd $(top_srcdir) && $(AUTOHEADER)
	@echo timestamp > $(srcdir)/stamp-h.in 2> /dev/null

mostlyclean-hdr:

clean-hdr:

distclean-hdr:
	-rm -f config.h

maintainer-clean-hdr:
libxml.spec: $(top_builddir)/config.status libxml.spec.in
	cd $(top_builddir) && CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status
xml2-config: $(top_builddir)/config.status xml2-config.in
	cd $(top_builddir) && CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status
libxml-2.0.pc: $(top_builddir)/config.status libxml-2.0.pc.in
	cd $(top_builddir) && CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status
xml2Conf.sh: $(top_builddir)/config.status xml2Conf.sh.in
	cd $(top_builddir) && CONFIG_FILES=$@ CONFIG_HEADERS= $(SHELL) ./config.status

mostlyclean-libLTLIBRARIES:

clean-libLTLIBRARIES:
	-test -z "$(lib_LTLIBRARIES)" || rm -f $(lib_LTLIBRARIES)

distclean-libLTLIBRARIES:

maintainer-clean-libLTLIBRARIES:

install-libLTLIBRARIES: $(lib_LTLIBRARIES)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(libdir)
	@list='$(lib_LTLIBRARIES)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo "$(LIBTOOL)  --mode=install $(INSTALL) $$p $(DESTDIR)$(libdir)/$$p"; \
	    $(LIBTOOL)  --mode=install $(INSTALL) $$p $(DESTDIR)$(libdir)/$$p; \
	  else :; fi; \
	done

uninstall-libLTLIBRARIES:
	@$(NORMAL_UNINSTALL)
	list='$(lib_LTLIBRARIES)'; for p in $$list; do \
	  $(LIBTOOL)  --mode=uninstall rm -f $(DESTDIR)$(libdir)/$$p; \
	done

.c.o:
	$(COMPILE) -c $<

# FIXME: We should only use cygpath when building on Windows,
# and only if it is available.
.c.obj:
	$(COMPILE) -c `cygpath -w $<`

.s.o:
	$(COMPILE) -c $<

.S.o:
	$(COMPILE) -c $<

mostlyclean-compile:
	-rm -f *.o core *.core
	-rm -f *.$(OBJEXT)

clean-compile:

distclean-compile:
	-rm -f *.tab.c

maintainer-clean-compile:

.c.lo:
	$(LIBTOOL) --mode=compile $(COMPILE) -c $<

.s.lo:
	$(LIBTOOL) --mode=compile $(COMPILE) -c $<

.S.lo:
	$(LIBTOOL) --mode=compile $(COMPILE) -c $<

mostlyclean-libtool:
	-rm -f *.lo

clean-libtool:
	-rm -rf .libs _libs

distclean-libtool:

maintainer-clean-libtool:

libxml2.la: $(libxml2_la_OBJECTS) $(libxml2_la_DEPENDENCIES)
	$(LINK) -rpath $(libdir) $(libxml2_la_LDFLAGS) $(libxml2_la_OBJECTS) $(libxml2_la_LIBADD) $(LIBS)

mostlyclean-binPROGRAMS:

clean-binPROGRAMS:
	-test -z "$(bin_PROGRAMS)" || rm -f $(bin_PROGRAMS)

distclean-binPROGRAMS:

maintainer-clean-binPROGRAMS:

install-binPROGRAMS: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@list='$(bin_PROGRAMS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo " $(LIBTOOL)  --mode=install $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`"; \
	    $(LIBTOOL)  --mode=install $(INSTALL_PROGRAM) $$p $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	  else :; fi; \
	done

uninstall-binPROGRAMS:
	@$(NORMAL_UNINSTALL)
	list='$(bin_PROGRAMS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(bindir)/`echo $$p|sed 's/$(EXEEXT)$$//'|sed '$(transform)'|sed 's/$$/$(EXEEXT)/'`; \
	done

mostlyclean-noinstPROGRAMS:

clean-noinstPROGRAMS:
	-test -z "$(noinst_PROGRAMS)" || rm -f $(noinst_PROGRAMS)

distclean-noinstPROGRAMS:

maintainer-clean-noinstPROGRAMS:

xmllint$(EXEEXT): $(xmllint_OBJECTS) $(xmllint_DEPENDENCIES)
	@rm -f xmllint$(EXEEXT)
	$(LINK) $(xmllint_LDFLAGS) $(xmllint_OBJECTS) $(xmllint_LDADD) $(LIBS)

xmlcatalog$(EXEEXT): $(xmlcatalog_OBJECTS) $(xmlcatalog_DEPENDENCIES)
	@rm -f xmlcatalog$(EXEEXT)
	$(LINK) $(xmlcatalog_LDFLAGS) $(xmlcatalog_OBJECTS) $(xmlcatalog_LDADD) $(LIBS)

testSchemas$(EXEEXT): $(testSchemas_OBJECTS) $(testSchemas_DEPENDENCIES)
	@rm -f testSchemas$(EXEEXT)
	$(LINK) $(testSchemas_LDFLAGS) $(testSchemas_OBJECTS) $(testSchemas_LDADD) $(LIBS)

testSAX$(EXEEXT): $(testSAX_OBJECTS) $(testSAX_DEPENDENCIES)
	@rm -f testSAX$(EXEEXT)
	$(LINK) $(testSAX_LDFLAGS) $(testSAX_OBJECTS) $(testSAX_LDADD) $(LIBS)

testHTML$(EXEEXT): $(testHTML_OBJECTS) $(testHTML_DEPENDENCIES)
	@rm -f testHTML$(EXEEXT)
	$(LINK) $(testHTML_LDFLAGS) $(testHTML_OBJECTS) $(testHTML_LDADD) $(LIBS)

testXPath$(EXEEXT): $(testXPath_OBJECTS) $(testXPath_DEPENDENCIES)
	@rm -f testXPath$(EXEEXT)
	$(LINK) $(testXPath_LDFLAGS) $(testXPath_OBJECTS) $(testXPath_LDADD) $(LIBS)

testURI$(EXEEXT): $(testURI_OBJECTS) $(testURI_DEPENDENCIES)
	@rm -f testURI$(EXEEXT)
	$(LINK) $(testURI_LDFLAGS) $(testURI_OBJECTS) $(testURI_LDADD) $(LIBS)

testDocbook$(EXEEXT): $(testDocbook_OBJECTS) $(testDocbook_DEPENDENCIES)
	@rm -f testDocbook$(EXEEXT)
	$(LINK) $(testDocbook_LDFLAGS) $(testDocbook_OBJECTS) $(testDocbook_LDADD) $(LIBS)

testThreads$(EXEEXT): $(testThreads_OBJECTS) $(testThreads_DEPENDENCIES)
	@rm -f testThreads$(EXEEXT)
	$(LINK) $(testThreads_LDFLAGS) $(testThreads_OBJECTS) $(testThreads_LDADD) $(LIBS)

testC14N$(EXEEXT): $(testC14N_OBJECTS) $(testC14N_DEPENDENCIES)
	@rm -f testC14N$(EXEEXT)
	$(LINK) $(testC14N_LDFLAGS) $(testC14N_OBJECTS) $(testC14N_LDADD) $(LIBS)

testAutomata$(EXEEXT): $(testAutomata_OBJECTS) $(testAutomata_DEPENDENCIES)
	@rm -f testAutomata$(EXEEXT)
	$(LINK) $(testAutomata_LDFLAGS) $(testAutomata_OBJECTS) $(testAutomata_LDADD) $(LIBS)

testRegexp$(EXEEXT): $(testRegexp_OBJECTS) $(testRegexp_DEPENDENCIES)
	@rm -f testRegexp$(EXEEXT)
	$(LINK) $(testRegexp_LDFLAGS) $(testRegexp_OBJECTS) $(testRegexp_LDADD) $(LIBS)

install-binSCRIPTS: $(bin_SCRIPTS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@list='$(bin_SCRIPTS)'; for p in $$list; do \
	  if test -f $$p; then \
	    echo " $(INSTALL_SCRIPT) $$p $(DESTDIR)$(bindir)/`echo $$p|sed '$(transform)'`"; \
	    $(INSTALL_SCRIPT) $$p $(DESTDIR)$(bindir)/`echo $$p|sed '$(transform)'`; \
	  else if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_SCRIPT) $(srcdir)/$$p $(DESTDIR)$(bindir)/`echo $$p|sed '$(transform)'`"; \
	    $(INSTALL_SCRIPT) $(srcdir)/$$p $(DESTDIR)$(bindir)/`echo $$p|sed '$(transform)'`; \
	  else :; fi; fi; \
	done

uninstall-binSCRIPTS:
	@$(NORMAL_UNINSTALL)
	list='$(bin_SCRIPTS)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(bindir)/`echo $$p|sed '$(transform)'`; \
	done

install-man1:
	$(mkinstalldirs) $(DESTDIR)$(man1dir)
	@list='$(man1_MANS)'; \
	l2='$(man_MANS)'; for i in $$l2; do \
	  case "$$i" in \
	    *.1*) list="$$list $$i" ;; \
	  esac; \
	done; \
	for i in $$list; do \
	  if test -f $(srcdir)/$$i; then file=$(srcdir)/$$i; \
	  else file=$$i; fi; \
	  ext=`echo $$i | sed -e 's/^.*\\.//'`; \
	  inst=`echo $$i | sed -e 's/\\.[0-9a-z]*$$//'`; \
	  inst=`echo $$inst | sed '$(transform)'`.$$ext; \
	  echo " $(INSTALL_DATA) $$file $(DESTDIR)$(man1dir)/$$inst"; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(man1dir)/$$inst; \
	done

uninstall-man1:
	@list='$(man1_MANS)'; \
	l2='$(man_MANS)'; for i in $$l2; do \
	  case "$$i" in \
	    *.1*) list="$$list $$i" ;; \
	  esac; \
	done; \
	for i in $$list; do \
	  ext=`echo $$i | sed -e 's/^.*\\.//'`; \
	  inst=`echo $$i | sed -e 's/\\.[0-9a-z]*$$//'`; \
	  inst=`echo $$inst | sed '$(transform)'`.$$ext; \
	  echo " rm -f $(DESTDIR)$(man1dir)/$$inst"; \
	  rm -f $(DESTDIR)$(man1dir)/$$inst; \
	done

install-man3:
	$(mkinstalldirs) $(DESTDIR)$(man3dir)
	@list='$(man3_MANS)'; \
	l2='$(man_MANS)'; for i in $$l2; do \
	  case "$$i" in \
	    *.3*) list="$$list $$i" ;; \
	  esac; \
	done; \
	for i in $$list; do \
	  if test -f $(srcdir)/$$i; then file=$(srcdir)/$$i; \
	  else file=$$i; fi; \
	  ext=`echo $$i | sed -e 's/^.*\\.//'`; \
	  inst=`echo $$i | sed -e 's/\\.[0-9a-z]*$$//'`; \
	  inst=`echo $$inst | sed '$(transform)'`.$$ext; \
	  echo " $(INSTALL_DATA) $$file $(DESTDIR)$(man3dir)/$$inst"; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(man3dir)/$$inst; \
	done

uninstall-man3:
	@list='$(man3_MANS)'; \
	l2='$(man_MANS)'; for i in $$l2; do \
	  case "$$i" in \
	    *.3*) list="$$list $$i" ;; \
	  esac; \
	done; \
	for i in $$list; do \
	  ext=`echo $$i | sed -e 's/^.*\\.//'`; \
	  inst=`echo $$i | sed -e 's/\\.[0-9a-z]*$$//'`; \
	  inst=`echo $$inst | sed '$(transform)'`.$$ext; \
	  echo " rm -f $(DESTDIR)$(man3dir)/$$inst"; \
	  rm -f $(DESTDIR)$(man3dir)/$$inst; \
	done
install-man: $(MANS)
	@$(NORMAL_INSTALL)
	$(MAKE) $(AM_MAKEFLAGS) install-man1 install-man3
uninstall-man:
	@$(NORMAL_UNINSTALL)
	$(MAKE) $(AM_MAKEFLAGS) uninstall-man1 uninstall-man3

install-confexecDATA: $(confexec_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(confexecdir)
	@list='$(confexec_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(confexecdir)/$$p"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(confexecdir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(DESTDIR)$(confexecdir)/$$p"; \
	    $(INSTALL_DATA) $$p $(DESTDIR)$(confexecdir)/$$p; \
	  fi; fi; \
	done

uninstall-confexecDATA:
	@$(NORMAL_UNINSTALL)
	list='$(confexec_DATA)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(confexecdir)/$$p; \
	done

install-m4dataDATA: $(m4data_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(m4datadir)
	@list='$(m4data_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(m4datadir)/$$p"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(m4datadir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(DESTDIR)$(m4datadir)/$$p"; \
	    $(INSTALL_DATA) $$p $(DESTDIR)$(m4datadir)/$$p; \
	  fi; fi; \
	done

uninstall-m4dataDATA:
	@$(NORMAL_UNINSTALL)
	list='$(m4data_DATA)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(m4datadir)/$$p; \
	done

install-pkgconfigDATA: $(pkgconfig_DATA)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(pkgconfigdir)
	@list='$(pkgconfig_DATA)'; for p in $$list; do \
	  if test -f $(srcdir)/$$p; then \
	    echo " $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(pkgconfigdir)/$$p"; \
	    $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(pkgconfigdir)/$$p; \
	  else if test -f $$p; then \
	    echo " $(INSTALL_DATA) $$p $(DESTDIR)$(pkgconfigdir)/$$p"; \
	    $(INSTALL_DATA) $$p $(DESTDIR)$(pkgconfigdir)/$$p; \
	  fi; fi; \
	done

uninstall-pkgconfigDATA:
	@$(NORMAL_UNINSTALL)
	list='$(pkgconfig_DATA)'; for p in $$list; do \
	  rm -f $(DESTDIR)$(pkgconfigdir)/$$p; \
	done

# This directory's subdirectories are mostly independent; you can cd
# into them and run `make' without going through this Makefile.
# To change the values of `make' variables: instead of editing Makefiles,
# (1) if the variable is set in `config.status', edit `config.status'
#     (which will cause the Makefiles to be regenerated when you run `make');
# (2) otherwise, pass the desired values on the `make' command line.



all-recursive install-data-recursive install-exec-recursive \
installdirs-recursive install-recursive uninstall-recursive  \
check-recursive installcheck-recursive info-recursive dvi-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	dot_seen=no; \
	target=`echo $@ | sed s/-recursive//`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    dot_seen=yes; \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done; \
	if test "$$dot_seen" = "no"; then \
	  $(MAKE) $(AM_MAKEFLAGS) "$$target-am" || exit 1; \
	fi; test -z "$$fail"

mostlyclean-recursive clean-recursive distclean-recursive \
maintainer-clean-recursive:
	@set fnord $(MAKEFLAGS); amf=$$2; \
	dot_seen=no; \
	rev=''; list='$(SUBDIRS)'; for subdir in $$list; do \
	  rev="$$subdir $$rev"; \
	  test "$$subdir" != "." || dot_seen=yes; \
	done; \
	test "$$dot_seen" = "no" && rev=". $$rev"; \
	target=`echo $@ | sed s/-recursive//`; \
	for subdir in $$rev; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done && test -z "$$fail"
tags-recursive:
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  test "$$subdir" = . || (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) tags); \
	done

tags: TAGS

ID: $(HEADERS) $(SOURCES) $(LISP)
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	here=`pwd` && cd $(srcdir) \
	  && mkid -f$$here/ID $$unique $(LISP)

TAGS: tags-recursive $(HEADERS) $(SOURCES) config.h.in $(TAGS_DEPENDENCIES) $(LISP)
	tags=; \
	here=`pwd`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
   if test "$$subdir" = .; then :; else \
	    test -f $$subdir/TAGS && tags="$$tags -i $$here/$$subdir/TAGS"; \
   fi; \
	done; \
	list='$(SOURCES) $(HEADERS)'; \
	unique=`for i in $$list; do echo $$i; done | \
	  awk '    { files[$$0] = 1; } \
	       END { for (i in files) print i; }'`; \
	test -z "$(ETAGS_ARGS)config.h.in$$unique$(LISP)$$tags" \
	  || (cd $(srcdir) && etags $(ETAGS_ARGS) $$tags config.h.in $$unique $(LISP) -o $$here/TAGS)

mostlyclean-tags:

clean-tags:

distclean-tags:
	-rm -f TAGS ID

maintainer-clean-tags:

distdir = $(PACKAGE)-$(VERSION)
top_distdir = $(distdir)

# This target untars the dist file and tries a VPATH configuration.  Then
# it guarantees that the distribution is self-contained by making another
# tarfile.
distcheck: dist
	-rm -rf $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) zxf $(distdir).tar.gz
	mkdir $(distdir)/=build
	mkdir $(distdir)/=inst
	dc_install_base=`cd $(distdir)/=inst && pwd`; \
	cd $(distdir)/=build \
	  && ../configure --srcdir=.. --prefix=$$dc_install_base \
	  && $(MAKE) $(AM_MAKEFLAGS) \
	  && $(MAKE) $(AM_MAKEFLAGS) dvi \
	  && $(MAKE) $(AM_MAKEFLAGS) check \
	  && $(MAKE) $(AM_MAKEFLAGS) install \
	  && $(MAKE) $(AM_MAKEFLAGS) installcheck \
	  && $(MAKE) $(AM_MAKEFLAGS) dist
	-rm -rf $(distdir)
	@banner="$(distdir).tar.gz is ready for distribution"; \
	dashes=`echo "$$banner" | sed s/./=/g`; \
	echo "$$dashes"; \
	echo "$$banner"; \
	echo "$$dashes"
dist: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	-rm -rf $(distdir)
dist-all: distdir
	-chmod -R a+r $(distdir)
	GZIP=$(GZIP_ENV) $(TAR) chozf $(distdir).tar.gz $(distdir)
	-rm -rf $(distdir)
distdir: $(DISTFILES)
	-rm -rf $(distdir)
	mkdir $(distdir)
	-chmod 777 $(distdir)
	$(mkinstalldirs) $(distdir)/example
	@for file in $(DISTFILES); do \
	  d=$(srcdir); \
	  if test -d $$d/$$file; then \
	    cp -pr $$d/$$file $(distdir)/$$file; \
	  else \
	    test -f $(distdir)/$$file \
	    || ln $$d/$$file $(distdir)/$$file 2> /dev/null \
	    || cp -p $$d/$$file $(distdir)/$$file || :; \
	  fi; \
	done
	for subdir in $(DIST_SUBDIRS); do \
	  if test "$$subdir" = .; then :; else \
	    test -d $(distdir)/$$subdir \
	    || mkdir $(distdir)/$$subdir \
	    || exit 1; \
	    chmod 777 $(distdir)/$$subdir; \
	    (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) top_distdir=../$(distdir) distdir=../$(distdir)/$$subdir distdir) \
	      || exit 1; \
	  fi; \
	done
	$(MAKE) $(AM_MAKEFLAGS) top_distdir="$(top_distdir)" distdir="$(distdir)" dist-hook
c14n.lo c14n.o : c14n.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h include/libxml/uri.h \
	include/libxml/xpathInternals.h include/libxml/xpath.h \
	include/libxml/hash.h include/libxml/c14n.h
catalog.lo catalog.o : catalog.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/hash.h include/libxml/uri.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h \
	include/libxml/catalog.h
debugXML.lo debugXML.o : debugXML.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h \
	include/libxml/debugXML.h include/libxml/xpath.h \
	include/libxml/hash.h include/libxml/HTMLtree.h \
	include/libxml/xpathInternals.h
DOCBparser.lo DOCBparser.o : DOCBparser.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h \
	include/libxml/DOCBparser.h include/libxml/uri.h
encoding.lo encoding.o : encoding.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/encoding.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/HTMLparser.h
entities.lo entities.o : entities.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/hash.h
error.lo error.o : error.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/globals.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h include/libxml/xmlmemory.h \
	include/libxml/threads.h
globals.lo globals.o : globals.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/xmlmemory.h include/libxml/threads.h
hash.lo hash.o : hash.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/hash.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/globals.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/xmlmemory.h include/libxml/threads.h
HTMLparser.lo HTMLparser.o : HTMLparser.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h \
	include/libxml/HTMLtree.h
HTMLtree.lo HTMLtree.o : HTMLtree.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/HTMLparser.h include/libxml/HTMLtree.h \
	include/libxml/parserInternals.h include/libxml/uri.h
list.lo list.o : list.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h
nanoftp.lo nanoftp.o : nanoftp.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/nanoftp.h
nanohttp.lo nanohttp.o : nanohttp.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/nanohttp.h
parser.lo parser.o : parser.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h \
	include/libxml/uri.h include/libxml/catalog.h \
	include/libxml/xpath.h include/libxml/hash.h
parserInternals.lo parserInternals.o : parserInternals.c libxml.h \
	config.h include/libxml/xmlversion.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h \
	include/libxml/uri.h include/libxml/catalog.h
regexp.lo regexp.o : regexp.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/tree.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h \
	include/libxml/regexp.h include/libxml/automata.h \
	include/libxml/unicode.h
SAX.lo SAX.o : SAX.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h include/libxml/parserInternals.h \
	include/libxml/HTMLparser.h include/libxml/debugXML.h \
	include/libxml/xpath.h include/libxml/hash.h \
	include/libxml/uri.h include/libxml/HTMLtree.h
schemas.lo schemas.o : schemas.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h \
	include/libxml/hash.h include/libxml/schemas.h \
	include/libxml/schemasInternals.h include/libxml/regexp.h \
	include/libxml/schemastypes.h include/libxml/automata.h
schemastypes.lo schemastypes.o : schemastypes.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h \
	include/libxml/hash.h include/libxml/schemas.h \
	include/libxml/schemasInternals.h include/libxml/regexp.h \
	include/libxml/schemastypes.h
testAutomata.o: testAutomata.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h
testC14N.o: testC14N.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h include/libxml/xpath.h \
	include/libxml/hash.h include/libxml/xpathInternals.h \
	include/libxml/c14n.h
testDocbook.o: testDocbook.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/DOCBparser.h include/libxml/debugXML.h \
	include/libxml/xpath.h include/libxml/hash.h
testHTML.o: testHTML.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h include/libxml/HTMLparser.h \
	include/libxml/HTMLtree.h include/libxml/debugXML.h \
	include/libxml/xpath.h include/libxml/hash.h
testRegexp.o: testRegexp.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h
testSAX.o: testSAX.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/parserInternals.h \
	include/libxml/HTMLparser.h include/libxml/debugXML.h \
	include/libxml/xpath.h include/libxml/hash.h
testSchemas.o: testSchemas.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/globals.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/debugXML.h include/libxml/xpath.h \
	include/libxml/hash.h include/libxml/xmlschemas.h \
	include/libxml/xmlschemastypes.h \
	include/libxml/schemasInternals.h
testThreads.o: testThreads.c libxml.h config.h \
	include/libxml/xmlversion.h
testURI.o: testURI.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h include/libxml/uri.h
testXPath.o: testXPath.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/xpath.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/hash.h include/libxml/debugXML.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h \
	include/libxml/xpathInternals.h include/libxml/xpointer.h
threads.lo threads.o : threads.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h include/libxml/xmlmemory.h
tree.lo tree.o : tree.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h include/libxml/uri.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h \
	include/libxml/HTMLtree.h include/libxml/hash.h
unicode.lo unicode.o : unicode.c libxml.h config.h \
	include/libxml/xmlversion.h unicode.h
uri.lo uri.o : uri.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h include/libxml/uri.h
valid.lo valid.o : valid.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h include/libxml/hash.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h
xinclude.lo xinclude.o : xinclude.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/uri.h include/libxml/xpointer.h \
	include/libxml/xpath.h include/libxml/hash.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h \
	include/libxml/xinclude.h
xlink.lo xlink.o : xlink.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h
xmlcatalog.o: xmlcatalog.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h include/libxml/uri.h \
	include/libxml/catalog.h
xmlIO.lo xmlIO.o : xmlIO.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h include/libxml/parserInternals.h \
	include/libxml/HTMLparser.h include/libxml/uri.h \
	include/libxml/nanohttp.h include/libxml/nanoftp.h \
	include/libxml/catalog.h
xmllint.o: xmllint.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h include/libxml/parserInternals.h \
	include/libxml/HTMLparser.h include/libxml/HTMLtree.h \
	include/libxml/xpath.h include/libxml/hash.h \
	include/libxml/debugXML.h include/libxml/xinclude.h \
	include/libxml/catalog.h include/libxml/DOCBparser.h
xmlmemory.lo xmlmemory.o : xmlmemory.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h
xmlregexp.lo xmlregexp.o : xmlregexp.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h \
	include/libxml/xmlunicode.h
xmlschemas.lo xmlschemas.o : xmlschemas.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h \
	include/libxml/hash.h include/libxml/uri.h \
	include/libxml/xmlschemas.h include/libxml/schemasInternals.h \
	include/libxml/xmlschemastypes.h
xmlschemastypes.lo xmlschemastypes.o : xmlschemastypes.c libxml.h \
	config.h include/libxml/xmlversion.h include/libxml/xmlmemory.h \
	include/libxml/threads.h include/libxml/globals.h \
	include/libxml/parser.h include/libxml/tree.h \
	include/libxml/xmlregexp.h include/libxml/valid.h \
	include/libxml/list.h include/libxml/xmlautomata.h \
	include/libxml/entities.h include/libxml/encoding.h \
	include/libxml/xmlIO.h include/libxml/xmlerror.h \
	include/libxml/SAX.h include/libxml/xlink.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h \
	include/libxml/hash.h include/libxml/xmlschemas.h \
	include/libxml/schemasInternals.h \
	include/libxml/xmlschemastypes.h
xmlunicode.lo xmlunicode.o : xmlunicode.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/xmlunicode.h
xpath.lo xpath.o : xpath.c libxml.h config.h include/libxml/xmlversion.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h include/libxml/xpath.h \
	include/libxml/hash.h include/libxml/xpathInternals.h \
	include/libxml/parserInternals.h include/libxml/HTMLparser.h \
	include/libxml/xpointer.h include/libxml/debugXML.h trionan.c \
	triodef.h trionan.h
xpointer.lo xpointer.o : xpointer.c libxml.h config.h \
	include/libxml/xmlversion.h include/libxml/xpointer.h \
	include/libxml/tree.h include/libxml/xmlregexp.h \
	include/libxml/xmlmemory.h include/libxml/threads.h \
	include/libxml/globals.h include/libxml/parser.h \
	include/libxml/valid.h include/libxml/list.h \
	include/libxml/xmlautomata.h include/libxml/entities.h \
	include/libxml/encoding.h include/libxml/xmlIO.h \
	include/libxml/xmlerror.h include/libxml/SAX.h \
	include/libxml/xlink.h include/libxml/xpath.h \
	include/libxml/hash.h include/libxml/parserInternals.h \
	include/libxml/HTMLparser.h include/libxml/uri.h \
	include/libxml/xpathInternals.h

info-am:
info: info-recursive
dvi-am:
dvi: dvi-recursive
check-am: all-am
	$(MAKE) $(AM_MAKEFLAGS) check-local
check: check-recursive
installcheck-am:
installcheck: installcheck-recursive
all-recursive-am: config.h
	$(MAKE) $(AM_MAKEFLAGS) all-recursive

install-exec-am: install-libLTLIBRARIES install-binPROGRAMS \
		install-binSCRIPTS install-confexecDATA
install-exec: install-exec-recursive

install-data-am: install-man install-m4dataDATA install-pkgconfigDATA \
		install-data-local
install-data: install-data-recursive

install-am: all-am
	@$(MAKE) $(AM_MAKEFLAGS) install-exec-am install-data-am
install: install-recursive
uninstall-am: uninstall-libLTLIBRARIES uninstall-binPROGRAMS \
		uninstall-binSCRIPTS uninstall-man \
		uninstall-confexecDATA uninstall-m4dataDATA \
		uninstall-pkgconfigDATA
uninstall: uninstall-recursive
all-am: Makefile $(LTLIBRARIES) $(PROGRAMS) $(SCRIPTS) $(MANS) $(DATA) \
		config.h
all-redirect: all-recursive-am
install-strip:
	$(MAKE) $(AM_MAKEFLAGS) AM_INSTALL_PROGRAM_FLAGS=-s install
installdirs: installdirs-recursive
installdirs-am:
	$(mkinstalldirs)  $(DESTDIR)$(libdir) $(DESTDIR)$(bindir) \
		$(DESTDIR)$(bindir) $(DESTDIR)$(mandir)/man1 \
		$(DESTDIR)$(mandir)/man3 $(DESTDIR)$(confexecdir) \
		$(DESTDIR)$(m4datadir) $(DESTDIR)$(pkgconfigdir)


mostlyclean-generic:

clean-generic:
	-test -z "$(CLEANFILES)" || rm -f $(CLEANFILES)

distclean-generic:
	-rm -f Makefile $(CONFIG_CLEAN_FILES)
	-rm -f config.cache config.log stamp-h stamp-h[0-9]*

maintainer-clean-generic:
mostlyclean-am:  mostlyclean-hdr mostlyclean-libLTLIBRARIES \
		mostlyclean-compile mostlyclean-libtool \
		mostlyclean-binPROGRAMS mostlyclean-noinstPROGRAMS \
		mostlyclean-tags mostlyclean-generic

mostlyclean: mostlyclean-recursive

clean-am:  clean-hdr clean-libLTLIBRARIES clean-compile clean-libtool \
		clean-binPROGRAMS clean-noinstPROGRAMS clean-tags \
		clean-generic mostlyclean-am

clean: clean-recursive

distclean-am:  distclean-hdr distclean-libLTLIBRARIES distclean-compile \
		distclean-libtool distclean-binPROGRAMS \
		distclean-noinstPROGRAMS distclean-tags \
		distclean-generic clean-am
	-rm -f libtool

distclean: distclean-recursive
	-rm -f config.status

maintainer-clean-am:  maintainer-clean-hdr \
		maintainer-clean-libLTLIBRARIES \
		maintainer-clean-compile maintainer-clean-libtool \
		maintainer-clean-binPROGRAMS \
		maintainer-clean-noinstPROGRAMS maintainer-clean-tags \
		maintainer-clean-generic distclean-am
	@echo "This command is intended for maintainers to use;"
	@echo "it deletes files that may require special tools to rebuild."

maintainer-clean: maintainer-clean-recursive
	-rm -f config.status

.PHONY: mostlyclean-hdr distclean-hdr clean-hdr maintainer-clean-hdr \
mostlyclean-libLTLIBRARIES distclean-libLTLIBRARIES \
clean-libLTLIBRARIES maintainer-clean-libLTLIBRARIES \
uninstall-libLTLIBRARIES install-libLTLIBRARIES mostlyclean-compile \
distclean-compile clean-compile maintainer-clean-compile \
mostlyclean-libtool distclean-libtool clean-libtool \
maintainer-clean-libtool mostlyclean-binPROGRAMS distclean-binPROGRAMS \
clean-binPROGRAMS maintainer-clean-binPROGRAMS uninstall-binPROGRAMS \
install-binPROGRAMS mostlyclean-noinstPROGRAMS distclean-noinstPROGRAMS \
clean-noinstPROGRAMS maintainer-clean-noinstPROGRAMS \
uninstall-binSCRIPTS install-binSCRIPTS install-man1 uninstall-man1 \
install-man3 uninstall-man3 install-man uninstall-man \
uninstall-confexecDATA install-confexecDATA uninstall-m4dataDATA \
install-m4dataDATA uninstall-pkgconfigDATA install-pkgconfigDATA \
install-data-recursive uninstall-data-recursive install-exec-recursive \
uninstall-exec-recursive installdirs-recursive uninstalldirs-recursive \
all-recursive check-recursive installcheck-recursive info-recursive \
dvi-recursive mostlyclean-recursive distclean-recursive clean-recursive \
maintainer-clean-recursive tags tags-recursive mostlyclean-tags \
distclean-tags clean-tags maintainer-clean-tags distdir info-am info \
dvi-am dvi check-local check check-am installcheck-am installcheck \
all-recursive-am install-exec-am install-exec install-data-local \
install-data-am install-data install-am install uninstall-am uninstall \
all-redirect all-am all installdirs-am installdirs mostlyclean-generic \
distclean-generic clean-generic maintainer-clean-generic clean \
mostlyclean distclean maintainer-clean


check-local: tests

testall : tests SVGtests SAXtests

tests: XMLtests XMLenttests HTMLtests Validtests URItests XPathtests XPtrtests XIncludetests C14Ntests Scripttests Catatests Regexptests Automatatests  
	@(if [ "python" != "" ] ; then cd python ; $(MAKE) tests ; fi)

valgrind:
	@echo '## Running the regression tests under Valgrind'
	@echo '## Go get a cup of coffee it is gonna take a while ...'
	$(MAKE) CHECKER='valgrind -q' tests

HTMLtests : testHTML$(EXEEXT)
	@(echo > .memdump)
	@echo "##"
	@echo "## HTML regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/HTML/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/HTML/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/testHTML $$i > $(srcdir)/result/HTML/$$name 2>$(srcdir)/result/HTML/$$name.err ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/testHTML $$i > result.$$name 2> error.$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/HTML/$$name result.$$name ; \
	      diff -b $(srcdir)/result/HTML/$$name.err error.$$name ; \
	      $(CHECKER) $(top_builddir)/testHTML result.$$name > result2.$$name 2>error.$$name ; \
	      diff result.$$name result2.$$name ; \
	      rm result.$$name result2.$$name error.$$name ; \
	  fi ; fi ; done)
	@echo "##"
	@echo "## Push HTML regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/HTML/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/HTML/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/testHTML $$i > $(srcdir)/result/HTML/$$name 2>$(srcdir)/result/HTML/$$name.err ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/testHTML --push $$i > result.$$name 2> error.$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/HTML/$$name result.$$name ; \
	      cut -b 1-15 $(srcdir)/result/HTML/$$name.err > errorcut.$$name; \
	      cut -b 1-15 error.$$name > errorcut2.$$name; \
	      diff -b errorcut.$$name errorcut2.$$name ; \
	      $(CHECKER) $(top_builddir)/testHTML --push result.$$name > result2.$$name 2>error.$$name ; \
	      diff result.$$name result2.$$name ; \
	      rm result.$$name result2.$$name error.$$name errorcut.$$name errorcut2.$$name ; \
	  fi ; fi ; done)
	@echo "##"
	@echo "## HTML SAX regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/HTML/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/HTML/$$name.sax ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/testHTML --sax $$i > $(srcdir)/result/HTML/$$name.sax ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/testHTML --sax $$i > result.$$name.sax ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/HTML/$$name.sax result.$$name.sax ; \
	      rm result.$$name.sax ; \
	  fi ; fi ; done)
	@echo "##"
	@echo "## Push HTML SAX regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/HTML/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/HTML/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/testHTML --sax $$i > $(srcdir)/result/HTML/$$name.sax ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/testHTML --push --sax $$i > result.$$name.sax ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/HTML/$$name.sax result.$$name.sax ; \
	      rm result.$$name.sax ; \
	  fi ; fi ; done)

XMLtests : xmllint$(EXEEXT)
	@(echo > .memdump)
	@echo "##"
	@echo "## XML regression tests"
	@echo "##"
	@echo "## You should see one warning on entity 'title.xml' for ent2"
	@echo "##"
	-@(for i in $(srcdir)/test/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint $$i > $(srcdir)/result/$$name ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint $$i > result.$$name ; \
	      diff $(srcdir)/result/$$name result.$$name ; \
	      $(CHECKER) $(top_builddir)/xmllint result.$$name > result2.$$name ; \
	      diff result.$$name result2.$$name ; \
	      rm result.$$name result2.$$name ; \
	  fi ; fi ; done)
	@(echo > .memdump)
	@echo "##"
	@echo "## XML push regression tests"
	@echo "##"
	@echo "## You should see one warning on entity 'title.xml' for ent2"
	@echo "##"
	-@(for i in $(srcdir)/test/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --push $$i > $(srcdir)/result/$$name ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --push $$i > result.$$name ; \
	      diff $(srcdir)/result/$$name result.$$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --push result.$$name > result2.$$name ; \
	      diff result.$$name result2.$$name ; \
	      rm result.$$name result2.$$name ; \
	  fi ; fi ; done)
	@echo "##"
	@echo "## XML regression tests on memory"
	@echo "##"
	@echo "## You should see two warnings on entity 'title.xml' for ent2"
	@echo "##"
	-@(for i in $(srcdir)/test/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --memory $$i > $(srcdir)/result/$$name ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --memory $$i > result.$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/$$name result.$$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --memory result.$$name > result2.$$name ; \
	      diff result.$$name result2.$$name ; \
	      rm result.$$name result2.$$name ; \
	  fi ; fi ; done)

Docbtests : xmllint$(EXEEXT)
	@(echo > .memdump)
	@echo "##"
	@echo "## SGML DocBook regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/DocBook/*.sgm ; do \
	  name=`basename $$i .sgm`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/DocBook/$$name.xml ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --sgml $$i > $(srcdir)/result/DocBook/$$name.xml ; \
	      $(CHECKER) $(top_builddir)/xmllint --valid --noout $(srcdir)/result/DocBook/$$name.xml ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --sgml $$i > result.$$name ; \
	      diff $(srcdir)/result/DocBook/$$name.xml result.$$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --valid --noout result.$$name ; \
	      rm result.$$name ; \
	  fi ; fi ; done)

XMLenttests : xmllint$(EXEEXT)
	@(echo > .memdump)
	@echo "##"
	@echo "## XML entity subst regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/noent/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --noent $$i > $(srcdir)/result/noent/$$name ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --noent $$i > result.$$name ; \
	      diff $(srcdir)/result/noent/$$name result.$$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --noent result.$$name > result2.$$name ; \
	      diff result.$$name result2.$$name ; \
	      rm result.$$name result2.$$name ; \
	  fi ; fi ; done)

URItests : testURI$(EXEEXT)
	@(echo > .memdump)
	@echo "##"
	@echo "## URI module regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/URI/*.data ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/URI/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/testURI -base 'http://foo.com/path/to/index.html?orig#help' < $$i > $(srcdir)/result/URI/$$name ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/testURI -base 'http://foo.com/path/to/index.html?orig#help' < $$i > result.$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/URI/$$name result.$$name ; \
	      rm result.$$name ; \
	  fi ; fi ; done)
	-@(for i in $(srcdir)/test/URI/*.uri ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/URI/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/testURI < $$i > $(srcdir)/result/URI/$$name ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/testURI < $$i > result.$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/URI/$$name result.$$name ; \
	      rm result.$$name ; \
	  fi ; fi ; done)

XPathtests : testXPath$(EXEEXT)
	@(echo > .memdump)
	@echo "##"
	@echo "## XPath regression tests"
	@echo "##"
	-@(if [ "`$(top_builddir)/testXPath | grep 'support not compiled in'`" != "" ] ; \
	  then echo Skipping debug not compiled in ; exit 0 ; fi ; \
	  for i in $(srcdir)/test/XPath/expr/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/XPath/expr/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/testXPath -f --expr $$i > $(srcdir)/result/XPath/expr/$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/testXPath -f --expr $$i > result.$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/XPath/expr/$$name result.$$name ; \
	      rm result.$$name ; \
	  fi ; fi ; done ; \
	  for i in $(srcdir)/test/XPath/docs/* ; do \
	  if [ ! -d $$i ] ; then \
	  doc=`basename $$i`; \
	  for j in $(srcdir)/test/XPath/tests/$$doc* ; do \
	  if [ ! -f $$j ] ; then continue ; fi ; \
	  name=`basename $$j`; \
	  if [ ! -d $$j ] ; then \
	  if [ ! -f $(srcdir)/result/XPath/tests/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/testXPath -f -i $$i $$j > $(srcdir)/result/XPath/tests/$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/testXPath -f -i $$i $$j > result.$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/XPath/tests/$$name result.$$name ; \
	      rm result.$$name ; \
	  fi ; fi ; done ; fi ; done)

XPtrtests : testXPath$(EXEEXT)
	@(echo > .memdump)
	@echo "##"
	@echo "## XPointer regression tests"
	@echo "##"
	-@(if [ "`$(top_builddir)/testXPath | grep 'support not compiled in'`" != "" ] ; \
	  then echo Skipping debug not compiled in ; exit 0 ; fi ; \
	  for i in $(srcdir)/test/XPath/docs/* ; do \
	  if [ ! -d $$i ] ; then \
	  doc=`basename $$i`; \
	  for j in $(srcdir)/test/XPath/xptr/$$doc* ; do \
	  if [ ! -f $$j ] ; then continue ; fi ; \
	  name=`basename $$j`; \
	  if [ ! -d $$j ] ; then \
	  if [ ! -f $(srcdir)/result/XPath/xptr/$$name ] ; then \
	      echo New test file $$name ; \
	      ./testXPath -xptr -f -i $$i $$j > $(srcdir)/result/XPath/xptr/$$name ; \
	  else \
	      echo Testing $$name ; \
	      ./testXPath -xptr -f -i $$i $$j > result.$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/XPath/xptr/$$name result.$$name ; \
	      rm result.$$name ; \
	  fi ; fi ; done ; fi ; done)

XIncludetests : xmllint$(EXEEXT)
	@(echo > .memdump)
	@echo "##"
	@echo "## XInclude regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/XInclude/docs/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/XInclude/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --nowarning --xinclude $$i > $(srcdir)/result/XInclude/$$name ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --nowarning --xinclude $$i > result.$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/XInclude/$$name result.$$name ; \
	      rm result.$$name ; \
	  fi ; fi ; done)

Scripttests : xmllint$(EXEEXT)
	@(echo > .memdump)
	@echo "##"
	@echo "## Scripts regression tests"
	@echo "## Some of the base computations may be different if srcdir != ."
	@echo "##"
	-@(for i in $(srcdir)/test/scripts/*.script ; do \
	  name=`basename $$i .script`; \
	  xml=$(srcdir)/test/scripts/`basename $$i .script`.xml; \
	  if [ -f $$xml ] ; then \
	  if [ ! -f $(srcdir)/result/scripts/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --shell $$xml < $$i > $(srcdir)/result/scripts/$$name ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --shell $$xml < $$i > result.$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/scripts/$$name result.$$name ; \
	      rm result.$$name ; \
	  fi ; fi ; done)

Catatests : xmlcatalog$(EXEEXT)
	@(echo > .memdump)
	@echo "##"
	@echo "## Catalog regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/catalogs/*.script ; do \
	  name=`basename $$i .script`; \
	  xml=$(srcdir)/test/catalogs/`basename $$i .script`.xml; \
	  if [ -f $$xml ] ; then \
	  if [ ! -f $(srcdir)/result/catalogs/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/xmlcatalog --shell $$xml < $$i > $(srcdir)/result/catalogs/$$name ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/xmlcatalog --shell $$xml < $$i > result.$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/catalogs/$$name result.$$name ; \
	      rm result.$$name ; \
	  fi ; fi ; done)
	-@(for i in $(srcdir)/test/catalogs/*.script ; do \
	  name=`basename $$i .script`; \
	  sgml=$(srcdir)/test/catalogs/`basename $$i .script`.sgml; \
	  if [ -f $$sgml ] ; then \
	  if [ ! -f $(srcdir)/result/catalogs/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/xmlcatalog --shell $$sgml < $$i > $(srcdir)/result/catalogs/$$name ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/xmlcatalog --shell $$sgml < $$i > result.$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/catalogs/$$name result.$$name ; \
	      rm result.$$name ; \
	  fi ; fi ; done)
	@echo "Add and del operations on XML Catalogs"
	-@($(CHECKER) $(top_builddir)/xmlcatalog --create --noout $(srcdir)/result/catalogs/mycatalog; \
	grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0"; \
	$(CHECKER) $(top_builddir)/xmlcatalog --noout --add public Pubid sysid $(srcdir)/result/catalogs/mycatalog; \
	grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0"; \
	$(CHECKER) $(top_builddir)/xmlcatalog --noout --add public Pubid2 sysid2 $(srcdir)/result/catalogs/mycatalog; \
	grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0"; \
	$(CHECKER) $(top_builddir)/xmlcatalog --noout --add public Pubid3 sysid3 $(srcdir)/result/catalogs/mycatalog; \
	grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0"; \
	diff result/catalogs/mycatalog.full $(srcdir)/result/catalogs/mycatalog; \
	grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0"; \
	$(CHECKER) $(top_builddir)/xmlcatalog --noout --del sysid $(srcdir)/result/catalogs/mycatalog; \
	grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0"; \
	$(CHECKER) $(top_builddir)/xmlcatalog --noout --del sysid3 $(srcdir)/result/catalogs/mycatalog; \
	grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0"; \
	$(CHECKER) $(top_builddir)/xmlcatalog --noout --del sysid2 $(srcdir)/result/catalogs/mycatalog; \
	grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0"; \
	diff result/catalogs/mycatalog.empty $(srcdir)/result/catalogs/mycatalog; \
	grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0"; \
	rm -f $(srcdir)/result/catalogs/mycatalog)

SVGtests : xmllint$(EXEEXT)
	@echo "##"
	@echo "## SVG parsing regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/SVG/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/SVG/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint $$i > $(srcdir)/result/SVG/$$name ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint $$i > result.$$name ; \
	      diff $(srcdir)/result/SVG/$$name result.$$name ; \
	      $(CHECKER) $(top_builddir)/xmllint result.$$name > result2.$$name ; \
	      diff result.$$name result2.$$name ; \
	      rm result.$$name result2.$$name ; \
	  fi ; fi ; done)

Threadtests : testThreads$(EXEEXT)
	@echo "##"
	@echo "## Threaded regression tests"
	@echo "##"
	$(CHECKER) $(top_builddir)/testThreads

SAXtests : testSAX$(EXEEXT)
	@(echo > .memdump)
	@echo "##"
	@echo "## SAX callbacks regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/SAXresult/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/testSAX $$i > $(srcdir)/SAXresult/$$name ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/testSAX $$i > result.$$name ; \
	      diff $(srcdir)/SAXresult/$$name result.$$name ; \
	      rm result.$$name ; \
	  fi ; fi ; done)

Validtests : xmllint$(EXEEXT)
	@(echo > .memdump)
	@echo "##"
	@echo "## Valid documents regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/VCM/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --valid --noout --nowarning $$i ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	  fi ; done ; exit 0)
	@echo "##"
	@echo "## Validity checking regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/VC/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/VC/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --noout --valid $$i 2> $(srcdir)/result/VC/$$name ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --noout --valid $$i 2> result.$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/VC/$$name result.$$name ; \
	      rm result.$$name ; \
	  fi ; fi ; done)
	@echo "##"
	@echo "## General documents valid regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/valid/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/valid/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --valid $$i > $(srcdir)/result/valid/$$name 2>$(srcdir)/result/valid/$$name.err ; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/xmllint --valid $$i > result.$$name 2>error.$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/valid/$$name result.$$name ; \
	      diff $(srcdir)/result/valid/$$name.err error.$$name ; \
	      rm result.$$name error.$$name ; \
	  fi ; fi ; done)

Regexptests: testRegexp$(EXEEXT)
	@(echo > .memdump)
	@echo "##"
	@echo "## Regexp regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/regexp/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/regexp/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/testRegexp -i $$i > $(srcdir)/result/regexp/$$name; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/testRegexp -i $$i > result.$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/regexp/$$name result.$$name ; \
	      rm result.$$name ; \
	  fi ; fi ; done)

Automatatests: testAutomata$(EXEEXT)
	@(echo > .memdump)
	@echo "##"
	@echo "## Automata regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/automata/* ; do \
	  name=`basename $$i`; \
	  if [ ! -d $$i ] ; then \
	  if [ ! -f $(srcdir)/result/automata/$$name ] ; then \
	      echo New test file $$name ; \
	      $(CHECKER) $(top_builddir)/testAutomata $$i > $(srcdir)/result/automata/$$name; \
	  else \
	      echo Testing $$name ; \
	      $(CHECKER) $(top_builddir)/testAutomata $$i > result.$$name ; \
	      grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	      diff $(srcdir)/result/automata/$$name result.$$name ; \
	      rm result.$$name ; \
	  fi ; fi ; done)

C14Ntests : testC14N$(EXEEXT)
	@echo "##"
	@echo "## C14N and XPath regression tests"
	@echo "##"
	-@(for m in with-comments without-comments exc-without-comments ; do \
	    for i in $(srcdir)/test/c14n/$$m/*.xml ; do  \
		if [ ! -d $$i ] ; then \
		    name=`basename $$i .xml`; \
		    echo Testing $$m/$$name; \
		    cmdline="$(CHECKER) $(top_builddir)/testC14N --$$m $$i"; \
		    if [ -f $(srcdir)/test/c14n/$$m/$$name.xpath ] ; then \
			cmdline="$$cmdline $(srcdir)/test/c14n/$$m/$$name.xpath"; \
			if [ -f $(srcdir)/test/c14n/$$m/$$name.ns ] ; then \
			    cmdline="$$cmdline '`cat $(srcdir)/test/c14n/$$m/$$name.ns`'"; \
			fi; \
		    fi; \
		    $$cmdline > $(srcdir)/test/c14n/test.tmp; \
		    if [ $$? -eq 0 ]; then \
			diff  $(srcdir)/result/c14n/$$m/$$name $(srcdir)/test/c14n/test.tmp; \
			if [ $$? -ne 0 ]; then \
			    echo "Test failed"; \
			    cat $(srcdir)/test/c14n/test.tmp; \
			fi; \
		    else \
			echo "C14N failed"; \
		    fi; \
		fi; \
		rm -f $(srcdir)/test/c14n/test.tmp; \
	    done; \
	done)

Schemastests: testSchemas$(EXEEXT)
	@(echo > .memdump)
	@echo "##"
	@echo "## Schemas regression tests"
	@echo "##"
	-@(for i in $(srcdir)/test/schemas/*_*.xsd ; do \
	  name=`basename $$i | sed 's+_.*++'`; \
	  sno=`basename $$i | sed 's+.*_\(.*\).xsd+\1+'`; \
	  for j in $(srcdir)/test/schemas/"$$name"_*.xml ; do \
	      xno=`basename $$j | sed 's+.*_\(.*\).xml+\1+'`; \
	      if [ ! -f $(srcdir)/result/schemas/"$$name"_"$$sno"_"$$xno" ]; \
	      then \
		  echo New test file "$$name"_"$$sno"_"$$xno" ; \
		  $(CHECKER) $(top_builddir)/testSchemas $$i $$j \
		    > $(srcdir)/result/schemas/"$$name"_"$$sno"_"$$xno" \
		    2> $(srcdir)/result/schemas/"$$name"_"$$sno"_"$$xno".err; \
	      else \
		  echo Testing "$$name"_"$$sno"_"$$xno" ; \
	          $(CHECKER) $(top_builddir)/testSchemas $$i $$j \
		    > res.$$name 2> err.$$name;\
	          grep "MORY ALLO" .memdump  | grep -v "MEMORY ALLOCATED : 0";\
	          diff $(srcdir)/result/schemas/"$$name"_"$$sno"_"$$xno" \
		       res.$$name;\
	          diff $(srcdir)/result/schemas/"$$name"_"$$sno"_"$$xno".err \
		       err.$$name;\
		  grep Unimplemented err.$$name; \
	          rm res.$$name err.$$name ; \
	       fi ; \
	  done; done)

dist-hook: libxml.spec
	-cp libxml.spec $(distdir)
	(cd $(srcdir) ; tar -cf - --exclude CVS win32 macos vms test result SAXresult ) | (cd $(distdir); tar xf -)

cleantar:
	@(rm -f libxml*.tar.gz COPYING.LIB)

rpm: cleantar
	@(unset CDPATH ; $(MAKE) dist && rpmbuild -ta $(distdir).tar.gz)

install-data-local: 
	$(mkinstalldirs) $(DESTDIR)$(BASE_DIR)/$(DOC_MODULE)
	-/usr/bin/install -c -m 0644 $(srcdir)/Copyright $(DESTDIR)$(BASE_DIR)/$(DOC_MODULE)
	$(mkinstalldirs) $(DESTDIR)$(EXAMPLES_DIR)
	-/usr/bin/install -c -m 0644 $(srcdir)/xmllint.c $(DESTDIR)$(EXAMPLES_DIR)
	-/usr/bin/install -c -m 0644 $(srcdir)/testSAX.c $(DESTDIR)$(EXAMPLES_DIR)
	-/usr/bin/install -c -m 0644 $(srcdir)/testHTML.c $(DESTDIR)$(EXAMPLES_DIR)
	-/usr/bin/install -c -m 0644 $(srcdir)/testXPath.c $(DESTDIR)$(EXAMPLES_DIR)

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
