TEMPLATE = subdirs

CONFIG  += ordered

SUBDIRS	=  uic \
	   uilib \
	   designer \
	   app

dll:SUBDIRS *=  editor plugins
shared:SUBDIRS *=  editor plugins
REQUIRES=full-config nocrosscompiler
