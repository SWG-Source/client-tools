// ======================================================================
//
// PropertyFontItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyBoolItem.h"
#include "PropertyIntItem.h"
#include "PropertyList.h"
#include "PropertyListItem.h"
#include "PropertyFontItem.h"

// ----------------------------------------------------------------------

#include <qapplication.h>
#include <qfontdialog.h>
#include <qfontdatabase.h>
#include <qhbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>

// ----------------------------------------------------------------------

#include "PropertyFontItem.moc"

// ----------------------------------------------------------------------

namespace PropertyFontItemNamespace
{
	QFontDatabase *fontDataBase = 0;

	void cleanupFontDatabase()
	{
		delete fontDataBase;
		fontDataBase = 0;
	}
	
	QStringList getFontList()
	{
		if (!fontDataBase)
		{
			fontDataBase = new QFontDatabase;
			qAddPostRoutine(cleanupFontDatabase);
		}

		return fontDataBase->families();
	}
}

using namespace PropertyFontItemNamespace;

// ----------------------------------------------------------------------

PropertyFontItem::PropertyFontItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName)
: PropertyItem(l, after, prop, propName)
{
	box = new QHBox(listview->viewport());
	box->hide();
	lined = new QLineEdit(box);
	button = new QPushButton("...", box);
	button->setFixedWidth(20);
	box->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	box->setLineWidth(2);
	lined->setFrame(FALSE);
	lined->setReadOnly(TRUE);
	box->setFocusProxy(lined);
	box->installEventFilter(listview);
	lined->installEventFilter(listview);
	button->installEventFilter(listview);
	connect(button, SIGNAL(clicked()), this, SLOT(getFont()));
}

// ----------------------------------------------------------------------

void PropertyFontItem::createChildren()
{
	PropertyItem *i = this;
	i = new PropertyListItem(listview, i, this, tr("Family"), FALSE);
	addChild(i);
	i = new PropertyIntItem(listview, i, this, tr("Point Size"), TRUE);
	addChild(i);
	i = new PropertyBoolItem(listview, i, this, tr("Bold"));
	addChild(i);
	i = new PropertyBoolItem(listview, i, this, tr("Italic"));
	addChild(i);
	i = new PropertyBoolItem(listview, i, this, tr("Underline"));
	addChild(i);
	i = new PropertyBoolItem(listview, i, this, tr("Strikeout"));
	addChild(i);
}

// ----------------------------------------------------------------------

void PropertyFontItem::initChildren()
{
	PropertyItem *item = 0;
	for (int i = 0; i < childCount(); ++i)
	{
		item = PropertyItem::child(i);
		if (item->name() == tr("Family"))
		{
			((PropertyListItem*)item)->setValue(getFontList());
			((PropertyListItem*)item)->setCurrentItem(val.toFont().family());
		}
		else if (item->name() == tr("Point Size"))
			item->setValue(val.toFont().pointSize());
		else if (item->name() == tr("Bold"))
			item->setValue(QVariant(val.toFont().bold(), 0));
		else if (item->name() == tr("Italic"))
			item->setValue(QVariant(val.toFont().italic(), 0));
		else if (item->name() == tr("Underline"))
			item->setValue(QVariant(val.toFont().underline(), 0));
		else if (item->name() == tr("Strikeout"))
			item->setValue(QVariant(val.toFont().strikeOut(), 0));
	}
}

// ----------------------------------------------------------------------

PropertyFontItem::~PropertyFontItem()
{
	delete (QHBox*)box;
}

// ----------------------------------------------------------------------

void PropertyFontItem::showEditor()
{
	PropertyItem::showEditor();
	placeEditor(box);
	if (!box->isVisible() || !lined->hasFocus())
	{
		box->show();
		setFocus(lined);
	}
}

// ----------------------------------------------------------------------

void PropertyFontItem::hideEditor()
{
	PropertyItem::hideEditor();
	box->hide();
}

// ----------------------------------------------------------------------

void PropertyFontItem::setValue(const QVariant &v)
{
	if (value() == v)
		return;
	
	setText(1, v.toFont().family() + "-" + QString::number(v.toFont().pointSize()));
	lined->setText(v.toFont().family() + "-" + QString::number(v.toFont().pointSize()));
	PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

void PropertyFontItem::getFont()
{
	bool ok = FALSE;
	QFont f = QFontDialog::getFont(&ok, val.toFont(), listview);
	if (ok && f != val.toFont())
	{
		setValue(f);
		notifyValueChange();
	}
}

// ----------------------------------------------------------------------

bool PropertyFontItem::hasSubItems() const
{
	return TRUE;
}

// ----------------------------------------------------------------------

void PropertyFontItem::childValueChanged(PropertyItem *child)
{
	QFont f = val.toFont();
	if (child->name() == tr("Family"))
		f.setFamily(((PropertyListItem*)child)->currentItem());
	else if (child->name() == tr("Point Size"))
		f.setPointSize(child->value().toInt());
	else if (child->name() == tr("Bold"))
		f.setBold(child->value().toBool());
	else if (child->name() == tr("Italic"))
		f.setItalic(child->value().toBool());
	else if (child->name() == tr("Underline"))
		f.setUnderline(child->value().toBool());
	else if (child->name() == tr("Strikeout"))
		f.setStrikeOut(child->value().toBool());

	setValue(f);
	notifyValueChange();
}

// ======================================================================
