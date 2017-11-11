// ======================================================================
//
// PropertyCursorItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyList.h"
#include "PropertyCursorItem.h"

// ----------------------------------------------------------------------

#include <qbitmap.h>
#include <qcombobox.h>
#include <qcursor.h>
#include <qpixmap.h>

// ----------------------------------------------------------------------

#include "PropertyCursorItem.moc"

// ----------------------------------------------------------------------

PropertyCursorItem::PropertyCursorItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName)
: PropertyItem(l, after, prop, propName)
{
	comb = 0;
}

// ----------------------------------------------------------------------

QComboBox *PropertyCursorItem::combo()
{
	if (comb)
		return comb;
	comb = new QComboBox(FALSE, listview->viewport());
	comb->hide();
	QBitmap cur;
	
	comb->insertItem(QPixmap::fromMimeSource("images/designer_arrow.png"), tr("Arrow"), QObject::ArrowCursor);
	comb->insertItem(QPixmap::fromMimeSource("images/designer_uparrow.png"), tr("Up-Arrow"), QObject::UpArrowCursor);
	comb->insertItem(QPixmap::fromMimeSource("images/designer_cross.png"), tr("Cross"), QObject::CrossCursor);
	comb->insertItem(QPixmap::fromMimeSource("images/designer_wait.png"), tr("Waiting"), QObject::WaitCursor);
	comb->insertItem(QPixmap::fromMimeSource("images/designer_ibeam.png"), tr("iBeam"), QObject::IbeamCursor);
	comb->insertItem(QPixmap::fromMimeSource("images/designer_sizev.png"), tr("Size Vertical"), QObject::SizeVerCursor);
	comb->insertItem(QPixmap::fromMimeSource("images/designer_sizeh.png"), tr("Size Horizontal"), QObject::SizeHorCursor);
	comb->insertItem(QPixmap::fromMimeSource("images/designer_sizef.png"), tr("Size Slash"), QObject::SizeBDiagCursor);
	comb->insertItem(QPixmap::fromMimeSource("images/designer_sizeb.png"), tr("Size Backslash"), QObject::SizeFDiagCursor);
	comb->insertItem(QPixmap::fromMimeSource("images/designer_sizeall.png"), tr("Size All"), QObject::SizeAllCursor);
	cur = QBitmap(25, 25, 1);
	cur.setMask(cur);
	comb->insertItem(cur, tr("Blank"), QObject::BlankCursor);
	comb->insertItem(QPixmap::fromMimeSource("images/designer_vsplit.png"), tr("Split Vertical"), QObject::SplitVCursor);
	comb->insertItem(QPixmap::fromMimeSource("images/designer_hsplit.png"), tr("Split Horizontal"), QObject::SplitHCursor);
	comb->insertItem(QPixmap::fromMimeSource("images/designer_hand.png"), tr("Pointing Hand"), QObject::PointingHandCursor);
	comb->insertItem(QPixmap::fromMimeSource("images/designer_no.png"), tr("Forbidden"), QObject::ForbiddenCursor);
	
	connect(comb, SIGNAL(activated(int)),
		this, SLOT(setValue()));
	comb->installEventFilter(listview);
	return comb;
}

// ----------------------------------------------------------------------

PropertyCursorItem::~PropertyCursorItem()
{
	delete (QComboBox*)comb;
}

// ----------------------------------------------------------------------

void PropertyCursorItem::showEditor()
{
	PropertyItem::showEditor();
	if (!comb)
	{
		combo()->blockSignals(TRUE);
		combo()->setCurrentItem((int)value().toCursor().shape());
		combo()->blockSignals(FALSE);
	}
	placeEditor(combo());
	if (!combo()->isVisible() || !combo()->hasFocus())
	{
		combo()->show();
		setFocus(combo());
	}
}

// ----------------------------------------------------------------------

void PropertyCursorItem::hideEditor()
{
	PropertyItem::hideEditor();
	combo()->hide();
}

// ----------------------------------------------------------------------

void PropertyCursorItem::setValue(const QVariant &v)
{
	if ((!hasSubItems() || !isOpen())
		&& value() == v)
		return;
	
	combo()->blockSignals(TRUE);
	combo()->setCurrentItem((int)v.toCursor().shape());
	combo()->blockSignals(FALSE);
	setText(1, combo()->currentText());
	PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

void PropertyCursorItem::setValue()
{
	if (!comb)
		return;
	if (QVariant(QCursor(combo()->currentItem())) == val)
		return;
	setText(1, combo()->currentText());
	PropertyItem::setValue(QCursor(combo()->currentItem()));
	notifyValueChange();
}

// ======================================================================
