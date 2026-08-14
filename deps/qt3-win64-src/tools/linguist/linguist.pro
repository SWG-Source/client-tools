TEMPLATE	= subdirs
SUBDIRS		= linguist \
		  tutorial/tt1 \
		  tutorial/tt2 \
		  tutorial/tt3 \
		  lrelease \
		  lupdate \
		  qm2ts
CONFIG += ordered

REQUIRES=full-config nocrosscompiler
