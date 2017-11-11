// ======================================================================
//
// PropertyLayoutItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyList.h"
#include "PropertyLayoutItem.h"

// ----------------------------------------------------------------------

#include <qobjectlist.h>
#include <qspinbox.h>

// ----------------------------------------------------------------------

#include "PropertyLayoutItem.moc"

// ----------------------------------------------------------------------

PropertyLayoutItem::PropertyLayoutItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName)
: PropertyItem(l, after, prop, propName)
{
	spinBx = 0;
}

// ----------------------------------------------------------------------

PropertyLayoutItem::~PropertyLayoutItem()
{
	delete (QSpinBox*)spinBx;
	spinBx = 0;
}

// ----------------------------------------------------------------------

QSpinBox* PropertyLayoutItem::spinBox()
{
	if (spinBx)
		return spinBx;
	spinBx = new QSpinBox(-1, INT_MAX, 1, listview->viewport());
	spinBx->setSpecialValueText(tr("default"));
	spinBx->hide();
	spinBx->installEventFilter(listview);
	QObjectList *ol = spinBx->queryList("QLineEdit");
	if (ol && ol->first())
		ol->first()->installEventFilter(listview);
	delete ol;
	connect(spinBx, SIGNAL(valueChanged(int)),
		this, SLOT(setValue()));
	return spinBx;
}

// ----------------------------------------------------------------------

void PropertyLayoutItem::showEditor()
{
	PropertyItem::showEditor();
	if (!spinBx)
	{
		spinBox()->blockSignals(TRUE);
		spinBox()->setValue(value().toInt());
		spinBox()->blockSignals(TRUE);
	}
	placeEditor(spinBox());
	if (!spinBox()->isVisible() || !spinBox()->hasFocus())
	{
		spinBox()->show();
		setFocus(spinBox());
	}
}

// ----------------------------------------------------------------------

void PropertyLayoutItem::hideEditor()
{
	PropertyItem::hideEditor();
	spinBox()->hide();
}

// ----------------------------------------------------------------------

void PropertyLayoutItem::setValue(const QVariant &v)
{
	if (spinBx)
	{
		spinBox()->blockSignals(TRUE);
		spinBox()->setValue(v.toInt());
		spinBox()->blockSignals(FALSE);
	}
	QString s = v.toString();
	if (v.toInt() == -1)
		s = spinBox()->specialValueText();
	setText(1, s);
	PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

void PropertyLayoutItem::setValue()
{
	if (!spinBx)
		return;
	PropertyItem::setValue(spinBox()->value());
	notifyValueChange();
}

// ======================================================================
