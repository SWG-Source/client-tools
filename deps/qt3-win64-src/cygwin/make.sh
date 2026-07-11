#!/bin/sh
# very small portage build system subset 
# (c) Ralf Habacker 	Ralf.Habacker@freenet.de 
#  requires the following cygwin packages 
#     cygwin, bash, fileutils, sh-utils, tar, wget 
#
# make.sh <ebuild file> <target>
# target = compile install package 
#
# the dir in which the source will be unpacked (on subdir above real source root)
WORKDIR=$QTDIR
# output package dir
E=/tmp/packages

# temp dir 
T=/tmp
fname=`basename $1`
P=`echo $fname | sed 's,.ebuild,,g'`
 
# temporary install root 
D=/tmp/packages/${P}
[ -d $D ] || mkdir -p $D

# source ebuild 
. $1 

BUILDDIR=${WORKDIR}
# source file(s) for unpacking 
A=$SRC_URI

S=${WORKDIR} #/${P}
cd ${S}
into()
{
	DESTTREE=$1
}

dodir()
{
for x in "$@" ; do
	install -d ${DIROPTIONS} "${D}${x}"
done
}

dobin()
{
if [ ${#} -lt 1 ] ; then
	echo "${0}: at least one argument needed"
	exit 1
fi

if [ ! -z "${CBUILD}" ] && [ "${CBUILD}" != "${CHOST}" ]; then
	STRIP=${CHOST}-strip
else
	STRIP=strip
fi

if [ ! -d "${D}${DESTTREE}/bin" ] ; then
	install -d "${D}${DESTTREE}/bin"
fi

for x in "$@" ; do
	if [ -x "${x}" ] ; then
#		if [ "${FEATURES//*nostrip*/true}" != "true" ] && [ "${RESTRICT//*nostrip*/true}" != "true" ] ; then
#			MYVAL=`file "${x}" | grep "ELF"` 
#			if [ -n "$MYVAL" ] ; then
#				${STRIP} "${x}"
#			fi
#		fi
		#if executable, use existing perms
		install "${x}" "${D}${DESTTREE}/bin"
	else
		#otherwise, use reasonable defaults
		echo ">>> dobin: making ${x} executable..."
		install -m0755 --owner=root --group=root "${x}" "${D}${DESTTREE}/bin"
	fi
done
}

dolib()
{
if [ ${#} -lt 1 ] ; then
	echo "${0}: at least one argument needed"
	exit 1
fi
if [ ! -d "${D}${DESTTREE}/lib" ] ; then
	install -d "${D}${DESTTREE}/lib"
fi

for x in "$@" ; do
	if [ -e "${x}" ] ; then
		install ${LIBOPTIONS} "${x}" "${D}${DESTTREE}/lib"
	else
		echo "${0}: ${x} does not exist"
	fi
done
}


exeinto()
{
	EXEDESTTREE=$1
}

doexe()
{
mynum=${#}
if [ ${mynum} -lt 1 ] ; then
	echo "doexe: at least one argument needed"
	exit 1
fi
if [ ! -d "${D}${EXEDESTTREE}" ] ; then
	install -d "${D}${EXEDESTTREE}"
fi

if [ ! -z "${CBUILD}" ] && [ "${CBUILD}" != "${CHOST}" ]; then
	STRIP=${CHOST}-strip
else
	STRIP=strip
fi

for x in "$@" ; do
#	if [ "${FEATURES//*nostrip*/true}" != "true" ] && [ "${RESTRICT//*nostrip*/true}" != "true" ] ; then
#		MYVAL=`file "${x}" | grep "ELF"` 
#		if [ -n "$MYVAL" ] ; then
#			${STRIP} "${x}"
#		fi
#	fi
	if [ -L "${x}" ] ; then
		cp "${x}" "${T}"
		mysrc="${T}"/`/usr/bin/basename "${x}"`
	elif [ -d "${x}" ] ; then
		echo "doexe: warning, skipping directory ${x}"
		continue
	else
		mysrc="${x}"
	fi
	install ${EXEOPTIONS} "${mysrc}" "${D}${EXEDESTTREE}"
done
}

src_fetch()
{
	cd ${T}
	wget -nd ${A}
}

unpack()
{
	cd ${WORKDIR}
	tar -xjf $1
}


src_package()
{
	cd ${D}
	tar -cjvf ${E}/${P}.tar.bz2 *
}

#src_srcpackage()
#{
#	cd ${WORKDIR}
# list all files in cvs 
#}

if test "$2" = "install"; then 
	rm -rf ${D}
fi
src_$2
