// ======================================================================
//
// Pixmaps.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "Pixmaps.h"

#include "res\application_icon.xpm"
#include "res\binary_file_icon.xpm"
#include "res\changelist_icon.xpm"
#include "res\depot_icon.xpm"
#include "res\folder_icon.xpm"
#include "res\text_file_icon.xpm"

#include <qpixmap.h>

// ======================================================================

QPixmap *Pixmaps::applicationIcon;
QPixmap *Pixmaps::binaryFileIcon;
QPixmap *Pixmaps::changeListIcon;
QPixmap *Pixmaps::depotIcon;
QPixmap *Pixmaps::folderIcon;
QPixmap *Pixmaps::textFileIcon;

// ======================================================================

void Pixmaps::install()
{
	applicationIcon = new QPixmap(application_icon);
	binaryFileIcon = new QPixmap(binary_file_icon);
	changeListIcon = new QPixmap(changelist_icon);
	depotIcon = new QPixmap(depot_icon);
	folderIcon = new QPixmap(folder_icon);
	textFileIcon = new QPixmap(text_file_icon);
}

// ======================================================================
