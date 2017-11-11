// ======================================================================
//
// PropertyFileItem.cpp
// Copyright 2004, Sony Online Entertainment
//
// ======================================================================

#include "FirstQuestEditor.h"

// ----------------------------------------------------------------------

#include "PropertyList.h"
#include "PropertyFileItem.h"

// ----------------------------------------------------------------------

#include <qfiledialog.h>
#include <qhbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>

// ----------------------------------------------------------------------

#include "PropertyFileItem.moc"

// ----------------------------------------------------------------------

PropertyFileItem::PropertyFileItem(PropertyList *l, PropertyItem *after, PropertyItem *prop, const QString &propName, const QString &defaultDirectory, const QString &extensionFilter)
: PropertyItem(l, after, prop, propName)
, m_defaultDirectory(defaultDirectory)
, m_extensionFilter(extensionFilter)
, m_browseButton(0)
, m_lin(0)
{
}

// ----------------------------------------------------------------------

QLineEdit *PropertyFileItem::lined()
{
    if (m_lin)
		return m_lin;

	m_hbox = new QHBox(listview->viewport());
	m_hbox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	m_hbox->setLineWidth(2);
	m_hbox->hide();
	
	m_lin = new QLineEdit(m_hbox);
	m_browseButton = new QPushButton(tr("..."), m_hbox);
	m_browseButton->setFixedWidth(20);
	connect(m_browseButton, SIGNAL(clicked()), this, SLOT(getFile()));
	m_lin->setFrame(FALSE);

    connect(m_lin, SIGNAL(returnPressed()), this, SLOT(setValue()));
    connect(m_lin, SIGNAL(textChanged(const QString &)), this, SLOT(setValue()));

    m_hbox->installEventFilter(listview);

    return m_lin;
}

// ----------------------------------------------------------------------

PropertyFileItem::~PropertyFileItem()
{
    delete (QLineEdit *)m_lin;
    m_lin = 0;
	
	delete (QHBox *)m_hbox;
	m_hbox = 0;
}

// ----------------------------------------------------------------------

void PropertyFileItem::showEditor()
{
    PropertyItem::showEditor();

    if (!m_lin || m_lin->text().length() == 0)
	{
		lined()->blockSignals(TRUE);
		lined()->setText(value().toString());
		lined()->blockSignals(FALSE);
    }
	
    placeEditor(m_hbox);
    if (!m_hbox->isVisible() || !lined()->hasFocus())
	{
		m_hbox->show();
		setFocus(lined());
    }
}

// ----------------------------------------------------------------------

void PropertyFileItem::hideEditor()
{
    PropertyItem::hideEditor();
    m_hbox->hide();
}

// ----------------------------------------------------------------------

void PropertyFileItem::setValue(const QVariant &v)
{
    if ((!hasSubItems() || !isOpen()) && value() == v)
		return;

    if (m_lin)
	{
		lined()->blockSignals(TRUE);
		int oldCursorPos;
		oldCursorPos = m_lin->cursorPosition();
		lined()->setText(v.toString());
		if (oldCursorPos < (int)m_lin->text().length())
			m_lin->setCursorPosition(oldCursorPos);
		lined()->blockSignals(FALSE);
    }
    setText(1, v.toString());
    PropertyItem::setValue(v);
}

// ----------------------------------------------------------------------

void PropertyFileItem::setValue()
{
    setText(1, lined()->text());
    QVariant v;
	v = lined()->text();
    PropertyItem::setValue(v);
    notifyValueChange();
}

// ----------------------------------------------------------------------

void PropertyFileItem::getFile()
{
	QString txt = QFileDialog::getOpenFileName(m_defaultDirectory, m_extensionFilter, listview, 0, "Browse for file");

    if (!txt.isEmpty())
	{
		if (validate(txt))
		{
			setText(1, txt);
			PropertyItem::setValue(txt);
			notifyValueChange();
			lined()->blockSignals(TRUE);
			lined()->setText(txt);
			lined()->blockSignals(FALSE);
		}
    }
}

// ----------------------------------------------------------------------

bool PropertyFileItem::validate(QString &)
{
	return true;
}

// ======================================================================
