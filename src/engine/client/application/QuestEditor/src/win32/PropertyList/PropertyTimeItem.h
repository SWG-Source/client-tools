// ======================================================================
//
// PropertyTimeItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyTimeItem_H
#define INCLUDED_PropertyTimeItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

class QTimeEdit;

// ----------------------------------------------------------------------

class PropertyTimeItem : public QObject, public PropertyItem
{
	Q_OBJECT //lint !e830 !e1511 !e1516 !e1924

public:
	PropertyTimeItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName);
	~PropertyTimeItem();

	virtual void showEditor();
	virtual void hideEditor();

	virtual void setValue(const QVariant &v);

private slots:
	void setValue();

private:
	QTimeEdit *lined();
	QGuardedPtr<QTimeEdit> lin;
};

// ======================================================================

#endif // INCLUDED_PropertyTimeItem_H
