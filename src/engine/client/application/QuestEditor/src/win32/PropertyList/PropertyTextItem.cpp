// ======================================================================
//
// PropertyTextItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyList.h"
#include "PropertyTextItem.h"

// ----------------------------------------------------------------------

#include <qhbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qvalidator.h>

// ----------------------------------------------------------------------

#include "PropertyTextItem.moc"

// ----------------------------------------------------------------------

PropertyTextItem::PropertyTextItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName, bool comment, bool multiLine, bool a)
: PropertyItem(l, after, prop, propName)
, withComment(comment)
, hasMultiLines(multiLine)
, accel(a)
{
    lin = 0;
    box = 0;
}

// ----------------------------------------------------------------------

QLineEdit *PropertyTextItem::lined()
{
    if (lin)
		return lin;

    if (hasMultiLines)
	{
		box = new QHBox(listview->viewport());
		box->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
		box->setLineWidth(2);
		box->hide();
    }
	
    lin = 0;
    if (hasMultiLines)
		lin = new QLineEdit(box);
    else
		lin = new QLineEdit(listview->viewport());
	
	if (!hasMultiLines)
	{
		lin->hide();
    }
	else
	{
		button = new QPushButton(tr("..."), box);
		button->setFixedWidth(20);
		connect(button, SIGNAL(clicked()), this, SLOT(getText()));
		lin->setFrame(FALSE);
    }

    connect(lin, SIGNAL(returnPressed()), this, SLOT(setValue()));
    connect(lin, SIGNAL(textChanged(const QString &)), this, SLOT(setValue()));

    lin->installEventFilter(listview);
    return lin;
}

// ----------------------------------------------------------------------

PropertyTextItem::~PropertyTextItem()
{
    delete (QLineEdit*)lin;
    lin = 0;
    delete (QHBox*)box;
    box = 0;
}

// ----------------------------------------------------------------------

void PropertyTextItem::setChanged(bool b, bool updateDb)
{
    PropertyItem::setChanged(b, updateDb);
    if (withComment && childCount() > 0)
		((PropertyTextItem*)PropertyItem::child(0))->lined()->setEnabled(b);
}

// ----------------------------------------------------------------------

bool PropertyTextItem::hasSubItems() const
{
    return withComment;
}

// ----------------------------------------------------------------------

void PropertyTextItem::childValueChanged(PropertyItem *child)
{
	UNREF(child);
}

// ----------------------------------------------------------------------

void PropertyTextItem::showEditor()
{
    PropertyItem::showEditor();
    if (!lin || lin->text().length() == 0)
	{
		lined()->blockSignals(TRUE);
		lined()->setText(value().toString());
		lined()->blockSignals(FALSE);
    }
	
    QWidget* w;
    if (hasMultiLines)
		w = box;
    else
		w = lined();
	
    placeEditor(w);
    if (!w->isVisible() || !lined()->hasFocus())
	{
		w->show();
		setFocus(lined());
    }
}

// ----------------------------------------------------------------------

void PropertyTextItem::createChildren()
{
    PropertyTextItem *i = new PropertyTextItem(listview, this, this,
		PropertyItem::name() == "name" ?
		"export macro" : "comment", FALSE, FALSE,
		PropertyItem::name() == "name");
    i->lined()->setEnabled(isChanged());
    addChild(i);
}

// ----------------------------------------------------------------------

void PropertyTextItem::initChildren()
{
    if (!childCount())
		return;
}

// ----------------------------------------------------------------------

void PropertyTextItem::hideEditor()
{
    PropertyItem::hideEditor();
    QWidget* w;
    if (hasMultiLines)
		w = box;
    else
		w = lined();
	
    w->hide();
}

// ----------------------------------------------------------------------

void PropertyTextItem::setValue(const QVariant &v)
{
    if ((!hasSubItems() || !isOpen()) && value() == v)
		return;

    if (lin)
	{
		lined()->blockSignals(TRUE);
		int oldCursorPos;
		oldCursorPos = lin->cursorPosition();
		lined()->setText(v.toString());
		if (oldCursorPos < (int)lin->text().length())
			lin->setCursorPosition(oldCursorPos);
		lined()->blockSignals(FALSE);
    }
    setText(1, v.toString());
    PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

void PropertyTextItem::setValue()
{
    setText(1, lined()->text());
    QVariant v;
    if (accel)
	{
		v = QVariant(QKeySequence(lined()->text()));
		if (v.toString().isNull())
			return; // not yet valid input
    } else
	{
		v = lined()->text();
    }
    PropertyItem::setValue(v);
    notifyValueChange();
}

// ----------------------------------------------------------------------

void PropertyTextItem::getText()
{
	/* TTODO -- support MultiLineEditor?
    QString txt = MultiLineEditor::getText(listview, value().toString(), richText, &doWrap);
	*/

    QString txt = value().toString();
    if (!txt.isEmpty())
	{
		setText(1, txt);
		PropertyItem::setValue(txt);
		notifyValueChange();
		lined()->blockSignals(TRUE);
		lined()->setText(txt);
		lined()->blockSignals(FALSE);
    }
}

// ----------------------------------------------------------------------

void PropertyTextItem::setValidator(QValidator const * validator)
{
	lined()->setValidator(validator);
}

// ======================================================================
