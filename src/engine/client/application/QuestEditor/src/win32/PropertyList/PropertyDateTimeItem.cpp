// ======================================================================
//
// PropertyDateTimeItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyList.h"
#include "PropertyDateTimeItem.h"

// ----------------------------------------------------------------------

#include <qdatetimeedit.h>
#include <qobjectlist.h>

// ----------------------------------------------------------------------

#include "PropertyDateTimeItem.moc"

// ----------------------------------------------------------------------

PropertyDateTimeItem::PropertyDateTimeItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName)
: PropertyItem(l, after, prop, propName)
{
	lin = 0;
}

// ----------------------------------------------------------------------

QDateTimeEdit *PropertyDateTimeItem::lined()
{
	if (lin)
		return lin;
	lin = new QDateTimeEdit(listview->viewport());
	connect(lin, SIGNAL(valueChanged(const QDateTime &)),
		this, SLOT(setValue()));
	QObjectList *l = lin->queryList("QLineEdit");
	for (QObject *o = l->first(); o; o = l->next())
		o->installEventFilter(listview);
	delete l;
	return lin;
}

// ----------------------------------------------------------------------

PropertyDateTimeItem::~PropertyDateTimeItem()
{
	delete (QDateTimeEdit*)lin;
	lin = 0;
}

// ----------------------------------------------------------------------

void PropertyDateTimeItem::showEditor()
{
	PropertyItem::showEditor();
	if (!lin)
	{
		lined()->blockSignals(TRUE);
		lined()->setDateTime(value().toDateTime());
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

void PropertyDateTimeItem::hideEditor()
{
	PropertyItem::hideEditor();
	if (lin)
		lin->hide();
}

// ----------------------------------------------------------------------

void PropertyDateTimeItem::setValue(const QVariant &v)
{
	if ((!hasSubItems() || !isOpen())
		&& value() == v)
		return;
	
	if (lin)
	{
		lined()->blockSignals(TRUE);
		if (lined()->dateTime() != v.toDateTime())
			lined()->setDateTime(v.toDateTime());
		lined()->blockSignals(FALSE);
	}
	setText(1, v.toDateTime().toString(::Qt::ISODate));
	PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

void PropertyDateTimeItem::setValue()
{
	setText(1, lined()->dateTime().toString(::Qt::ISODate));
	QVariant v;
	v = lined()->dateTime();
	PropertyItem::setValue(v);
	notifyValueChange();
}

// ======================================================================
