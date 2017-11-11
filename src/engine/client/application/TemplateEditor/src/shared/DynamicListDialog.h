// ============================================================================
//
// DynamicListDialog.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_DynamicListDialog_H
#define INCLUDED_DynamicListDialog_H

#include "BaseItemListDialog.h"

class TemplateTableRow;

//-----------------------------------------------------------------------------
class DynamicListDialog : public BaseItemListDialog
{
	Q_OBJECT

public:

	DynamicListDialog(QWidget *parent, const char *name, TemplateTableRow &parentTemplateTableRow);
	virtual ~DynamicListDialog();

	int                     getRowCount() const;
	TemplateTableRow *      getTemplateTableRow(int const row) const;
	TemplateTableRow const &getParentTemplateTableRow();
	void                    createRows(int const rowCount);

protected slots:

	virtual void addNewRowAboveCurrent();
	virtual void addNewRowBelowCurrent();
	virtual void deleteCurrentRow();

private:

	void addNewRow(int const row);
	void insertNewRow(int const row);

	TemplateTableRow const &m_parentTemplateTableRow;

private:

	// Disabled

	DynamicListDialog();
	DynamicListDialog(DynamicListDialog const &);
	DynamicListDialog &operator=(DynamicListDialog const &);
};

// ============================================================================

#endif // INCLUDED_DynamicListDialog_H
