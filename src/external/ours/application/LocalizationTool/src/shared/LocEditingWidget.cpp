// ======================================================================
//
// LocEditingWidget.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "StdAfx.h"
#include "LocEditingWidget.h"
#include "LocEditingWidget.moc"

#include "LocalizationData.h"
#include "LocalizedString.h"
#include "LocalizedStringTableReaderWriter.h"
#include "MyMultiLineEdit.h"
#include "StringUtils.h"
#include "UnicodeUtils.h"

#include <cassert>
#include <qfont.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qmessagebox.h>


//-----------------------------------------------------------------

namespace
{
/**
* Filter to prevent multiline edits from receiving TAB events
	*/
	class MyTabFilter : 
	public QObject
	{
	public:
		MyTabFilter () : QObject () {}
		
		inline bool eventFilter( QObject *o, QEvent *e )
		{
			if ( e->type() == QEvent::KeyPress)
			{
				const QKeyEvent * const k = static_cast<QKeyEvent*>(e);
				
				if (k->key () == static_cast<int>(Qt::Key_Tab))
				{
					static_cast<MyMultiLineEdit*>(o)->focusNextPrevChild (true);
					return true;                        // eat event
				}
			}
			return o->eventFilter( o, e );    // standard event processing
		}
		
		
	private:	// Disabled copy constructor and operator=
		MyTabFilter( const MyTabFilter & );
		MyTabFilter &operator=( const MyTabFilter & );
	};
	
	//----------------------------------------------------------------------
	
	class MyNameFilter : 
	public QObject
	{
	public:
		MyNameFilter () : QObject () {}
		
		inline bool eventFilter( QObject *o, QEvent *e )
		{
			if ( e->type() == QEvent::KeyPress)
			{
				const QKeyEvent * const k = static_cast<QKeyEvent*>(e);
				
				if (isprint (k->ascii ()) && 
					!isalpha (k->ascii ()) &&
					!isdigit (k->ascii ()) &&
					k->ascii () != ' ' &&
					k->ascii () != '+' &&
					k->ascii () != '_')
				{
					QApplication::beep ();
					return true;
				}
			}
			return o->eventFilter( o, e );    // standard event processing
		}
		
	private:	// Disabled copy constructor and operator=
		MyNameFilter( const MyNameFilter & );
		MyNameFilter &operator=( const MyNameFilter & );
	};
}

// ======================================================================

LocEditingWidget::LocEditingWidget (QWidget * theParent) :
EditingWidget              (theParent, "EditingWidget"),
m_stringPair               (),
m_originalSourceString     (),
m_originalTranslatedString (),
m_originalName             (),
m_pairValid                (false),
m_filter                   (new MyTabFilter),
m_nameFilter               (new MyNameFilter),
m_textChanging             (false)
{
	assert (m_filter);
	assert (m_nameFilter);

	assert (EditName);
	assert (EditSourceText);
	assert (EditTranslatedText);

	EditName->installEventFilter           (m_nameFilter);
	EditSourceText->installEventFilter     (m_filter);
	EditTranslatedText->installEventFilter (m_filter);

	setEditingString (0);

	assert (ButtonApply);
	connect( ButtonApply, SIGNAL( clicked() ), this, SLOT( applyChanges() ) );

	connect( EditName, SIGNAL( textChanged(const QString &) ), this, SLOT( onNameTextChanged(const QString & ) ) );
}

//-----------------------------------------------------------------

LocEditingWidget::~LocEditingWidget ()
{
	EditName->removeEventFilter           (m_nameFilter);

	EditSourceText->removeEventFilter     (m_filter);
	EditTranslatedText->removeEventFilter (m_filter);

	delete m_filter;
	m_filter = 0;

	delete m_nameFilter;
	m_nameFilter = 0;
}

//-----------------------------------------------------------------

void LocEditingWidget::resetValues ()
{
	if (m_pairValid)
	{
		EditTranslatedText->setText (m_originalTranslatedString);
		EditSourceText->setText     (m_originalSourceString);
		EditName->setText           (m_originalName);
	
		LabelID->setText            (QString::number (m_stringPair.getId ()));
	}
	else
	{
		EditTranslatedText->clear ();
		EditSourceText->clear ();
		EditName->clear ();
		LabelID->clear ();
	}

	ButtonRevert->setEnabled (false);
	ButtonCancel->setEnabled (m_pairValid);

	//-- enable controls only if we are editing something
	EditTranslatedText->setReadOnly (!m_pairValid);

	EditSourceText->setEnabled  (m_pairValid);
	EditTranslatedText->setEnabled (m_pairValid && m_stringPair.getHasTranslatedString ());

	if (LocalizationData::getData ().getHasTranslatedData ())
	{
		TranslatedTextLabel->show ();
		EditTranslatedText->show ();
	}
	else
	{
		TranslatedTextLabel->hide ();
		EditTranslatedText->hide ();
	}

	//-- this updates the buttons and controls which depend on readonly status of the string tables
	readOnlyChanged ();
}

//-----------------------------------------------------------------

void LocEditingWidget::startEditingString (const LocalizedStringPair & stringPair)
{
	//-- apply changes to current, if modified
	if (ButtonRevert->isEnabled ())
	{
		if (!applyChanges ())
			return;
	}

	setEditingString (&stringPair);

	EditName->setFocus ();

	LocalizationData::getData ().setCurrentEditId (static_cast<int>(stringPair.getId ()));
}

//-----------------------------------------------------------------

