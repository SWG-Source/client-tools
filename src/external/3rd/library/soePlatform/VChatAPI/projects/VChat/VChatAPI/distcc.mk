ifneq (,$(filter /usr/bin/c++, $(shell ls /usr/bin/c++)))
	export DISTCC_HOSTS=sdplatdev1-int sdplatdev2-int sdplatdev3 sdt-build1 sdt-build2
	CPP=./cc.sh
	CFLAGS_USER+=-w
    ifeq ($(LINK),c++)
         LINK=g++
	endif
	MAKEFLAGS+="-j CPP=$(CPP) "
else	
	MAKEFLAGS+=-j
endif

