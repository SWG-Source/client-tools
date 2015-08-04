#!/bin/bash

ROOTDIR=`pwd`
cd ..
TOPDIR=`pwd`
cd $ROOTDIR

WARNING_CFLAGS="-Wall -Wno-unknown-pragmas -Wsign-compare -Wformat -Wunused"
WARNING_CXXFLAGS="$WARNING_CFLAGS -Wno-ctor-dtor-privacy -Wconversion"
RELEASE_CFLAGS="-g3 -O2 -pipe"
RELEASE_CXXFLAGS="$RELEASE_CFLAGS"
DEBUG_CFLAGS="-O0 -g3 -pipe"
DEBUG_CXXFLAGS="$DEBUG_CFLAGS"
BUILD_PREFIX=$ROOTDIR/compile/default
BINDIR=""
LIBDIR=""
BIN_TARGET="dev"

# TODO: This is a gross hack. siteSetup should work on any platform. 
# it is currently only running on Linux. autoconf should handle this!
PLATFORM="linux"

DEPENDENCY_ARGS="--with-archive=$ROOTDIR/external/ours/library/archive --with-crypto=$ROOTDIR/external/ours/library/crypto --with-engine=$ROOTDIR/engine --with-game=$ROOTDIR/game --with-fileInterface=$ROOTDIR/external/ours/library/fileInterface --with-localization=$ROOTDIR/external/ours/library/localization --with-localizationArchive=$ROOTDIR/external/ours/library/localizationArchive --with-singleton=$ROOTDIR/external/ours/library/singleton --with-unicode=$ROOTDIR/external/ours/library/unicode --with-unicodeArchive=$ROOTDIR/external/ours/library/unicodeArchive --with-boost=$ROOTDIR/external/3rd/library/boost --with-platform=$ROOTDIR/external/3rd/library/platform --with-stlport=$ROOTDIR/external/3rd/library/stlport453 --with-udplibrary=$ROOTDIR/external/3rd/library/udplibrary --with-oracle=$ROOTDIR/external/3rd/library/oracle --with-perforce=$ROOTDIR/external/3rd/library/perforce --with-pcre=$ROOTDIR/external/3rd/library/pcre/4.1/$PLATFORM --with-soePlatform=$ROOTDIR/external/3rd/library/soePlatform --with-libxml=$ROOTDIR/external/3rd/library/libxml"


# run the bootstrap
bash bootstrap
if [ $? != 0 ]; then exit 1; fi


configureDebug()
{
	echo "Configuring site for debug $BIN_TARGET builds..."
	DEBUG_BUILD_PREFIX=$ROOTDIR/compile/$BIN_TARGET/debug
	if [ ! -d $DEBUG_BUILD_PREFIX ]; then
		mkdir -p $DEBUG_BUILD_PREFIX
		if [ $? != 0 ]; then exit 1; fi
	fi
	
	cd $DEBUG_BUILD_PREFIX
	if [ $? != 0 ]; then exit 1; fi
	
	BINDIR=$TOPDIR/$BIN_TARGET/linux/debug
	LIBDIR=$BINDIR/lib
	LDFLAGS="-Wl,--rpath -Wl,./linux/debug/lib -Wl,--rpath -Wl,$BIN_TARGET/linux/debug/lib -Wl,--rpath -Wl,./debug/lib -Wl,--rpath -Wl,./lib" CXXFLAGS="$DEBUG_CXXFLAGS $WARNING_CXXFLAGS" CFLAGS="$DEBUG_CFLAGS $WARNING_CFLAGS" $ROOTDIR/configure --bindir=$BINDIR --libdir=$LIBDIR --enable-debug=full --disable-static $DEPENDENCY_ARGS
	if [ $? != 0 ]; then exit 1; fi
}

configureDebugStatic()
{
	echo "Configuring site for debug-static $BIN_TARGET builds..."
	DEBUG_STATIC_BUILD_PREFIX=$ROOTDIR/compile/$BIN_TARGET/debug-static
	if [ ! -d $DEBUG_STATIC_BUILD_PREFIX ]; then
		mkdir -p $DEBUG_STATIC_BUILD_PREFIX
		if [ $? != 0 ]; then exit 1; fi
	fi
	
	cd $DEBUG_STATIC_BUILD_PREFIX
	if [ $? != 0 ]; then exit 1; fi
	
	BINDIR=$TOPDIR/$BIN_TARGET/linux/debug-static
	LIBDIR=$BINDIR/lib
	LDFLAGS="-Wl,--rpath -Wl,./linux/debug-static/lib -Wl,--rpath -Wl,$BIN_TARGET/linux/debug-static/lib -Wl,--rpath -Wl,./debug-static/lib -Wl,--rpath -Wl,./lib" CXXFLAGS="$DEBUG_CXXFLAGS $WARNING_CXXFLAGS" CFLAGS="$DEBUG_CFLAGS $WARNING_CFLAGS" $ROOTDIR/configure --bindir=$BINDIR --libdir=$LIBDIR --enable-debug=full --disable-shared $DEPENDENCY_ARGS
	if [ $? != 0 ]; then exit 1; fi
}

