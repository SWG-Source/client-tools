TEMPLATE = subdirs

contains( QT_PRODUCT, qt-(enterprise|internal|eval) ) {
	win32:	SUBDIRS = activeqt
	x11:	SUBDIRS = motif
}
