// ======================================================================
//
// PropertyColorItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyIntItem.h"
#include "PropertyList.h"
#include "PropertyColorItem.h"

// ----------------------------------------------------------------------

#include <qcolordialog.h>
#include <qframe.h>
#include <qhbox.h>
#include <qpainter.h>
#include <qpushbutton.h>

// ----------------------------------------------------------------------

#include "PropertyColorItem.moc"

// ----------------------------------------------------------------------

PropertyColorItem::PropertyColorItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName, bool children)
: PropertyItem(l, after, prop, propName)
, withChildren(children)
{
	box = new QHBox(listview->viewport());
	box->hide();
	colorPrev = new QFrame(box);
	button = new QPushButton("...", box);
	button->setFixedWidth(20);
	box->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	box->setLineWidth(2);
	colorPrev->setFrameStyle(QFrame::Plain | QFrame::Box);
	colorPrev->setLineWidth(2);

	QPalette pal = colorPrev->palette();
	QColorGroup cg = pal.active();
	cg.setColor(QColorGroup::Foreground, cg.color(QColorGroup::Base));
	pal.setActive(cg);
	pal.setInactive(cg);
	pal.setDisabled(cg);

	colorPrev->setPalette(pal);
	box->installEventFilter(listview);
	connect(button, SIGNAL(clicked()), this, SLOT(getColor()));
}

// ----------------------------------------------------------------------

void PropertyColorItem::createChildren()
{
	PropertyItem *i = this;
	i = new PropertyIntItem(listview, i, this, tr("Red"), TRUE);
	addChild(i);
	i = new PropertyIntItem(listview, i, this, tr("Green"), TRUE);
	addChild(i);
	i = new PropertyIntItem(listview, i, this, tr("Blue"), TRUE);
	addChild(i);
}

// ----------------------------------------------------------------------

void PropertyColorItem::initChildren()
{
	PropertyItem *item = 0;
	for (int i = 0; i < childCount(); ++i)
	{
		item = PropertyItem::child(i);
		if (item->name() == tr("Red"))
			item->setValue(val.toColor().red());
		else if (item->name() == tr("Green"))
			item->setValue(val.toColor().green());
		else if (item->name() == tr("Blue"))
			item->setValue(val.toColor().blue());
	}
}

// ----------------------------------------------------------------------

PropertyColorItem::~PropertyColorItem()
{
	delete (QHBox*)box;
}

// ----------------------------------------------------------------------

void PropertyColorItem::showEditor()
{
	PropertyItem::showEditor();
	placeEditor(box);
	if (!box->isVisible())
	{
		box->show();
		listView()->viewport()->setFocus();
	}
}

// ----------------------------------------------------------------------

void PropertyColorItem::hideEditor()
{
	PropertyItem::hideEditor();
	box->hide();
}

// ----------------------------------------------------------------------

void PropertyColorItem::setValue(const QVariant &v)
{
	if ((!hasSubItems() || !isOpen())
		&& value() == v)
		return;
	
	QString s;
	setText(1, v.toColor().name());
	colorPrev->setBackgroundColor(v.toColor());
	PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

bool PropertyColorItem::hasSubItems() const
{
	return withChildren;
}

// ----------------------------------------------------------------------

void PropertyColorItem::childValueChanged(PropertyItem *child)
{
	QColor c(val.toColor());
	if (child->name() == tr("Red"))
		c.setRgb(child->value().toInt(), c.green(), c.blue());
	else if (child->name() == tr("Green"))
		c.setRgb(c.red(), child->value().toInt(), c.blue());
	else if (child->name() == tr("Blue"))
		c.setRgb(c.red(), c.green(), child->value().toInt());
	setValue(c);
	notifyValueChange();
}

// ----------------------------------------------------------------------

void PropertyColorItem::getColor()
{
	QColor c = QColorDialog::getColor(val.asColor(), listview);
	if (c.isValid())
	{
		setValue(c);
		notifyValueChange();
	}
}

// ----------------------------------------------------------------------

bool PropertyColorItem::hasCustomContents() const
{
	return TRUE;
}

// ----------------------------------------------------------------------

void PropertyColorItem::drawCustomContents(QPainter *p, const QRect &r)
{
	p->save();
	p->setPen(QPen(black, 1));
	p->setBrush(val.toColor());
	p->drawRect(r.x() + 2, r.y() + 2, r.width() - 5, r.height() - 5);
	p->restore();
}

// ======================================================================
