// ======================================================================
//
// PropertyFontItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyFontItem_H
#define INCLUDED_PropertyFontItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

class QLineEdit;
class QPushButton;
class QHBox;

// ----------------------------------------------------------------------

class PropertyFontItem : public QObject, public PropertyItem
{
	Q_OBJECT

public:
	PropertyFontItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName);
	~PropertyFontItem();

	virtual void createChildren();
	virtual void initChildren();

	virtual void showEditor();
	virtual void hideEditor();

	virtual void setValue(const QVariant &v);
	virtual bool hasSubItems() const;
	virtual void childValueChanged(PropertyItem *child);

private slots:
	void getFont();

private:
	QGuardedPtr<QLineEdit> lined;
	QGuardedPtr<QPushButton> button;
	QGuardedPtr<QHBox> box;
};

// ======================================================================

#endif // INCLUDED_PropertyFontItem_H
