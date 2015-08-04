// ======================================================================
//
// LocEditingWidget.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LocEditingWidget_H
#define INCLUDED_LocEditingWidget_H

#include "ui_EditingWidget.h"
#include "Unicode.h"
#include "LocalizedStringPair.h"
#include "QString.h"

class LocalizedString;
class LocalizedStringPair;
class QFont;

// ======================================================================

class LocEditingWidget : public EditingWidget
{
private:
		Q_OBJECT

public:

	explicit               LocEditingWidget (QWidget * parent);
	                      ~LocEditingWidget ();

	virtual void           onModified();

public slots:
	void                   startEditingString (const LocalizedStringPair & stringPair);
	void                   cancelEditing ();
	void                   resetValues ();
	void                   createNewEntry ();
	void                   deleteEntry ();
	bool                   applyChanges ();

	void                   readOnlyChanged ();

	const QFont            getSourceFont () const;
	const QFont            getTranslatedFont () const;

	void                   setSourceFont (const QFont &);
	void                   setTranslatedFont (const QFont &);

	void                   onNameTextChanged (const QString & str);

private:
	                       LocEditingWidget ();
	                       LocEditingWidget (const LocEditingWidget & rhs);
	LocEditingWidget &     operator=    (const LocEditingWidget & rhs);

	void                   setEditingString (const LocalizedStringPair * stringPair);

	LocalizedStringPair    m_stringPair;

	QString                m_originalSourceString;
	QString                m_originalTranslatedString;
	QString                m_originalName;

	bool                   m_pairValid;

	QObject *              m_filter;
	QObject *              m_nameFilter;

	bool                   m_textChanging;
};

// ======================================================================

#endif
