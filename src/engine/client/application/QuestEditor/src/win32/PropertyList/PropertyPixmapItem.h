// ======================================================================
//
// PropertyPixmapItem.h
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PropertyPixmapItem_H
#define INCLUDED_PropertyPixmapItem_H

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

class QHBox;
class QLabel;
class QPushButton;

// ----------------------------------------------------------------------

class PropertyPixmapItem : public QObject, public PropertyItem
{
	Q_OBJECT

public:
	enum Type { Pixmap, IconSet, Image };

	PropertyPixmapItem(PropertyList *l, PropertyItem *after, PropertyItem *prop,
		const QString &propName, Type t);
	~PropertyPixmapItem();

	virtual void showEditor();
	virtual void hideEditor();

	virtual void setValue(const QVariant &v);

	virtual bool hasCustomContents() const;
	virtual void drawCustomContents(QPainter *p, const QRect &r);

private slots:
	void getPixmap();

private:
	QGuardedPtr<QHBox> box;
	QGuardedPtr<QLabel> pixPrev;
	QPushButton *button;
	Type type;
};

// ======================================================================

#endif // INCLUDED_PropertyPixmapItem_H
