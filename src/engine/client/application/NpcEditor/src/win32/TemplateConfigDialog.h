// ============================================================================
//
// TemplateConfigDialog.h
// Copyright 2004, Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TemplateConfigDialog_H
#define INCLUDED_TemplateConfigDialog_H

#include "BaseTemplateConfigDialog.h"
#include "ObjectTemplateWriter.h"

class DataTable;

// ----------------------------------------------------------------------------

class TemplateConfigDialog : public BaseTemplateConfigDialog
{
	Q_OBJECT //lint !e1924 !e1511 !e1516

public:

	explicit TemplateConfigDialog(QWidget *myParent, char const *windowName = 0);
	virtual ~TemplateConfigDialog();

	void setNpcDataTable(DataTable * npcDataTable);

	void setupControls(DataTable * npcDataTable,
		ObjectTemplateWriter::ObjectTemplateParameterMap * serverParameters,
		ObjectTemplateWriter::ObjectTemplateParameterMap * sharedParameters);

	void updateParameters(ObjectTemplateWriter::ObjectTemplateParameterMap * serverParameters,
		ObjectTemplateWriter::ObjectTemplateParameterMap * sharedParameters) const;

public slots:

    virtual void slotServerBrowseWeaponReleased();

private:

	DataTable * m_npcDataTable;

private: //-- disabled

	TemplateConfigDialog(TemplateConfigDialog const &);
	TemplateConfigDialog &operator =(TemplateConfigDialog const &);
	TemplateConfigDialog();
};

// ----------------------------------------------------------------------------

inline void TemplateConfigDialog::setNpcDataTable(DataTable * npcDataTable)
{
	m_npcDataTable = npcDataTable;
}

// ============================================================================

#endif // INCLUDED_TemplateConfigDialog_H
