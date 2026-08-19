# qt3 with g++ 3.3 
# fixed qmake quilib problem
# added .qmake.cache to dist 
# added creating of symlink /opt/qt3
# added DISPLAY setting 
# added xft and xkb extensions 
# added xfreetype config patch 
# removed cygwin-g++-2 specs  
# uses minires lib, but minires does not contain a dll, so a link "ln -fs libminires.a /usr/lib/libresolv.a" is required

#inherit cvs

#ECVS_SERVER="offline"
#ECVS_USER="anoncvs"
#ECVS_PASS="anoncvs"
#ECVS_MODULE="qt-3"
#ECVS_TOP_DIR="${DISTDIR}/cvs-src/${PN}"
#S=${WORKDIR}/${ECVS_MODULE}

# please not minires is currently compiled in staticly 
DEPEND="
	cygwin-binaries/make
	cygwin-binaries/binutils
	cygwin-binaries/gcc
	cygwin-binaries/flex
	cygwin-binaries/byacc
	cygwin-binaries/xorg-x11-base
	cygwin-binaries/xorg-x11-devel
	cygwin-binaries/minires-devel
	cygwin-binaries/freetype2-devel
	cygwin-binaries/fontconfig-devel
"

RDEPEND="
	cygwin-binaries/xorg-x11-base
	cygwin-binaries/fontconfig
	cygwin-binaries/freetype2
	cygwin-binaries/minires
"

DESCRIPTION=QT-Library 
HOMEPAGE=http://www.trolltech.com 

QTRELEASE=3.3
QTBASE=/opt/qt/${QTRELEASE}
#subdir under workdir required if really unpacked
#SRC_ROOT=qt-3

#SRC_URI="ftp://ftp.trolltech.com/qt/source/qt-x11-free-3.3.3.tar.bz2"

SLOT="3.3"
LICENSE="GPL"
KEYWORDS="x86"

src_unpack() {
#    cvs_src_unpack ${A}
    unpack ${A}
    cd ${WORKDIR}/${P}
    make -f Makefile.cvs
}

src_compile() {
    cd ${S}
        
    export QTDIR=`pwd`
    export PATH=$QTDIR/bin:$PATH
    
    export YACC='byacc -d'

    if  [ ! -f .qmake.cache ]; then 
       echo yes | ./configure -platform cygwin-g++ -fast -plugin-imgfmt-mng -thread -no-stl \
       	-qt-zlib -qt-gif -qt-libpng -qt-libjpeg -qt-libmng -no-g++-exceptions -xft -xkb \
       	-no-ipv6 -qt-sql-odbc -qt-sql-sqlite -prefix ${QTBASE}
    fi     

		make src-qmake src-moc sub-src sub-tools 
}

src_install() {
    echo install step reached

    if test -n "$QT_SOURCE_TREE"; then 
      SRC=$QT_SOURCE_TREE
    else 
    	if ! test -d "$QTDIR/translations"; then 
    		echo "Seems that you are building qt outside the source dir, please set QT_SOURCE_TREE to the qt source dir"
    		exit
    	fi 
      SRC=$QTDIR
    fi 
		
    cd ${S}

    into $QTBASE
    dobin bin/*.dll
    # bin contains only a symlink to qmake 
    dobin qmake/*.exe
    dobin bin/*.exe
    dobin bin/findtr bin/qt20fix bin/qtrename140
        
#    dolib lib/*
# do not install all the static archives 
    dodir ${QTBASE}/lib
    cp -d -L lib/*.dll.a lib/*.la  ${D}${QTBASE}/lib/
    perl -pi -e "s,${BUILDDIR},${QTBASE},g" ${D}${QTBASE}/lib/*.la
	
    cp -d lib/libq[tu]*.prl lib/*.pc ${D}${QTBASE}/lib/

    dodir ${QTBASE}/include/private
    cp -fL include/*.h ${D}${QTBASE}/include    
    cp -fL include/private/*.h ${D}${QTBASE}/include/private
    
    #todo: plugins and so on...
    exeinto ${QTBASE}/plugins/imageformats    
    doexe plugins/imageformats/*.dll  

    exeinto ${QTBASE}/plugins/designer
    doexe plugins/designer/*.dll  

    dodir ${QTBASE}/translations
    cp -a -L ${SRC}/translations ${D}${QTBASE}

    dodir ${QTBASE}/doc
    cp -a -L doc/html ${D}${QTBASE}/doc
    cp -a -L doc/man  ${D}${QTBASE}/doc

    dodir ${QTBASE}/mkspecs
    cp -a -L mkspecs/cygwin-g++ ${D}${QTBASE}/mkspecs
    
    cp ${SRC}/README-QT.TXT ${SRC}/cygwin/README-CYGWIN.TXT ${SRC}/INSTALL ${SRC}/FAQ ${SRC}/MANIFEST ${SRC}/PLATFORMS ${SRC}/changes-* ${SRC}/.qmake.cache ${D}${QTBASE}
    cp ${SRC}/LICENSE.GPL ${D}${QTBASE}/LICENSE.GPL.TXT
    cp ${SRC}/LICENSE.QPL ${D}${QTBASE}/LICENSE.QPL.TXT
    
    perl -pi -e "s,${BUILDDIR},${QTBASE},g" ${D}${QTBASE}/.qmake.cache 
		
    # install bash startup script 	
    dodir /etc/profile.d
    QTPF=${D}/etc/profile.d/qt${QTRELEASE}.sh
    echo "export DISPLAY=127.0.0.1:0" >${QTPF}
    echo "export QTDIR=$QTBASE" >>${QTPF}
    echo "export PATH=\$QTDIR/bin:\$PATH" >>${QTPF}
    echo "export MANPATH=\$MANPATH:\$QTDIR/doc/man" >>${QTPF}
    echo "export QMAKESPEC=cygwin-g++" >>${QTPF}
 		echo "if [ ! -e /opt/qt3 ] ; then" >>${QTPF}
		echo "	ln -s qt/${QTRELEASE} /opt/qt3" >>${QTPF}
		echo "fi" >>${QTPF}

    # strip debuggings symbols  
    X=${D}${QTBASE}
		strip -s  $X/bin/*.dll $X/bin/*.exe  $X/plugins/designer/*.dll $X/plugins/imageformats/*.dll
		find ${X} -type d  -name 'CVS' -exec rm -r {} \;
		find ${X} \( -iname '*.bak' -o -iname '.*.bak' -o -iname '.#*' \) -exec rm {} \;
		
}		


	