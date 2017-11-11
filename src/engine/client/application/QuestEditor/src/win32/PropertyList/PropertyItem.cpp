// ======================================================================
//
// PropertyItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyItem.h"

// ----------------------------------------------------------------------

#include "PropertyList.h"

// ----------------------------------------------------------------------

#include <qapplication.h>
#include <qhbox.h>
#include <qheader.h>
#include <qimage.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qstyle.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

// ----------------------------------------------------------------------

#include "resetproperty.xpm"

// ----------------------------------------------------------------------

static QColor backColor1(250, 248, 235);
static QColor backColor2(255, 255, 255);
static QColor selectedBack(230, 230, 230);

// ----------------------------------------------------------------------

PropertyItem::PropertyItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName)
: QListViewItem(l, after)
, listview(l)
, property(prop)
, propertyName(propName)
, valueName()
, whatsThisText(cs_defaultInfoText)
{
	setSelectable(FALSE);
	open = FALSE;
	setText(0, propertyName);
	changed = FALSE;
	setText(1, "");
	resetButton = 0;
}

// ----------------------------------------------------------------------

PropertyItem::~PropertyItem()
{
	if (resetButton)
		delete resetButton->parentWidget();
	resetButton = 0;
}

// ----------------------------------------------------------------------

void PropertyItem::toggle()
{
}

// ----------------------------------------------------------------------

void PropertyItem::updateBackColor()
{
	if (itemAbove() && this != listview->firstChild())
	{
		if (((PropertyItem*)itemAbove())->backColor == backColor1)
			backColor = backColor2;
		else
			backColor = backColor1;
	} else
	{
		backColor = backColor1;
	}
	if (listview->firstChild() == this)
		backColor = backColor1;
}

// ----------------------------------------------------------------------

QColor PropertyItem::backgroundColor()
{
	updateBackColor();
	if ((QListViewItem*)this == listview->currentItem())
		return selectedBack;
	return backColor;
}

// ----------------------------------------------------------------------

void PropertyItem::createChildren()
{
}

// ----------------------------------------------------------------------

void PropertyItem::initChildren()
{
}

// ----------------------------------------------------------------------

void PropertyItem::removeChildren()
{
	//-- remove all the children first
	for (int i=0; i < childCount(); ++i)
		child(i)->removeChildren();

	children.setAutoDelete(TRUE);
	children.clear();
	children.setAutoDelete(FALSE);
}

// ----------------------------------------------------------------------

void PropertyItem::paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align)
{
	QColorGroup g(cg);
	g.setColor(QColorGroup::Base, backgroundColor());
	g.setColor(QColorGroup::Foreground, Qt::black);
	g.setColor(QColorGroup::Text, Qt::black);
	int indent = 0;
	if (column == 0)
	{
		indent = 20 + (property ? 20 : 0);
		p->fillRect(0, 0, width, height(), backgroundColor());
		p->save();
		p->translate(indent, 0);
	}
	
	if (isChanged() && column == 0)
	{
		p->save();
		QFont f = p->font();
		f.setBold(TRUE);
		p->setFont(f);
	}
	
	if (!hasCustomContents() || column != 1)
	{
		QListViewItem::paintCell(p, g, column, width - indent, align );
	}
	else
	{
		p->fillRect(0, 0, width, height(), backgroundColor());
		drawCustomContents(p, QRect(0, 0, width, height()));
	}
	
	if (isChanged() && column == 0)
		p->restore();
	if (column == 0)
		p->restore();
	if (hasSubItems() && column == 0)
	{
		p->save();
		p->setPen(cg.foreground());
		p->setBrush(cg.base());
		p->drawRect(5, height() / 2 - 4, 9, 9);
		p->drawLine(7, height() / 2, 11, height() / 2);
		if (!isOpen())
			p->drawLine(9, height() / 2 - 2, 9, height() / 2 + 2);
		p->restore();
	}
	p->save();
	p->setPen(QPen(cg.dark(), 1));
	p->drawLine(0, height() - 1, width, height() - 1);
	p->drawLine(width - 1, 0, width - 1, height());
	p->restore();
	
	if (listview->currentItem() == this && column == 0 &&
		!listview->hasFocus() && !listview->viewport()->hasFocus())
		paintFocus(p, cg, QRect(0, 0, width, height()));
}

// ----------------------------------------------------------------------

void PropertyItem::paintBranches(QPainter * p, const QColorGroup & cg,
																 int w, int y, int h)
{
	QColorGroup g(cg);
	g.setColor(QColorGroup::Base, backgroundColor());
	QListViewItem::paintBranches(p, g, w, y, h);
}

// ----------------------------------------------------------------------

void PropertyItem::paintFocus(QPainter *p, const QColorGroup &cg, const QRect &r)
{
	p->save();
	QApplication::style().drawPrimitive(QStyle::PE_Panel, p, r, cg,
		QStyle::Style_Sunken, QStyleOption(1,1));
	p->restore();
}

// ----------------------------------------------------------------------

bool PropertyItem::hasSubItems() const
{
	return FALSE;
}

// ----------------------------------------------------------------------

PropertyItem *PropertyItem::propertyParent() const
{
	return property;
}

// ----------------------------------------------------------------------

bool PropertyItem::isOpen() const
{
	return open;
}

// ----------------------------------------------------------------------

void PropertyItem::setOpen(bool b)
{
	if (b == open)
		return;
	open = b;
	
	if (!open)
	{
		removeChildren();
		qApp->processEvents();
		listview->updateEditorSize();
		return;
	}
	
	createChildren();
	initChildren();
	qApp->processEvents();
	listview->updateEditorSize();
}

