// ======================================================================
//
// PropertyListItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyListItem.h"

// ----------------------------------------------------------------------

#include "PropertyList.h"

// ----------------------------------------------------------------------

#include <qcombobox.h>
#include <qlistbox.h>
#include <qobjectlist.h>

// ----------------------------------------------------------------------

#include "PropertyListItem.moc"

// ----------------------------------------------------------------------

PropertyListItem::PropertyListItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName, bool e)
: PropertyItem(l, after, prop, propName)
, m_comboBox(0)
, m_oldInt(-1)
, m_editable(e)
, m_oldString("")
, m_comboStringList()
{
}

// ----------------------------------------------------------------------

QComboBox *PropertyListItem::combo()
{
	if (m_comboBox)
		return m_comboBox;

	m_comboBox = new QComboBox(m_editable, listview->viewport());
	m_comboBox->hide();
	connect(m_comboBox, SIGNAL(activated(int)), this, SLOT(setValue()));
	m_comboBox->installEventFilter(listview);

	if (m_editable)
	{
		QObjectList *ol = m_comboBox->queryList("QLineEdit");

		if (ol && ol->first())
			ol->first()->installEventFilter(listview);
		delete ol;
	}

	return m_comboBox;
}

// ----------------------------------------------------------------------

PropertyListItem::~PropertyListItem()
{
	delete (QComboBox*)m_comboBox;
	m_comboBox = 0;
}

// ----------------------------------------------------------------------

void PropertyListItem::showEditor()
{
	PropertyItem::showEditor();

	if (!m_comboBox)
	{
		combo()->blockSignals(true);
		combo()->clear();
		combo()->insertStringList(m_comboStringList);
		combo()->blockSignals(false);
	}

	placeEditor(combo());

	if (!combo()->isVisible() || !combo()->hasFocus())
	{
		combo()->show();
		setFocus(combo());
	}
}

// ----------------------------------------------------------------------

void PropertyListItem::hideEditor()
{
	PropertyItem::hideEditor();
	combo()->hide();
}

// ----------------------------------------------------------------------

void PropertyListItem::setValue(const QVariant &v)
{
	WARNING(!m_comboBox, ("ComboBox not set!"));

	bool found = false;
	QString newValue = v.toString();

	for (uint i = 0; i < combo()->listBox()->count(); ++i)
	{
		if (combo()->listBox()->item(i)->text().lower() == newValue.lower())
		{
			combo()->setCurrentItem(i);
			found = true;
			break;
		}
	}

	WARNING(!found, ("Value not found in string list [%s]", newValue.ascii()));

	setText(1, newValue);
	PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

void PropertyListItem::setValue()
{
	if (!m_comboBox)
		return;

	//-- When the combo box is activated, update the string list with
	//--   value from the combo itself. This allows editabled combos
	//--   to remember values entered by the user?

	m_comboStringList.clear();
	for (uint i = 0; i < combo()->listBox()->count(); ++i)
		m_comboStringList << combo()->listBox()->item(i)->text();

	setValue(combo()->currentText());

	notifyValueChange();

	m_oldInt = currentIntItem();
	m_oldString = currentItem();
}

// ----------------------------------------------------------------------

void PropertyListItem::setCurrentItem(const QString &s)
{
	if (m_comboBox && currentItem().lower() == s.lower())
		return;

	if (!m_comboBox)
	{
		combo()->blockSignals(true);
		combo()->clear();
		combo()->insertStringList(m_comboStringList);
		combo()->blockSignals(false);
	}

	setValue(s);

	m_oldInt = currentIntItem();
	m_oldString = currentItem();
}

// ----------------------------------------------------------------------

void PropertyListItem::addItem(const QString &s)
{
	combo()->insertItem(s);
	m_comboStringList.append(s);
}

// ----------------------------------------------------------------------

void PropertyListItem::setCurrentItem(int i)
{
	if (m_comboBox && i == combo()->currentItem())
		return;
	
	if (!m_comboBox)
	{
		combo()->blockSignals(true);
		combo()->clear();
		combo()->insertStringList(m_comboStringList);
		combo()->blockSignals(false);
	}

	combo()->setCurrentItem(i);
	setValue(combo()->currentText());

	m_oldInt = currentIntItem();
	m_oldString = currentItem();
}

// ----------------------------------------------------------------------

QString PropertyListItem::currentItem() const
{
	return const_cast<PropertyListItem*>(this)->combo()->currentText();
}

// ----------------------------------------------------------------------

int PropertyListItem::currentIntItem() const
{
	return const_cast<PropertyListItem*>(this)->combo()->currentItem();
}

// ----------------------------------------------------------------------

int PropertyListItem::currentIntItemFromObject() const
{
	return m_oldInt;
}

// ----------------------------------------------------------------------

QString PropertyListItem::currentItemFromObject() const
{
	return m_oldString;
}

// ======================================================================
