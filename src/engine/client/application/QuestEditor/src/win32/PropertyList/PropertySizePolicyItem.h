// ======================================================================
//
// PropertySizePolicyItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertySizePolicyItem_H
#define INCLUDED_PropertySizePolicyItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

class QLineEdit;

// ----------------------------------------------------------------------

class PropertySizePolicyItem : public QObject, public PropertyItem
{
	Q_OBJECT

public:
	PropertySizePolicyItem(PropertyList *l, PropertyItem *after, PropertyItem *prop,
		const QString &propName);
	~PropertySizePolicyItem();

	virtual void createChildren();
	virtual void initChildren();

	virtual void showEditor();
	virtual void hideEditor();

	virtual void setValue(const QVariant &v);
	virtual bool hasSubItems() const;
	virtual void childValueChanged(PropertyItem *child);

private:
	QLineEdit *lined();
	QGuardedPtr<QLineEdit> lin;
};

// ======================================================================

#endif // INCLUDED_PropertySizePolicyItem_H
