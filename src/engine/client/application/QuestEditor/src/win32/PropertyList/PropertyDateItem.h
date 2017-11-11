// ======================================================================
//
// PropertyDateItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyDateItem_H
#define INCLUDED_PropertyDateItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

class QDateEdit;

// ----------------------------------------------------------------------

class PropertyDateItem : public QObject, public PropertyItem
{
	Q_OBJECT

public:
	PropertyDateItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName);
	~PropertyDateItem();

	virtual void showEditor();
	virtual void hideEditor();

	virtual void setValue(const QVariant &v);

private slots:
	void setValue();

private:
	QDateEdit *lined();
	QGuardedPtr<QDateEdit> lin;
};

// ======================================================================

#endif // INCLUDED_PropertyDateItem_H
