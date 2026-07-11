TEMPLATE	= subdirs
no-png {
    message("Tools not available without PNG support")
} else {
    SUBDIRS		= assistant/lib \
		      designer \
		      assistant \
		      linguist
    unix:SUBDIRS	+= qtconfig
    win32:SUBDIRS	+= qtconfig
}

CONFIG+=ordered
REQUIRES=full-config