// ----------------------------------------------------------------------

void PropertyItem::showEditor()
{
	createResetButton();
	resetButton->parentWidget()->show();
}

// ----------------------------------------------------------------------

void PropertyItem::hideEditor()
{
	createResetButton();
	resetButton->parentWidget()->hide();
}

// ----------------------------------------------------------------------

void PropertyItem::setValue(const QVariant &v)
{
	val = v;
}

// ----------------------------------------------------------------------

QVariant PropertyItem::value() const
{
	return val;
}

// ----------------------------------------------------------------------

bool PropertyItem::isChanged() const
{
	return changed;
}

// ----------------------------------------------------------------------

void PropertyItem::setChanged(bool b, bool updateDb)
{
	UNREF(updateDb);
	
	if (propertyParent())
		return;
	if (changed == b)
		return;
	changed = b;
	repaint();
	
	updateResetButtonState();
}

// ----------------------------------------------------------------------

QString PropertyItem::name() const
{
	return propertyName;
}

// ----------------------------------------------------------------------

void PropertyItem::createResetButton()
{
	if (resetButton)
	{
		resetButton->parentWidget()->lower();
		return;
	}
	QHBox *hbox = new QHBox(listview->viewport());
	hbox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	hbox->setLineWidth(1);
	resetButton = new QPushButton(hbox);
	resetButton->setPixmap(QPixmap(resetproperty_xpm));
	resetButton->setFixedWidth(resetButton->sizeHint().width());
	hbox->layout()->setAlignment(Qt::AlignRight);
	listview->addChild(hbox);
	hbox->hide();
	QObject::connect(resetButton, SIGNAL(clicked()), listview, SLOT(resetProperty()));
	QToolTip::add(resetButton,
		PropertyList::tr("Reset the property to its default value"));
	QWhatsThis::add(resetButton,
		PropertyList::tr("Click this button to reset the property to its default value"));
	
	updateResetButtonState();
}

// ----------------------------------------------------------------------

void PropertyItem::updateResetButtonState()
{
	if (!resetButton)
		return;
	
	if (propertyParent())
		resetButton->setEnabled(FALSE);
	else
		resetButton->setEnabled(isChanged());
}

// ----------------------------------------------------------------------

void PropertyItem::placeEditor(QWidget *w)
{
	createResetButton();
	QRect r = listview->itemRect(this);
	if (!r.size().isValid())
	{
		listview->ensureItemVisible(this);
#if defined(Q_WS_WIN)
		listview->repaintContents(FALSE);
#endif
		r = listview->itemRect(this);
	}
	r.setX(listview->header()->sectionPos(1));
	r.setWidth(listview->header()->sectionSize(1) - 1);
	r.setWidth(r.width() - resetButton->width() - 2);
	r = QRect(listview->viewportToContents(r.topLeft()), r.size());
	w->resize(r.size());
	listview->moveChild(w, r.x(), r.y());
	resetButton->parentWidget()->resize(resetButton->sizeHint().width() + 10, r.height());
	listview->moveChild(resetButton->parentWidget(), r.x() + r.width() - 8, r.y());
	resetButton->setFixedHeight(QMAX(0, r.height() - 3));
}

// ----------------------------------------------------------------------

void PropertyItem::notifyValueChange()
{
	if (!propertyParent())
	{
		listview->valueChanged(this);
		setChanged(TRUE);
		if (hasSubItems())
			initChildren();
	}
	else
	{
		propertyParent()->childValueChanged(this);
		setChanged(TRUE);
	}
}

// ----------------------------------------------------------------------

void PropertyItem::childValueChanged(PropertyItem *)
{
}

// ----------------------------------------------------------------------

void PropertyItem::addChild(PropertyItem *i)
{
	children.append(i);
}

// ----------------------------------------------------------------------

int PropertyItem::childCount() const
{
	return children.count();
}

// ----------------------------------------------------------------------

PropertyItem *PropertyItem::child(int i) const
{
	// ARRRRRRRRG
	return ((PropertyItem*)this)->children.at(i);
}

// ----------------------------------------------------------------------

bool PropertyItem::hasCustomContents() const
{
	return FALSE;
}

// ----------------------------------------------------------------------

void PropertyItem::drawCustomContents(QPainter *, const QRect &)
{
}

// ----------------------------------------------------------------------

QString PropertyItem::currentItem() const
{
	return QString::null;
}

// ----------------------------------------------------------------------

int PropertyItem::currentIntItem() const
{
	return -1;
}

// ----------------------------------------------------------------------

void PropertyItem::setCurrentItem(const QString &)
{
}

// ----------------------------------------------------------------------

void PropertyItem::setCurrentItem(int)
{
}

// ----------------------------------------------------------------------

int PropertyItem::currentIntItemFromObject() const
{
	return -1;
}

// ----------------------------------------------------------------------

QString PropertyItem::currentItemFromObject() const
{
	return QString::null;
}

// ----------------------------------------------------------------------

void PropertyItem::setFocus(QWidget *w)
{
	if (listview && listview->hasFocus() && w)
		w->setFocus();
}

// ----------------------------------------------------------------------

void PropertyItem::setText(int col, const QString &t)
{
	QString txt(t);
	if (col == 1)
		txt = txt.replace("\n", " ");
	QListViewItem::setText(col, txt);
}

// ======================================================================
