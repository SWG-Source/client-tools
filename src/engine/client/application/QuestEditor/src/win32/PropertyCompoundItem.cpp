// ======================================================================
//
// PropertyCompoundItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "ElementPropertyEditor.h"
#include "PropertyCompoundItem.h"
#include "PropertyList/PropertyIntItem.h"
#include "PropertyList/PropertyList.h"
#include "QuestEditorConstants.h"

// ----------------------------------------------------------------------

#include <qlineedit.h>

// ----------------------------------------------------------------------

#include "PropertyCompoundItem.moc"

// ----------------------------------------------------------------------

PropertyCompoundItem::PropertyCompoundItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName, QDomElement const & configInputElement)
: PropertyItem(l, after, prop, propName)
, m_lin(0)
, m_configInputElement(configInputElement)
{
}

// ----------------------------------------------------------------------

QLineEdit *PropertyCompoundItem::lined()
{
	if (m_lin)
		return m_lin;
	m_lin = new QLineEdit(listview->viewport());
	m_lin->setReadOnly(TRUE);
	m_lin->installEventFilter(listview);
	m_lin->hide();
	return m_lin;
}

// ----------------------------------------------------------------------

void PropertyCompoundItem::createChildren()
{
	ElementPropertyEditor * propertyEditor = safe_cast<ElementPropertyEditor *>(listview);

	// For each child input element call setupInput and add item as a child to this input.
	QDomNode childData = m_configInputElement.lastChild();

	for (; !childData.isNull(); childData = childData.previousSibling())
		if (childData.nodeName() == cs_Data)
		{
			PropertyItem * childItem = this;

			propertyEditor->setupInput(childItem, childData, this);

			addChild(childItem);
		}

	setConcatenatedValue();
}

// ----------------------------------------------------------------------

PropertyCompoundItem::~PropertyCompoundItem()
{
	delete (QLineEdit*)m_lin;
	m_lin = 0;
}

// ----------------------------------------------------------------------

void PropertyCompoundItem::showEditor()
{
	PropertyItem::showEditor();
	if (!m_lin)
		lined()->setText(text(1));

	placeEditor(lined());
	if (!lined()->isVisible() || !lined()->hasFocus())
	{
		lined()->show();
		setFocus(lined());
	}
}

// ----------------------------------------------------------------------

void PropertyCompoundItem::hideEditor()
{
	PropertyItem::hideEditor();
	lined()->hide();
}

// ----------------------------------------------------------------------

void PropertyCompoundItem::setValue(const QVariant &v)
{
	if ((!hasSubItems() || !isOpen()) && value() == v)
		return;

	QString const s = v.toString();

	setText(1, s);
	lined()->setText(s);

	PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

bool PropertyCompoundItem::hasSubItems() const
{
	return TRUE;
}

// ----------------------------------------------------------------------

void PropertyCompoundItem::childValueChanged(PropertyItem * child)
{
	//-- construct this value based on the new child values
	setConcatenatedValue();

	//-- if a child is null, then do not set changes state of notify the list view
	if (child)
	{
		listview->valueChanged(child);
		child->setChanged(true);
	}

	notifyValueChange();
}

// ----------------------------------------------------------------------

void PropertyCompoundItem::notifyValueChange()
{
	//-- notify the parent control (as it may be another compound input)
	if (propertyParent())
		propertyParent()->childValueChanged(0);
}

// ----------------------------------------------------------------------

void PropertyCompoundItem::setConcatenatedValue()
{
	// Create a value by looping through each child and appending its value
	QString s = "[ ";
	PropertyItem *item = 0;

	for (int i = childCount() - 1; i >= 0 ; --i)
	{
		item = PropertyItem::child(i);
		QString v = item->value().asString();
		s += (v.isEmpty() || v.isNull() ? QString("\"\"") : v) + (i ? ", " : "");
	}
	s += " ]";

	setValue(s);
}

// ======================================================================