void LocEditingWidget::setEditingString (const LocalizedStringPair * stringPair)
{
	if (stringPair)
	{
		m_stringPair               = *stringPair;
		m_originalSourceString     = StringUtils::convertUnicode (m_stringPair.getSourceString ().getString ().c_str ());
		m_originalTranslatedString = StringUtils::convertUnicode (m_stringPair.getTranslatedString ().getString ().c_str ());
		m_originalName = m_stringPair.getName ().c_str ();
		m_pairValid = true;
	}
	else
	{
		m_pairValid = false;			
	}

	resetValues ();
}

//-----------------------------------------------------------------

void LocEditingWidget::onModified()
{
	ButtonNew->setEnabled    (true);

	bool isModified = 
		m_originalName != EditName->text () || 
		m_originalSourceString != EditSourceText->text () || 
		(m_stringPair.getHasTranslatedString () && m_originalTranslatedString != EditTranslatedText->text ());

	ButtonRevert->setEnabled (isModified);
}

//-----------------------------------------------------------------

void LocEditingWidget::cancelEditing()
{
	// TODO: confirm discard edits?
	setEditingString (0);
	LocalizationData::getData ().setCurrentEditId (0);
}

//-----------------------------------------------------------------

void LocEditingWidget::createNewEntry ()
{
	if (!EditName->text ().isEmpty())
	{
		applyChanges ();
	}

	const LocalizedStringPair * sp = LocalizationData::getData ().getEntryByName("default");

	if (sp == NULL)
	{
		sp = LocalizationData::getData ().createNewEntry ();
	}

	assert (sp); //lint !e1924 //libc bug
	startEditingString (*sp);
	EditName->selectAll ();
}

//-----------------------------------------------------------------

void LocEditingWidget::deleteEntry ()
{
	assert (m_pairValid); //lint !e1924 //libc bug

	LocalizedString::id_type const id = m_stringPair.getId();

	setEditingString (0);

	LocalizationData::getData ().deleteEntryById (id);
	LocalizationData::getData ().setCurrentEditId (0);
}

//-----------------------------------------------------------------

bool LocEditingWidget::applyChanges ()
{
	using namespace StringUtils;
	
	//-- careful not to modify anything needlessly
	
	if (!LocalizationData::getData ().getReadOnly (0))
	{
		const std::string name (convertQStringNarrow (EditName->text ()));
		const bool nameValid = LocalizedStringTable::validateStringName (name);

		if (EditName->text () != m_originalName || !nameValid)
		{
			if (!nameValid)
			{
				QMessageBox::information (0, "Information", "Invalid String name.\nString name must contain only lower-case alphanumerics and '_' characters.");
				return false;
			}

			m_stringPair.setName (name);
		}
		
		if (m_originalSourceString != EditSourceText->text ())
		{
			LocalizedString & s_str                          = m_stringPair.getSourceString ();
			LocalizedStringTableRW::str_setString (s_str,      convertQString (EditSourceText->text ()));
		}
	}
	
	if (m_stringPair.getHasTranslatedString ())
	{
		LocalizedString & t_str                          = m_stringPair.getTranslatedString ();

		if (m_originalTranslatedString != EditTranslatedText->text ())
		{
			LocalizedStringTableRW::str_setString (t_str, convertQString (EditTranslatedText->text ()));
		}

		//-- always update the crc on the translated file when applying
		LocalizedStringTableRW::str_getSourceCrc(t_str) = m_stringPair.getSourceString().getCrc();
	}

	LocalizationData::getData().modifyEntry (m_stringPair);

	// changeup the values
	setEditingString (&m_stringPair);

	return true;
}

//-----------------------------------------------------------------

void LocEditingWidget::readOnlyChanged ()
{

	ButtonApply->setEnabled     (m_pairValid && (LocalizationData::getData ().getReadOnly (0) == false || m_stringPair.getHasTranslatedString ()));

	ButtonNew->setEnabled       (LocalizationData::getData ().getReadOnly (0) == false);

	ButtonDelete->setEnabled    (m_pairValid && LocalizationData::getData ().getReadOnly (0) == false);

	EditSourceText->setReadOnly (!m_pairValid || LocalizationData::getData ().getReadOnly (0));

	EditName->setEnabled        (m_pairValid && LocalizationData::getData ().getReadOnly (0) == false);
	EditName->setReadOnly       (!m_pairValid || LocalizationData::getData ().getReadOnly (0));
}

//-----------------------------------------------------------------

const QFont LocEditingWidget::getSourceFont () const
{
	return EditSourceText->font ();
}

//-----------------------------------------------------------------

const QFont LocEditingWidget::getTranslatedFont () const
{
	return EditTranslatedText->font ();
}

//-----------------------------------------------------------------

void LocEditingWidget::setSourceFont (const QFont & font)
{
	EditSourceText->setFont (font);
}

//-----------------------------------------------------------------

void LocEditingWidget::setTranslatedFont (const QFont & font)
{
	EditTranslatedText->setFont (font);
}

//----------------------------------------------------------------------

void LocEditingWidget::onNameTextChanged (const QString & str)
{
	if (m_textChanging)
		return;

	std::string name (StringUtils::convertQStringNarrow (str));
	const bool nameValid = LocalizedStringTable::validateStringName (name);
	if (!nameValid)
	{
		LocalizedStringTable::fixupStringName (name);
		m_textChanging = true;
		EditName->setText (StringUtils::convertUnicode (Unicode::narrowToWide (name)));
	}

	m_textChanging = false;
}

// ======================================================================
