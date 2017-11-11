// ======================================================================
//
// PropertyRelativeFileItem.h
// Copyright 2005, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyRelativeFileItem_H
#define INCLUDED_PropertyRelativeFileItem_H

// ----------------------------------------------------------------------

#include "PropertyList/PropertyFileItem.h"

// ----------------------------------------------------------------------

class PropertyRelativeFileItem : public PropertyFileItem
{
	Q_OBJECT

public:
	PropertyRelativeFileItem(PropertyList *l, PropertyItem *after, PropertyItem *prop,
		const QString &propName, const QString& defaultDirectory, const QString& extensionFilter);
	~PropertyRelativeFileItem();

protected:
	virtual bool validate(QString & value);
};

// ======================================================================

#endif // INCLUDED_PropertyRelativeFileItem_H
