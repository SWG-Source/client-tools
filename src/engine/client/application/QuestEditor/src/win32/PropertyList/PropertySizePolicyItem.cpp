// ======================================================================
//
// PropertySizePolicyItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyIntItem.h"
#include "PropertyList.h"
#include "PropertyListItem.h"
#include "PropertySizePolicyItem.h"

// ----------------------------------------------------------------------

#include <qlineedit.h>
#include <qsizepolicy.h>

// ----------------------------------------------------------------------

#include "PropertySizePolicyItem.moc"

// ----------------------------------------------------------------------

namespace PropertySizePolicyItemNamespace
{
	int size_type_to_int(QSizePolicy::SizeType t)
	{
		if (t == QSizePolicy::Fixed)
			return 0;
		if (t == QSizePolicy::Minimum)
			return 1;
		if (t == QSizePolicy::Maximum)
			return 2;
		if (t == QSizePolicy::Preferred)
			return 3;
		if (t == QSizePolicy::MinimumExpanding)
			return 4;
		if (t == QSizePolicy::Expanding)
			return 5;
		if (t == QSizePolicy::Ignored)
			return 6;
		return 0;
	}
	
	QString size_type_to_string(QSizePolicy::SizeType t)
	{
		if (t == QSizePolicy::Fixed)
			return "Fixed";
		if (t == QSizePolicy::Minimum)
			return "Minimum";
		if (t == QSizePolicy::Maximum)
			return "Maximum";
		if (t == QSizePolicy::Preferred)
			return "Preferred";
		if (t == QSizePolicy::MinimumExpanding)
			return "MinimumExpanding";
		if (t == QSizePolicy::Expanding)
			return "Expanding";
		if (t == QSizePolicy::Ignored)
			return "Ignored";
		return QString();
	}
	
	QSizePolicy::SizeType int_to_size_type(int i)
	{	
		if (i == 0)
			return QSizePolicy::Fixed;
		if (i == 1)
			return QSizePolicy::Minimum;
		if (i == 2)
			return QSizePolicy::Maximum;
		if (i == 3)
			return QSizePolicy::Preferred;
		if (i == 4)
			return QSizePolicy::MinimumExpanding;
		if (i == 5)
			return QSizePolicy::Expanding;
		if (i == 6)
			return QSizePolicy::Ignored;
		return QSizePolicy::Preferred;
	}
}

using namespace PropertySizePolicyItemNamespace;

// ----------------------------------------------------------------------

PropertySizePolicyItem::PropertySizePolicyItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName)
: PropertyItem(l, after, prop, propName)
{
	lin = 0;
}

// ----------------------------------------------------------------------

QLineEdit *PropertySizePolicyItem::lined()
{
	if (lin)
		return lin;
	lin = new QLineEdit(listview->viewport());
	lin->hide();
	lin->setReadOnly(TRUE);
	return lin;
}

// ----------------------------------------------------------------------

void PropertySizePolicyItem::createChildren()
{
	QStringList lst;
	lst << "Fixed" << "Minimum" << "Maximum" << "Preferred" << "MinimumExpanding" << "Expanding" << "Ignored";
	
	PropertyItem *i = this;
	i = new PropertyListItem(listview, i, this, tr("hSizeType"), FALSE);
	i->setValue(lst);
	addChild(i);
	i = new PropertyListItem(listview, i, this, tr("vSizeType"), FALSE);
	i->setValue(lst);
	addChild(i);
	i = new PropertyIntItem(listview, i, this, tr("horizontalStretch"), TRUE);
	addChild(i);
	i = new PropertyIntItem(listview, i, this, tr("verticalStretch"), TRUE);
	addChild(i);
}

// ----------------------------------------------------------------------

void PropertySizePolicyItem::initChildren()
{
	PropertyItem *item = 0;
	QSizePolicy sp = val.toSizePolicy();
	for (int i = 0; i < childCount(); ++i)
	{
		item = PropertyItem::child(i);
		if (item->name() == tr("hSizeType"))
			((PropertyListItem*)item)->setCurrentItem(size_type_to_int(sp.horData()));
		else if (item->name() == tr("vSizeType"))
			((PropertyListItem*)item)->setCurrentItem(size_type_to_int(sp.verData()));
		else if (item->name() == tr("horizontalStretch"))
			((PropertyIntItem*)item)->setValue(sp.horStretch());
		else if (item->name() == tr("verticalStretch"))
			((PropertyIntItem*)item)->setValue(sp.verStretch());
	}
}

// ----------------------------------------------------------------------

PropertySizePolicyItem::~PropertySizePolicyItem()
{
	delete (QLineEdit*)lin;
}

// ----------------------------------------------------------------------

void PropertySizePolicyItem::showEditor()
{
	PropertyItem::showEditor();
	placeEditor(lined());
	if (!lined()->isVisible() || !lined()->hasFocus())
	{
		lined()->show();
		listView()->viewport()->setFocus();
	}
}

// ----------------------------------------------------------------------

void PropertySizePolicyItem::hideEditor()
{
	PropertyItem::hideEditor();
	lined()->hide();
}

// ----------------------------------------------------------------------

void PropertySizePolicyItem::setValue(const QVariant &v)
{
	if (value() == v)
		return;
	
	QString s = tr("%1/%2/%2/%2");
	s = s.arg(size_type_to_string(v.toSizePolicy().horData())).
		arg(size_type_to_string(v.toSizePolicy().verData())).
		arg(v.toSizePolicy().horStretch()).
		arg(v.toSizePolicy().verStretch());
	setText(1, s);
	lined()->setText(s);
	PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

void PropertySizePolicyItem::childValueChanged(PropertyItem *child)
{
	QSizePolicy sp = val.toSizePolicy();
	if (child->name() == tr("hSizeType"))
		sp.setHorData(int_to_size_type(((PropertyListItem*)child)->currentIntItem()));
	else if (child->name() == tr("vSizeType"))
		sp.setVerData(int_to_size_type(((PropertyListItem*)child)->currentIntItem()));
	else if (child->name() == tr("horizontalStretch"))
		sp.setHorStretch(((PropertyIntItem*)child)->value().toInt());
	else if (child->name() == tr("verticalStretch"))
		sp.setVerStretch(((PropertyIntItem*)child)->value().toInt());
	setValue(sp);
	notifyValueChange();
}

// ----------------------------------------------------------------------

bool PropertySizePolicyItem::hasSubItems() const
{
	return TRUE;
}

// ======================================================================
