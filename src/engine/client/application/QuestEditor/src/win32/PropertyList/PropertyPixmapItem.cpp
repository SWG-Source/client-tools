// ======================================================================
//
// PropertyPixmapItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyList.h"
#include "PropertyPixmapItem.h"

// ----------------------------------------------------------------------

#include <qhbox.h>
#include <qimage.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qpushbutton.h>

// ----------------------------------------------------------------------

#include "PropertyPixmapItem.moc"

// ----------------------------------------------------------------------

PropertyPixmapItem::PropertyPixmapItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName, Type t)
: PropertyItem(l, after, prop, propName)
, type(t)
{
	box = new QHBox(listview->viewport());
	box->hide();
	pixPrev = new QLabel(box);
	pixPrev->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Minimum));
	pixPrev->setBackgroundColor(pixPrev->colorGroup().color(QColorGroup::Base));
	button = new QPushButton("...", box);
	button->setFixedWidth(20);
	box->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	box->setLineWidth(2);
	pixPrev->setFrameStyle(QFrame::NoFrame);
	box->installEventFilter(listview);
	connect(button, SIGNAL(clicked()),
		this, SLOT(getPixmap()));
}

// ----------------------------------------------------------------------

PropertyPixmapItem::~PropertyPixmapItem()
{
	delete (QHBox*)box;
}

// ----------------------------------------------------------------------

void PropertyPixmapItem::showEditor()
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

void PropertyPixmapItem::hideEditor()
{
	PropertyItem::hideEditor();
	box->hide();
}

// ----------------------------------------------------------------------

void PropertyPixmapItem::setValue(const QVariant &v)
{
	QString s;
	if (type == Pixmap)
		pixPrev->setPixmap(v.toPixmap());
	else if (type == IconSet)
		pixPrev->setPixmap(v.toIconSet().pixmap());
	else
		pixPrev->setPixmap(v.toImage());
	PropertyItem::setValue(v);
	repaint();
}

// ----------------------------------------------------------------------

void PropertyPixmapItem::getPixmap()
{
	QPixmap pix; //TTODO = qChoosePixmap(listview, listview->propertyEditor()->formWindow(), value().toPixmap());
	if (!pix.isNull())
	{
		if (type == Pixmap)
			setValue(pix);
		else if (type == IconSet)
			setValue(QIconSet(pix));
		else
			setValue(pix.convertToImage());
		
		notifyValueChange();
	}
}

// ----------------------------------------------------------------------

bool PropertyPixmapItem::hasCustomContents() const
{
	return TRUE;
}

// ----------------------------------------------------------------------

void PropertyPixmapItem::drawCustomContents(QPainter *p, const QRect &r)
{
	QPixmap pix;
	if (type == Pixmap)
		pix = value().toPixmap();
	else if (type == IconSet)
		pix = value().toIconSet().pixmap();
	else
		pix = value().toImage();
	
	if (!pix.isNull())
	{
		p->save();
		p->setClipRect(QRect(QPoint((int)(p->worldMatrix().dx() + r.x()),
			(int)(p->worldMatrix().dy() + r.y())),
			r.size()));
		p->drawPixmap(r.x(), r.y() + (r.height() - pix.height()) / 2, pix);
		p->restore();
	}
}

// ======================================================================
