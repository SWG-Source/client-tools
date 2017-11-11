// ======================================================================
//
// PropertyKeySequenceItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyList.h"
#include "PropertyKeySequenceItem.h"

// ----------------------------------------------------------------------

#include <qhbox.h>
#include <qkeysequence.h>
#include <qlineedit.h>

// ----------------------------------------------------------------------

#include "PropertyKeySequenceItem.moc"

// ----------------------------------------------------------------------

PropertyKeySequenceItem::PropertyKeySequenceItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName)
: PropertyItem(l, after, prop, propName)
, k1(0)
, k2(0)
, k3(0)
, k4(0)
, num(0)
, mouseEnter(FALSE)
{
	box = new QHBox(listview->viewport());
	box->hide();
	sequence = new QLineEdit(box);
	connect(sequence, SIGNAL(textChanged(const QString &)),
		this, SLOT(setValue()));
	sequence->installEventFilter(this);
}

// ----------------------------------------------------------------------

PropertyKeySequenceItem::~PropertyKeySequenceItem()
{
	delete (QHBox*)box;
}

// ----------------------------------------------------------------------

void PropertyKeySequenceItem::showEditor()
{
	PropertyItem::showEditor();
	placeEditor(box);
	if (!box->isVisible())
	{
		box->show();
		sequence->setFocus();
	}
}

// ----------------------------------------------------------------------

void PropertyKeySequenceItem::hideEditor()
{
	PropertyItem::hideEditor();
	box->hide();
}

// ----------------------------------------------------------------------

bool PropertyKeySequenceItem::eventFilter(QObject *o, QEvent *e)
{
	Q_UNUSED(o);
	if (e->type() == QEvent::KeyPress)
	{
		QKeyEvent *k = (QKeyEvent *)e;
		if (!mouseEnter &&
			(k->key() == QObject::Key_Up ||
			k->key() == QObject::Key_Down))
			return FALSE;
		handleKeyEvent(k);
		return TRUE;
	} else if ((e->type() == QEvent::FocusIn) ||
		(e->type() == QEvent::MouseButtonPress))
	{
		mouseEnter = (listview->lastEvent() == PropertyList::MouseEvent) ||
			(e->type() == QEvent::MouseButtonPress);
		return TRUE;
	}
	
	// Lets eat accelerators now..
	if (e->type() == QEvent::Accel ||
		e->type() == QEvent::AccelOverride  ||
		e->type() == QEvent::KeyRelease)
		return TRUE;
	return FALSE;
}

// ----------------------------------------------------------------------

void PropertyKeySequenceItem::handleKeyEvent(QKeyEvent *e)
{
	int nextKey = e->key();
	
	if (num > 3 ||
		nextKey == QObject::Key_Control ||
		nextKey == QObject::Key_Shift ||
		nextKey == QObject::Key_Meta ||
		nextKey == QObject::Key_Alt)
		return;
	
	nextKey |= translateModifiers(e->state());
	switch(num)
	{
	case 0:
		k1 = nextKey;
		break;
	case 1:
		k2 = nextKey;
		break;
	case 2:
		k3 = nextKey;
		break;
	case 3:
		k4 = nextKey;
		break;
	default:
		break;
	}
	num++;
	QKeySequence ks(k1, k2, k3, k4);
	sequence->setText(ks);
}

// ----------------------------------------------------------------------

int PropertyKeySequenceItem::translateModifiers(int state)
{
	int result = 0;
	if (state & QObject::ShiftButton)
		result |= QObject::SHIFT;
	if (state & QObject::ControlButton)
		result |= QObject::CTRL;
	if (state & QObject::MetaButton)
		result |= QObject::META;
	if (state & QObject::AltButton)
		result |= QObject::ALT;
	return result;
}

// ----------------------------------------------------------------------

void PropertyKeySequenceItem::setValue()
{
	QVariant v;
	v = QVariant(QKeySequence(sequence->text()));
	if (v.toString().isNull())
		return;
	setText(1, sequence->text());
	PropertyItem::setValue(v);
	if (sequence->hasFocus())
		notifyValueChange();
	setChanged(TRUE);
}

// ----------------------------------------------------------------------

void PropertyKeySequenceItem::setValue(const QVariant &v)
{
	QKeySequence ks = v.toKeySequence();
	if (sequence)
	{
		sequence->setText(ks);
	}
	num = ks.count();
	k1 = ks[0];
	k2 = ks[1];
	k3 = ks[2];
	k4 = ks[3];
	setText(1, ks);
	PropertyItem::setValue(v);
}

// ======================================================================
