TEMPLATE = subdirs

shared {
	SUBDIRS	*= accessible codecs imageformats sqldrivers styles
	embedded:SUBDIRS *=  gfxdrivers
}
dll {
	SUBDIRS	*= accessible codecs imageformats sqldrivers styles
	embedded:SUBDIRS *=  gfxdrivers
}
