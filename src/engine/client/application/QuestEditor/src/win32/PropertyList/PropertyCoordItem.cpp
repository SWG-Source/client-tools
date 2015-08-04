// ======================================================================
//
// PropertyCoordItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyCoordItem.h"
#include "PropertyIntItem.h"
#include "PropertyList.h"

// ----------------------------------------------------------------------

#include <qlineedit.h>

// ----------------------------------------------------------------------

#include "PropertyCoordItem.moc"

// ----------------------------------------------------------------------

PropertyCoordItem::PropertyCoordItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName, Type t)
: PropertyItem(l, after, prop, propName)
, typ(t)
{
	lin = 0;
}

// ----------------------------------------------------------------------

QLineEdit *PropertyCoordItem::lined()
{
	if (lin)
		return lin;
	lin = new QLineEdit(listview->viewport());
	lin->setReadOnly(TRUE);
	lin->installEventFilter(listview);
	lin->hide();
	return lin;
}

// ----------------------------------------------------------------------

void PropertyCoordItem::createChildren()
{
	PropertyItem *i = this;
	if (typ == Rect || typ == Point)
	{
		i = new PropertyIntItem(listview, i, this, tr("x"), TRUE);
		addChild(i);
		i = new PropertyIntItem(listview, i, this, tr("y"), TRUE);
		addChild(i);
	}
	if (typ == Rect || typ == Size)
	{
		i = new PropertyIntItem(listview, i, this, tr("width"), TRUE);
		addChild(i);
		i = new PropertyIntItem(listview, i, this, tr("height"), TRUE);
		addChild(i);
	}
}

// ----------------------------------------------------------------------

void PropertyCoordItem::initChildren()
{
	PropertyItem *item = 0;
	for (int i = 0; i < childCount(); ++i)
	{
		item = PropertyItem::child(i);
		if (item->name() == tr("x"))
		{
			if (typ == Rect)
				item->setValue(val.toRect().x());
			else if (typ == Point)
				item->setValue(val.toPoint().x());
		}
		else if (item->name() == tr("y"))
		{
			if (typ == Rect)
				item->setValue(val.toRect().y());
			else if (typ == Point)
				item->setValue(val.toPoint().y());
		}
		else if (item->name() == tr("width"))
		{
			if (typ == Rect)
				item->setValue(val.toRect().width());
			else if (typ == Size)
				item->setValue(val.toSize().width());
		}
		else if (item->name() == tr("height")) {
			if (typ == Rect)
				item->setValue(val.toRect().height());
			else if (typ == Size)
				item->setValue(val.toSize().height());
		}
	}
}

// ----------------------------------------------------------------------

PropertyCoordItem::~PropertyCoordItem()
{
	delete (QLineEdit*)lin;
	lin = 0;
}

// ----------------------------------------------------------------------

void PropertyCoordItem::showEditor()
{
	PropertyItem::showEditor();
	if (!lin)
		lined()->setText(text(1));

	placeEditor(lined());
	if (!lined()->isVisible() || !lined()->hasFocus())
	{
		lined()->show();
		setFocus(lined());
	}
}

// ----------------------------------------------------------------------

void PropertyCoordItem::hideEditor()
{
	PropertyItem::hideEditor();
	lined()->hide();
}

// ----------------------------------------------------------------------

void PropertyCoordItem::setValue(const QVariant &v)
{
	if ((!hasSubItems() || !isOpen()) && value() == v)
		return;
	
	QString s;
	if (typ == Rect)
	{
		s = "[ " + QString::number(v.toRect().x()) + ", " + QString::number(v.toRect().y()) + ", " +
		QString::number(v.toRect().width()) + ", " + QString::number(v.toRect().height()) + " ]";
	}
	else if (typ == Point)
	{
		s = "[ " + QString::number(v.toPoint().x()) + ", " +
		QString::number(v.toPoint().y()) + " ]";
	}
	else if (typ == Size)
	{
		s = "[ " + QString::number(v.toSize().width()) + ", " +
		QString::number(v.toSize().height()) + " ]";
	}
	setText(1, s);

	if (lin)
		lined()->setText(s);
	PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

bool PropertyCoordItem::hasSubItems() const
{
	return TRUE;
}

// ----------------------------------------------------------------------

void PropertyCoordItem::childValueChanged(PropertyItem *child)
{
	if (typ == Rect)
	{
		QRect r = value().toRect();
		if (child->name() == tr("x"))
			r.moveBy(-r.x() + child->value().toInt(), 0);
		else if (child->name() == tr("y"))
			r.moveBy(0, -r.y() + child->value().toInt());
		else if (child->name() == tr("width"))
			r.setWidth(child->value().toInt());
		else if (child->name() == tr("height"))
			r.setHeight(child->value().toInt());
		setValue(r);
	}
	else if (typ == Point)
	{
		QPoint r = value().toPoint();
		if (child->name() == tr("x"))
			r.setX(child->value().toInt());
		else if (child->name() == tr("y"))
			r.setY(child->value().toInt());
		setValue(r);
	}
	else if (typ == Size)
	{
		QSize r = value().toSize();
		if (child->name() == tr("width"))
			r.setWidth(child->value().toInt());
		else if (child->name() == tr("height"))
			r.setHeight(child->value().toInt());
		setValue(r);
	}
	
	notifyValueChange();
}

// ======================================================================
