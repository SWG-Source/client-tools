// ======================================================================
//
// PropertyLayoutItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyLayoutItem_H
#define INCLUDED_PropertyLayoutItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

class QSpinBox;

// ----------------------------------------------------------------------

class PropertyLayoutItem : public QObject, public PropertyItem
{
	Q_OBJECT

public:
	PropertyLayoutItem(PropertyList *l, PropertyItem *after, PropertyItem *prop,
		const QString &propName);
	~PropertyLayoutItem();

	virtual void showEditor();
	virtual void hideEditor();

	virtual void setValue(const QVariant &v);

private slots:
	void setValue();

private:
	QSpinBox *spinBox();
	QGuardedPtr<QSpinBox> spinBx;
};

// ======================================================================

#endif // INCLUDED_PropertyLayoutItem_H
