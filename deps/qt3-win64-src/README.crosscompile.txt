This release contains the mingw cross compile tool chain running under
linux (tested with suse linux 9.x, knoppix 3.8 and Gentoo 2007.0), which
allows you to cross compile the mingw based qt release.

Please note that this information is prelimary and may be incomplete.  If
you detect problems or have fixes please report these to the
qtwin-general@lists.sourceforge.net email list.

requirements:
- installed mingw cross compiler package
- host system based qt tools (qmake, moc and uic)

1. install mingw cross compiler package (see below for different distros).  I
   have added Gentoo information but there is no info for any other distros.
2. download a recent qt win free source package
3. make two copies of the qt win free source tree from this package.  One will
   be used to build host system binaries needed to support the actual mingw
   build and the other will be used for the actual mingw build.  In this documentation the source tree    for building the host system binaries will
   be called <qt-host-source-tree> and the mingw source directory will be
   called <qt-mingw-source-tree>
4. build a host based qt release using the downloaded host source tree
   -> cd <qt-host-source-tree>
   -> run 'make -f Makefile.cvs'
   -> run ./configure
   -> run make
   -> this step is quite important since the qmake modifications contained in
      the qt win free source tarball are essential for building qt on mingw
   -> only qmake, moc and uic must be built;  moc and uic - are not needed if
      there is an already installed host qt version that is the same as the one
      you are trying to build with the cross compiler - IE. both are 3.3.7 for
      example.  The make can be stopped when qmake, moc and uic (if needed)
      are built
5. unpack the qt source package in <qt-mingw-source-tree> - please see #2 above.
6. setup your build environement

  $ cd <qt-mingw-source-tree>
  $ ln -s <qt-host-source-tree>/bin/qmake bin/qmake

  if you built uic and moc in <qt-host-source-tree>

  $ ln -s <qt-host-source-tree>/bin/moc bin/moc
  $ ln -s <qt-host-source-tree>/bin/uic bin/uic

  or if you are using the host system uic and moc

  $ ln -s $QTDIR/bin/moc bin/moc
  $ ln -s $QTDIR/bin/uic bin/uic

  $ export QTDIR=$PWD
  $ export QMAKESPEC=win32-g++-cross
  $ export PATH=$PWD/bin:$PATH

7. create .qtwinconfig [1]

win32-g++-cross:contains(TEMPLATE,lib) {
        contains(CONFIG,shared) {
        CONFIG += dll
        }
}

8. run

   $ make -f Makefile.cvs

9 modify mkspecs/win32-g++-cross/qmake.conf so that the compiler names
  match your installation.  On my Gentoo machine I had to change this from

  i586-mingw32msvc-*

  to

  i686-mingw32-*

  So this will depend on your mingw installation.

10. run

   $ ./configure -no-cups -platform win32-g++-cross -shared -release -thread

11. edit .qmake.cache and replace the CONFIG line with

  CONFIG += enterprise nocrosscompiler stl warn_off create_prl
  link_prl minimal-config small-config medium-config large-config
  full-config no-exceptions no-rtti release shared no-incremental
  thread no-largefile no-gif no-tablet ipv6 zlib no-opengl sound
  precompile_header bigcodecs styles tools thread kernel widgets
  dialogs iconview workspace network canvas table xml opengl sql
  accessibility tablet sound png mng jpeg

and also the "styles" line with

  styles += cde common compact interlace motifplus motif platinum sgi windows

Be careful about line breaks.

12. replace include/qconfig.h file with the one from mkspecs/win32-g++-cross

  $ rm -f include/qconfig.h
  $ cp mkspecs/win32-g++-cross/qconfig.h.crosscompile include/qconfig.h

  Since I was using gcc 4 I modified the build key line from:

  /* Build key */
  #define QT_BUILD_KEY "i686 Linux g++-3.*"

  to

  /* Build key */
  #define QT_BUILD_KEY "i686 Linux g++-4.*"

  I am not sure if this was needed.

13. run

    $ make

Please note:  The linker stage for the qt-mt.dll will eat up about 831M of
memory, so take care.  Also the build will break at some point after the
library has already been created when some Makefile tries to link against
the libqtmain.a (-lqtmain) library.  To fix this:

14. compile qtmain

    $ cd src/
    $ qmake -spec ../mkspecs/win32-g++-cross -o Makefile.main qtmain.pro
    $ cp ../mkspecs/win32-g++-cross/qtcrtentrypoint.cpp ../mkspecs/win32-g++/
    $ make -f Makefile.main
    $ cd ..

15. continue building the remaing stuff

    $ make

note [1]: this is required because otherwise qmake creates exe's with
a dll extension.

mingw installation.

Gentoo

Gentoo now uses the crossdev ebuild to create cross developement environments.  After emerging it you can run crossdev to create a mingw environment.  Please see

http://gentoo-wiki.com/HOWTO_MinGW

and

http://www.gentoo.org/proj/en/base/embedded/cross-development.xml

for more details.