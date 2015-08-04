// ============================================================================
//
// NewNpcDialog.h
// Copyright 2004, Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_NewNpcDialog_H
#define INCLUDED_NewNpcDialog_H

#include "BaseNewNpcDialog.h"

class DataTable;

// ----------------------------------------------------------------------------

class NewNpcDialog : public BaseNewNpcDialog
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	explicit NewNpcDialog(QWidget *myParent, char const *windowName = 0);
	virtual ~NewNpcDialog();

	void setNpcDataTable(DataTable * npcDataTable);
	void setupControls(DataTable * npcDataTable);

	const char * getSelectedSAT() const;
	int getSelectedSATRow() const;

public slots:


private:

	DataTable * m_npcDataTable;

private: //-- disabled

	NewNpcDialog(NewNpcDialog const &);
	NewNpcDialog &operator =(NewNpcDialog const &);
	NewNpcDialog();
};

// ----------------------------------------------------------------------------

inline void NewNpcDialog::setNpcDataTable(DataTable * npcDataTable)
{
	m_npcDataTable = npcDataTable;
}

// ============================================================================

#endif // INCLUDED_NewNpcDialog_H
