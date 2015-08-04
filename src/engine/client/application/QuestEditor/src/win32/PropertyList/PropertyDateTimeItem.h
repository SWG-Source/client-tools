// ======================================================================
//
// PropertyDateTimeItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyDateTimeItem_H
#define INCLUDED_PropertyDateTimeItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

class QDateTimeEdit;

// ----------------------------------------------------------------------

class PropertyDateTimeItem : public QObject, public PropertyItem
{
	Q_OBJECT

public:
	PropertyDateTimeItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName);
	~PropertyDateTimeItem();

	virtual void showEditor();
	virtual void hideEditor();

	virtual void setValue(const QVariant &v);

private slots:
	void setValue();

private:
	QDateTimeEdit *lined();
	QGuardedPtr<QDateTimeEdit> lin;
};

// ======================================================================

#endif // INCLUDED_PropertyDateTimeItem_H
