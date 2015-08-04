// ======================================================================
//
// Pixmaps.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Pixmaps_H
#define INCLUDED_Pixmaps_H

// ======================================================================

class QPixmap;

// ======================================================================

struct Pixmaps
{
	static QPixmap *applicationIcon;
	static QPixmap *binaryFileIcon;
	static QPixmap *changeListIcon;
	static QPixmap *depotIcon;
	static QPixmap *folderIcon;
	static QPixmap *textFileIcon;

	static void install();
};

// ======================================================================

#endif
