// ======================================================================
//
// PropertyColorItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyColorItem_H
#define INCLUDED_PropertyColorItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

class QFrame;
class QHBox;
class QPainter;
class QPushButton;

// ----------------------------------------------------------------------

class PropertyColorItem : public QObject, public PropertyItem
{
	Q_OBJECT

public:
	PropertyColorItem(PropertyList *l, PropertyItem *after, PropertyItem *prop,
		const QString &propName, bool children);
	~PropertyColorItem();

	virtual void createChildren();
	virtual void initChildren();

	virtual void showEditor();
	virtual void hideEditor();

	virtual void setValue(const QVariant &v);
	virtual bool hasSubItems() const;
	virtual void childValueChanged(PropertyItem *child);

	virtual bool hasCustomContents() const;
	virtual void drawCustomContents(QPainter *p, const QRect &r);

private slots:
	void getColor();

private:
	QGuardedPtr<QHBox> box;
	QGuardedPtr<QFrame> colorPrev;
	QGuardedPtr<QPushButton> button;
	bool withChildren;
};

// ======================================================================

#endif // INCLUDED_PropertyColorItem_H
