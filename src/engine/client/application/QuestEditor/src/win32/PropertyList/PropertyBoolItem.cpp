// ======================================================================
//
// PropertyBoolItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyList.h"
#include "PropertyBoolItem.h"

// ----------------------------------------------------------------------

#include <qcombobox.h>

// ----------------------------------------------------------------------

#include "PropertyBoolItem.moc"

// ----------------------------------------------------------------------

PropertyBoolItem::PropertyBoolItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName)
: PropertyItem(l, after, prop, propName)
{
	comb = 0;
}

// ----------------------------------------------------------------------

QComboBox *PropertyBoolItem::combo()
{
	if (comb)
		return comb;
	
	comb = new QComboBox(FALSE, listview->viewport());
	comb->hide();
	comb->insertItem(tr("False"));
	comb->insertItem(tr("True"));

	connect(comb, SIGNAL(activated(int)), this, SLOT(setValue()));
	comb->installEventFilter(listview);
	
	return comb;
}

// ----------------------------------------------------------------------

PropertyBoolItem::~PropertyBoolItem()
{
	delete (QComboBox*)comb;
	comb = 0;
}

// ----------------------------------------------------------------------

void PropertyBoolItem::toggle()
{
	bool b = value().toBool();
	setValue(QVariant(!b, 0));
	setValue();
}

// ----------------------------------------------------------------------

void PropertyBoolItem::showEditor()
{
	PropertyItem::showEditor();
	if (!comb)
	{
		combo()->blockSignals(TRUE);
		if (value().toBool())
			combo()->setCurrentItem(1);
		else
			combo()->setCurrentItem(0);
		combo()->blockSignals(FALSE);
	}

	placeEditor(combo());
	
	if (!combo()->isVisible()  || !combo()->hasFocus())
	{
		combo()->show();
		setFocus(combo());
	}
}

// ----------------------------------------------------------------------

void PropertyBoolItem::hideEditor()
{
	PropertyItem::hideEditor();
	combo()->hide();
}

// ----------------------------------------------------------------------

void PropertyBoolItem::setValue(const QVariant &v)
{
	if ((!hasSubItems() || !isOpen()) && value() == v)
		return;

	if (comb)
	{
		combo()->blockSignals(TRUE);
		if (v.toBool())
			combo()->setCurrentItem(1);
		else
			combo()->setCurrentItem(0);
		combo()->blockSignals(FALSE);
	}

	QString tmp = tr("True");
	if (!v.toBool())
		tmp = tr("False");

	setText(1, tmp);
	PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

void PropertyBoolItem::setValue()
{
	if (!comb)
		return;

	setText(1, combo()->currentText());
	bool b = combo()->currentItem() == 0 ? (bool)FALSE : (bool)TRUE;
	PropertyItem::setValue(QVariant(b, 0));
	notifyValueChange();
}

// ======================================================================
