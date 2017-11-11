// ============================================================================
//
// SaveDialog.h
// Copyright 2004, Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_SaveDialog_H
#define INCLUDED_SaveDialog_H

#include "BaseSaveDialog.h"

#include <qlineedit.h>

// ----------------------------------------------------------------------------

class SaveDialog : public BaseSaveDialog
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	SaveDialog(QWidget *myParent, bool openFileMode, char const *windowName = 0);
	virtual ~SaveDialog();

	char const * getClientDataFileName(bool relative = false) const;
	char const * getServerFileName(bool relative = false) const;
	char const * getSharedFileName(bool relative = false) const;

	void setClientDataFileName(char const * filename);
	void setServerFileName(char const * filename);
	void setSharedFileName(char const * filename);

	static void makeClientNameRelative(QString * filename);
	static void makeServerNameRelative(QString * filename);
	static void makeSharedNameRelative(QString * filename);

public slots:

    virtual void slotServerBrowseReleased();
    virtual void slotSharedBrowseReleased();
    virtual void slotClientDataFileBrowseReleased();
	virtual void slotOkReleased();

private: //-- disabled

	SaveDialog(SaveDialog const &);
	SaveDialog &operator =(SaveDialog const &);
	SaveDialog();
};

// ----------------------------------------------------------------------------

inline void SaveDialog::setClientDataFileName(char const * filename)
{
	m_lineEditClientDataFile->setText(filename);
}

// ----------------------------------------------------------------------------

inline void SaveDialog::setServerFileName(char const * filename)
{
	m_lineEditServerFileName->setText(filename);
}

// ----------------------------------------------------------------------------

inline void SaveDialog::setSharedFileName(char const * filename)
{
	m_lineEditSharedFileName->setText(filename);
}

// ============================================================================

#endif // INCLUDED_SaveDialog_H
