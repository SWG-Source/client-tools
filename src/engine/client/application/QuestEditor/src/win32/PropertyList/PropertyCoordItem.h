// ======================================================================
//
// PropertyCoordItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyCoordItem_H
#define INCLUDED_PropertyCoordItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

class QLineEdit;

// ----------------------------------------------------------------------

class PropertyCoordItem : public QObject, public PropertyItem
{
	Q_OBJECT

public:
	enum Type { Rect, Size, Point };

	PropertyCoordItem(PropertyList *l, PropertyItem *after, PropertyItem *prop,
		const QString &propName, Type t);
	~PropertyCoordItem();

	virtual void createChildren();
	virtual void initChildren();

	virtual void showEditor();
	virtual void hideEditor();

	virtual void setValue(const QVariant &v);
	virtual bool hasSubItems() const;
	virtual void childValueChanged( PropertyItem *child );

private:
	QLineEdit *lined();
	QGuardedPtr<QLineEdit> lin;
	Type typ;
};

// ======================================================================

#endif // INCLUDED_PropertyCoordItem_H
