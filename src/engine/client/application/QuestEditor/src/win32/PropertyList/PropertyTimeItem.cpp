// ======================================================================
//
// PropertyTimeItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyList.h"
#include "PropertyTimeItem.h"

// ----------------------------------------------------------------------

#include <qdatetimeedit.h>
#include <qobjectlist.h>

// ----------------------------------------------------------------------

#include "PropertyTimeItem.moc"

// ----------------------------------------------------------------------

PropertyTimeItem::PropertyTimeItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName)
: PropertyItem(l, after, prop, propName)
{
	lin = 0;
}

// ----------------------------------------------------------------------

QTimeEdit *PropertyTimeItem::lined()
{
	if (lin)
		return lin;
	lin = new QTimeEdit(listview->viewport());
	connect(lin, SIGNAL(valueChanged(const QTime &)),
		this, SLOT(setValue()));
	QObjectList *l = lin->queryList("QLineEdit");
	for (QObject *o = l->first(); o; o = l->next())
		o->installEventFilter(listview);
	delete l;
	return lin;
}

// ----------------------------------------------------------------------

PropertyTimeItem::~PropertyTimeItem()
{
	delete (QTimeEdit*)lin;
	lin = 0;
}

// ----------------------------------------------------------------------

void PropertyTimeItem::showEditor()
{
	PropertyItem::showEditor();
	if (!lin)
	{
		lined()->blockSignals(true);
		lined()->setTime(value().toTime());
		lined()->blockSignals(false);
	}
	placeEditor(lin);
	if (!lin->isVisible())
	{
		lin->show();
		setFocus(lin);
	}
}

// ----------------------------------------------------------------------

void PropertyTimeItem::hideEditor()
{
	PropertyItem::hideEditor();
	if (lin)
		lin->hide();
}

// ----------------------------------------------------------------------

void PropertyTimeItem::setValue(const QVariant &v)
{
	if ((!hasSubItems() || !isOpen()) && value() == v)
		return;
	
	if (lin)
	{
		lined()->blockSignals(true);
		if (lined()->time() != v.toTime())
			lined()->setTime(v.toTime());
		lined()->blockSignals(false);
	}
	setText(1, v.toTime().toString(::Qt::ISODate));
	PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

void PropertyTimeItem::setValue()
{
	setText(1, lined()->time().toString(::Qt::ISODate));
	QVariant v;
	v = lined()->time();
	PropertyItem::setValue(v);
	notifyValueChange();
}

// ======================================================================