configureProfileStatic()
{
	echo "Configuring site for statically linled profile $BIN_TARGET builds..."
	PROFILE_STATIC_BUILD_PREFIX=$ROOTDIR/compile/$BIN_TARGET/profile-static
	if [ ! -d $PROFILE_STATIC_BUILD_PREFIX ]; then
		mkdir -p $PROFILE_STATIC_BUILD_PREFIX
		if [ $? != 0 ]; then exit 1; fi
	fi
	
	cd $PROFILE_STATIC_BUILD_PREFIX
	if [ $? != 0 ]; then exit 1; fi
	
	BINDIR=$TOPDIR/$BIN_TARGET/linux/profile-static
	LIBDIR=$BINDIR/lib
	LDFLAGS="-Wl,--rpath -Wl,./linux/profile-static/lib -Wl,--rpath -Wl,$BIN_TARGET/linux/profile-static/lib -Wl,--rpath -Wl,./profile-static/lib -Wl,--rpath -Wl,./lib" CXXFLAGS="$DEBUG_CXXFLAGS -pg $WARNING_CXXFLAGS" CFLAGS="$DEBUG_CFLAGS -pg $WARNING_CFLAGS" $ROOTDIR/configure --bindir=$BINDIR --libdir=$LIBDIR --enable-debug=full --disable-shared $DEPENDENCY_ARGS
	if [ $? != 0 ]; then exit 1; fi
}

configureRelease()
{
	echo "Configuring site for release $BIN_TARGET builds..."
	RELEASE_BUILD_PREFIX=$ROOTDIR/compile/$BIN_TARGET/release
	if [ ! -d $RELEASE_BUILD_PREFIX ]; then
		mkdir -p $RELEASE_BUILD_PREFIX
		if [ $? != 0 ]; then exit 1; fi
	fi

	cd $RELEASE_BUILD_PREFIX
	if [ $? != 0 ]; then exit 1; fi

	BINDIR=$TOPDIR/$BIN_TARGET/linux/release
	LIBDIR=$BINDIR/lib
	LDFLAGS="-Wl,--rpath -Wl,./linux/release/lib -Wl,--rpath -Wl,$BIN_TARGET/linux/release/lib -Wl,--rpath -Wl,./release/lib -Wl,--rpath -Wl,./lib" CXXFLAGS="$RELEASE_CXXFLAGS $WARNING_CXXFLAGS" CFLAGS="$RELEASE_CFLAGS $WARNING_CFLAGS" $ROOTDIR/configure --bindir=$BINDIR --libdir=$LIBDIR --disable-static $DEPENDENCY_ARGS
	if [ $? != 0 ]; then exit 1; fi
}

configureReleaseStatic()
{
	echo "Configuring site for statically linked release $BIN_TARGET builds..."
	RELEASE_STATIC_BUILD_PREFIX=$ROOTDIR/compile/$BIN_TARGET/release-static
	if [ ! -d $RELEASE_STATIC_BUILD_PREFIX ]; then
		mkdir -p $RELEASE_STATIC_BUILD_PREFIX
		if [ $? != 0 ]; then exit 1; fi
	fi

	cd $RELEASE_STATIC_BUILD_PREFIX
	if [ $? != 0 ]; then exit 1; fi

	BINDIR=$TOPDIR/$BIN_TARGET/linux/release-static
	LIBDIR=$BINDIR/lib
	LDFLAGS="-Wl,--rpath -Wl,./linux/release-static/lib -Wl,--rpath -Wl,$BIN_TARGET/linux/release-static/lib -Wl,--rpath -Wl,./release-static/lib -Wl,--rpath -Wl,./lib" CXXFLAGS="$RELEASE_CXXFLAGS $WARNING_CXXFLAGS" CFLAGS="$RELEASE_CFLAGS $WARNING_CFLAGS" $ROOTDIR/configure --bindir=$BINDIR --libdir=$LIBDIR --disable-shared $DEPENDENCY_ARGS
	if [ $? != 0 ]; then exit 1; fi
}

if [ "$1" == "--publish" ]; then
	BIN_TARGET="exe"
	configureDebug
	configureRelease
	exit 0
fi

if [ "$1" == "--release" ]; then
	configureRelease
	exit 0
fi

if [ "$1" == "--debug" ]; then
	configureDebug
	exit 0
fi

if [ "$1" == "--debug-static" ]; then
	configureDebugStatic
	exit 0
fi

if [ "$1" == "--profile" ]; then
	configureProfile
	exit 0
fi

if [ "$1" == "--profile-static" ]; then
	configureProfileStatic
	exit 0
fi

if [ "$1" == "--release-static" ]; then
	configureReleaseStatic
	exit 0
fi

if [ "$1" == "--all" ]; then
	BIN_TARGET="exe"
	configureDebug
	configureRelease
	BIN_TARGET="dev"
	configureDebug
	configureRelease
	configureDebugStatic
	configureReleaseStatic
	configureProfileStatic
	exit 0
fi

# fallback to configuring the site for debug builds/runs
echo "No configuration target was specified, falling back to debug. Other targets include publish ($0 --publish), release ($0 --release), debug ($0 --debug) and all ($0 --all)"
configureDebug
exit 0
