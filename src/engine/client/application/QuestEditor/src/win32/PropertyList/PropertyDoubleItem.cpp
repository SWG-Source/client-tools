// ======================================================================
//
// PropertyDoubleItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyList.h"
#include "PropertyDoubleItem.h"

// ----------------------------------------------------------------------

#include <qlineedit.h>
#include <qvalidator.h>

// ----------------------------------------------------------------------

#include <math.h>

// ----------------------------------------------------------------------

#include "PropertyDoubleItem.moc"

// ----------------------------------------------------------------------

PropertyDoubleItem::PropertyDoubleItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName)
: PropertyItem(l, after, prop, propName)
, m_doubleValidator(0)
, m_minValue(-HUGE_VAL)
, m_maxValue(HUGE_VAL)
{
	lin = 0;
}

// ----------------------------------------------------------------------

QLineEdit *PropertyDoubleItem::lined()
{
	if (lin)
		return lin;

	lin = new QLineEdit(listview->viewport());
	m_doubleValidator = new QDoubleValidator(lin, "double_validator");
	m_doubleValidator->setBottom(m_minValue);
	m_doubleValidator->setTop(m_maxValue);
	lin->setValidator(m_doubleValidator);
	
	connect(lin, SIGNAL(returnPressed()), this, SLOT(setValue()));
	connect(lin, SIGNAL(textChanged(const QString &)), this, SLOT(setValue()));

	lin->installEventFilter(listview);
	return lin;
}

// ----------------------------------------------------------------------

PropertyDoubleItem::~PropertyDoubleItem()
{
	m_doubleValidator = 0;
	delete (QLineEdit*)lin;
	lin = 0;
}

// ----------------------------------------------------------------------

void PropertyDoubleItem::showEditor()
{
	PropertyItem::showEditor();
	if (!lin)
	{
		lined()->blockSignals(TRUE);
		lined()->setText(QString::number(value().toDouble()));
		lined()->blockSignals(FALSE);
		updateBackground();
	}

	QWidget* w = lined();
	placeEditor(w);
	if (!w->isVisible() || !lined()->hasFocus())
	{
		w->show();
		setFocus(lined());
	}
}

// ----------------------------------------------------------------------

void PropertyDoubleItem::hideEditor()
{
	PropertyItem::hideEditor();
	QWidget* w = lined();
	w->hide();
}

// ----------------------------------------------------------------------

void PropertyDoubleItem::setValue(const QVariant &v)
{
	if (value() == v)
		return;
	
	if (lin)
	{
		lined()->blockSignals(TRUE);
		int oldCursorPos;
		oldCursorPos = lin->cursorPosition();
		lined()->setText(QString::number(v.toDouble()));

		if (oldCursorPos < (int)lin->text().length())
			lin->setCursorPosition(oldCursorPos);

		lined()->blockSignals(FALSE);
	}
	
	setText(1, QString::number(v.toDouble()));
	PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

void PropertyDoubleItem::setValue()
{
	setText(1, lined()->text());
	QVariant v = lined()->text().toDouble();
	PropertyItem::setValue(v);
	notifyValueChange();
	updateBackground();
}

// ----------------------------------------------------------------------

void PropertyDoubleItem::setMinValue(float minValue)
{
	m_minValue = minValue;
}

// ----------------------------------------------------------------------

void PropertyDoubleItem::setMaxValue(float maxValue)
{
	m_maxValue = maxValue;
}

// ----------------------------------------------------------------------

void PropertyDoubleItem::updateBackground()
{
	if (lined()->hasAcceptableInput())
	{
		lined()->unsetPalette();
	}
	else
	{
		lined()->setPaletteBackgroundColor(QColor(255, 0, 0));
	}
}

// ======================================================================
