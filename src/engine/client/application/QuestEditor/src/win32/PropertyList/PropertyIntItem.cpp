// ======================================================================
//
// PropertyIntItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyList.h"
#include "PropertyIntItem.h"

// ----------------------------------------------------------------------

#include <qobjectlist.h>

// ----------------------------------------------------------------------

#include "PropertyIntItem.moc"

// ----------------------------------------------------------------------

PropertyIntItem::PropertyIntItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName, bool s)
: PropertyItem(l, after, prop, propName)
, signedValue(s)
, m_minValue(-INT_MAX)
, m_maxValue(INT_MAX)
{
	spinBx = 0;
}

// ----------------------------------------------------------------------

QSpinBox *PropertyIntItem::spinBox()
{
	if (spinBx)
		return spinBx;
	
	if (signedValue)
		spinBx = new QSpinBox(m_minValue, m_maxValue, 1, listview->viewport());
	else
		spinBx = new QSpinBox(0, m_maxValue, 1, listview->viewport());
	
	spinBx->hide();
	spinBx->installEventFilter(listview);
	
	QObjectList *ol = spinBx->queryList("QLineEdit");
	if (ol && ol->first())
		ol->first()->installEventFilter(listview);
	delete ol;
	
	connect(spinBx, SIGNAL(valueChanged(int)), this, SLOT(setValue()));
	
	return spinBx;
}

// ----------------------------------------------------------------------

PropertyIntItem::~PropertyIntItem()
{
	delete (QSpinBox*)spinBx;
	spinBx = 0;
}

// ----------------------------------------------------------------------

void PropertyIntItem::showEditor()
{
	PropertyItem::showEditor();
	
	if (!spinBx)
	{
		spinBox()->blockSignals(TRUE);
		if (signedValue)
			spinBox()->setValue(value().toInt());
		else
			spinBox()->setValue(value().toUInt());
		spinBox()->blockSignals(FALSE);
	}
	
	placeEditor(spinBox());
	
	if (!spinBox()->isVisible()  || !spinBox()->hasFocus() )
	{
		spinBox()->show();
		setFocus(spinBox());
	}
}

// ----------------------------------------------------------------------

void PropertyIntItem::hideEditor()
{
	PropertyItem::hideEditor();
	if (spinBx)
	{
		spinBox()->hide();
	}
}

// ----------------------------------------------------------------------

void PropertyIntItem::setValue(const QVariant &v)
{
	if ((!hasSubItems() || !isOpen())
		&& value() == v)
		return;
	
	if (spinBx)
	{
		spinBox()->blockSignals(TRUE);
		if (signedValue)
			spinBox()->setValue(v.toInt());
		else
			spinBox()->setValue(v.toUInt());
		spinBox()->blockSignals(FALSE);
	}
	
	if (signedValue)
		setText(1, QString::number(v.toInt()));
	else
		setText(1, QString::number(v.toUInt()));
	
	PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

void PropertyIntItem::setValue()
{
	if (!spinBx)
		return;
	
	setText(1, QString::number(spinBox()->value()));
	
	if (signedValue)
		PropertyItem::setValue(spinBox()->value());
	else
		PropertyItem::setValue((uint)spinBox()->value());
	
	notifyValueChange();
}

// ----------------------------------------------------------------------

void PropertyIntItem::setMinValue(int minValue)
{
	m_minValue = minValue;
}

// ----------------------------------------------------------------------

void PropertyIntItem::setMaxValue(int maxValue)
{
	m_maxValue = maxValue;
}

// ======================================================================
