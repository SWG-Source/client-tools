// ======================================================================
//
// PropertyDateItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyList.h"
#include "PropertyDateItem.h"

// ----------------------------------------------------------------------

#include <qdatetimeedit.h>
#include <qobjectlist.h>

// ----------------------------------------------------------------------

#include "PropertyDateItem.moc"

// ----------------------------------------------------------------------

PropertyDateItem::PropertyDateItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName)
: PropertyItem(l, after, prop, propName)
{
	lin = 0;
}

// ----------------------------------------------------------------------

QDateEdit *PropertyDateItem::lined()
{
	if (lin)
		return lin;

	lin = new QDateEdit(listview->viewport());
	QObjectList *l = lin->queryList("QLineEdit");
	for (QObject *o = l->first(); o; o = l->next())
		o->installEventFilter(listview);
	delete l;

	connect(lin, SIGNAL(valueChanged(const QDate &)), this, SLOT(setValue()));

	return lin;
}

// ----------------------------------------------------------------------

PropertyDateItem::~PropertyDateItem()
{
	delete (QDateEdit*)lin;
	lin = 0;
}

// ----------------------------------------------------------------------

void PropertyDateItem::showEditor()
{
	PropertyItem::showEditor();
	if (!lin)
	{
		lined()->blockSignals(TRUE);
		lined()->setDate(value().toDate());
		lined()->blockSignals(FALSE);
	}
	placeEditor(lin);
	if (!lin->isVisible())
	{
		lin->show();
		setFocus(lin);
	}
}

// ----------------------------------------------------------------------

void PropertyDateItem::hideEditor()
{
	PropertyItem::hideEditor();
	if (lin)
		lin->hide();
}

// ----------------------------------------------------------------------

void PropertyDateItem::setValue(const QVariant &v)
{
	if ((!hasSubItems() || !isOpen())
		&& value() == v)
		return;
	
	if (lin)
	{
		lined()->blockSignals(TRUE);
		if (lined()->date() != v.toDate())
			lined()->setDate(v.toDate());
		lined()->blockSignals(FALSE);
	}
	setText(1, v.toDate().toString(::Qt::ISODate));
	PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

void PropertyDateItem::setValue()
{
	setText(1, lined()->date().toString(::Qt::ISODate));
	QVariant v;
	v = lined()->date();
	PropertyItem::setValue(v);
	notifyValueChange();
}

// ======================================================================
