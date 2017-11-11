// ======================================================================
//
// PropertyRelativeFileItem.cpp
// Copyright 2005, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyRelativeFileItem.h"

// ----------------------------------------------------------------------

#include "sharedFile/TreeFile.h"

// ----------------------------------------------------------------------

#include "PropertyRelativeFileItem.moc"

// ----------------------------------------------------------------------

PropertyRelativeFileItem::PropertyRelativeFileItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName, const QString &defaultDirectory, const QString &extensionFilter)
: PropertyFileItem(l, after, prop, propName, defaultDirectory, extensionFilter)
{
}

// ----------------------------------------------------------------------

PropertyRelativeFileItem::~PropertyRelativeFileItem()
{
}

// ----------------------------------------------------------------------

bool PropertyRelativeFileItem::validate(QString & filename)
{
	filename = QString(TreeFile::getShortestExistingPath(filename));

	return !(filename.isEmpty() || filename.isNull());
}

// ======================================================================
