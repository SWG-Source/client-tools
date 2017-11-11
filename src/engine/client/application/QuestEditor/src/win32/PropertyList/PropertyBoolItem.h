// ======================================================================
//
// PropertyBoolItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyBoolItem_H
#define INCLUDED_PropertyBoolItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

class QComboBox;

// ----------------------------------------------------------------------

class PropertyBoolItem : public QObject, public PropertyItem
{
	Q_OBJECT

public:
	PropertyBoolItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName);
	~PropertyBoolItem();

	virtual void showEditor();
	virtual void hideEditor();

	virtual void setValue(const QVariant &v);
	virtual void toggle();

private slots:
	void setValue();

private:
	QComboBox *combo();
	QGuardedPtr<QComboBox> comb;
};

// ======================================================================

#endif // INCLUDED_PropertyBoolItem_H
