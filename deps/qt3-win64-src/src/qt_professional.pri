!xml:contains( DEFINES, QT_INTERNAL_XML ) {
	CONFIG += xml
	XML_CPP = $$QT_SOURCE_TREE/src/xml
	win32 {
		WIN_ALL_H = $$QT_SOURCE_TREE/include
		XML_H		= $$WIN_ALL_H
	}
	unix {
		XML_H		= $$XML_CPP
		#needed for svg
		LIBS            += -lm  
	}
	INCLUDEPATH += $$QT_SOURCE_TREE/src/xml
	include( $$QT_SOURCE_TREE/src/xml/qt_xml.pri )
	DEFINES     *= QT_MODULE_XML 
}

!network:contains( DEFINES, QT_INTERNAL_NETWORK) {
	CONFIG += network
	NETWORK_CPP = $$QT_SOURCE_TREE/src/network
	win32 {
		WIN_ALL_H = $$QT_SOURCE_TREE/include
		NETWORK_H	= $$WIN_ALL_H
	}
	unix {
		NETWORK_H	= $$NETWORK_CPP
	}
	mac:INCLUDEPATH += $$QT_SOURCE_TREE/src/3rdparty/dlcompat
	INCLUDEPATH += $$QT_SOURCE_TREE/src/network
	include( $$QT_SOURCE_TREE/src/network/qt_network.pri )
	DEFINES     *= QT_MODULE_NETWORK
}

!workspace:contains( DEFINES, QT_INTERNAL_WORKSPACE ) {
	CONFIG += workspace
	WORKSPACE_CPP = $$QT_SOURCE_TREE/src/workspace
	win32 {
		WIN_ALL_H = $$QT_SOURCE_TREE/include
		WORKSPACE_H = $$WIN_ALL_H
	}
	unix {
		WORKSPACE_H = $$WORKSPACE_CPP
	}
	INCLUDEPATH += $$QT_SOURCE_TREE/src/workspace
	include( $$QT_SOURCE_TREE/src/workspace/qt_workspace.pri )
	DEFINES     *= QT_MODULE_WORKSPACE
}

!iconview:contains( DEFINES, QT_INTERNAL_ICONVIEW ) {
	CONFIG += iconview
	ICONVIEW_CPP = $$QT_SOURCE_TREE/src/iconview
	win32 {
		WIN_ALL_H = $$QT_SOURCE_TREE/include
		ICONVIEW_H = $$WIN_ALL_H
	}
	unix {
		ICONVIEW_H = $$ICONVIEW_CPP
	}
	INCLUDEPATH += $$QT_SOURCE_TREE/src/iconview
	include( $$QT_SOURCE_TREE/src/iconview/qt_iconview.pri )
	DEFINES     *= QT_MODULE_ICONVIEW
}

!canvas:contains( DEFINES, QT_INTERNAL_CANVAS ) {
	CONFIG += canvas
	CANVAS_CPP = $$QT_SOURCE_TREE/src/canvas
	win32 {
		WIN_ALL_H = $$QT_SOURCE_TREE/include
		CANVAS_H = $$WIN_ALL_H
	}
	unix {
		CANVAS_H = $$CANVAS_CPP
	}
	INCLUDEPATH += $$QT_SOURCE_TREE/src/canvas
	include( $$QT_SOURCE_TREE/src/canvas/qt_canvas.pri )
	DEFINES     *= QT_MODULE_CANVAS
}

!table:contains( DEFINES, QT_INTERNAL_TABLE ) {
	CONFIG += table
	TABLE_CPP = $$QT_SOURCE_TREE/src/table
	win32 {
		WIN_ALL_H = $$QT_SOURCE_TREE/include
		TABLE_H = $$WIN_ALL_H
	}
	unix {
		TABLE_H = $$TABLE_CPP
	}
	INCLUDEPATH += $$QT_SOURCE_TREE/src/table
	include( $$QT_SOURCE_TREE/src/table/qt_table.pri )
	DEFINES     *= QT_MODULE_TABLE
}

contains(QT_PRODUCT,qt-professional) {
	DEFINES     *= QT_LICENSE_PROFESSIONAL
}
