# -*- makefile -*-
#
# Main Makefile for building the Qt library, examples and tutorial.

SHELL=/bin/sh

init: FORCE
	@$(MAKE) all
uninstall: FORCE
	@$(MAKE) qt.uninstall
install: FORCE
	@$(MAKE) qt.install

all: symlinks src-qmake src-moc sub-src sub-tools sub-tutorial sub-examples
	@echo
	@echo "The Qt library is now built in ./lib"
	@echo "The Qt examples are built in the directories in ./examples"
	@echo "The Qt tutorials are built in the directories in ./tutorial"
	@echo
	@echo "Enjoy!   - the Trolltech team"
	@echo

qt.install: qmake-install moc-install src-install tools-install plugins-install
qt.uninstall: qmake-uninstall moc-uninstall src-uninstall tools-uninstall plugins-uninstall

#moc
moc-uninstall: .qmake.cache
	cd src/moc && $(MAKE) uninstall
moc-install: src-moc
	cd src/moc && $(MAKE) install
src-moc: src-qmake FORCE
	cd src/moc && $(MAKE)

#Qt
src-uninstall: .qmake.cache
	cd src && $(MAKE) uninstall
src-install: sub-src
	cd src && $(MAKE) install
sub-src: src-moc .qmake.cache FORCE
	cd src && $(MAKE)

#qmake
qmake-uninstall: .qmake.cache
	cd qmake && $(MAKE) uninstall
qmake-install: src-qmake
	cd qmake && $(MAKE) install
src-qmake: symlinks FORCE
	cd qmake && $(MAKE)

#tools
tools-uninstall: .qmake.cache
	cd tools && $(MAKE) uninstall
tools-install: sub-tools
	cd tools && $(MAKE) install
sub-tools: sub-plugins FORCE
	cd tools && $(MAKE)

#plugins
plugins-uninstall: .qmake.cache
	cd plugins/src && $(MAKE) uninstall
plugins-install: sub-plugins
	cd plugins/src && $(MAKE) install
sub-plugins: sub-src .qmake.cache FORCE
	cd plugins/src && $(MAKE)

#tutorials
sub-tutorial: sub-src FORCE
	cd tutorial && $(MAKE)

#examples
sub-examples: sub-tools FORCE
	cd examples && $(MAKE)

#docs
doc: FORCE
	qdoc util/qdoc/qdoc.conf

symlinks: .qmake.cache
#	@cd include && rm -f q*.h; ln -s ../src/*/q*.h .; ln -s ../extensions/*/src/q*.h .; rm -f q*_p.h

distclean clean uiclean mocclean:
	cd tools && $(MAKE) $@
	cd src/moc && $(MAKE) $@
	cd src && $(MAKE) $@
	cd tutorial && $(MAKE) $@
	cd plugins/src && $(MAKE) $@
	cd examples && $(MAKE) $@
	cd config.tests/unix/stl && $(MAKE) $@
	cd config.tests/unix/endian && $(MAKE) distclean
	cd config.tests/unix/ipv6 && $(MAKE) distclean
	cd config.tests/unix/largefile && $(MAKE) distclean
	cd config.tests/unix/ptrsize && $(MAKE) distclean
	[ ! -f config.tests/x11/notype/Makefile ] || cd config.tests/x11/notype && $(MAKE) $@
	cd qmake && $(MAKE) $@

#confclean is actually the same as distclean, except qmake and moc
#are not cleaned.
confclean:
	cd tools && $(MAKE) distclean
	cd src && $(MAKE) distclean
	cd tutorial && $(MAKE) distclean
	cd plugins/src && $(MAKE) distclean
	cd examples && $(MAKE) distclean
	cd config.tests/unix/stl && $(MAKE) distclean
	cd config.tests/unix/endian && $(MAKE) distclean
	cd config.tests/unix/ipv6 && $(MAKE) distclean
	cd config.tests/unix/largefile && $(MAKE) distclean
	cd config.tests/unix/ptrsize && $(MAKE) distclean
	[ ! -f config.tests/x11/notype/Makefile ] || cd config.tests/x11/notype && $(MAKE) distclean

.qmake.cache:
	@echo
	@echo '  Qt must first be configured using the "configure" script.'
	@echo
	@test ! /bin/true

FORCE:
