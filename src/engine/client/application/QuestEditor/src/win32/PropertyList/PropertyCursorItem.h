// ======================================================================
//
// PropertyCursorItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyCursorItem_H
#define INCLUDED_PropertyCursorItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

class QComboBox;

// ----------------------------------------------------------------------

class PropertyCursorItem : public QObject, public PropertyItem
{
	Q_OBJECT

public:
	PropertyCursorItem(PropertyList *l, PropertyItem *after, PropertyItem *prop,
		const QString &propName);
	~PropertyCursorItem();

	virtual void showEditor();
	virtual void hideEditor();

	virtual void setValue(const QVariant &v);

private slots:
	void setValue();

private:
	QComboBox *combo();
	QGuardedPtr<QComboBox> comb;
};

// ======================================================================

#endif // INCLUDED_PropertyCursorItem_H
