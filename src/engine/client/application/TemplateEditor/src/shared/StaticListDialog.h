// ============================================================================
//
// StaticListDialog.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_StaticListDialog_H
#define INCLUDED_StaticListDialog_H

#include "BaseItemListDialog.h"

class TemplateTableRow;

//-----------------------------------------------------------------------------
class StaticListDialog : public BaseItemListDialog
{
public:

	StaticListDialog(QWidget *parent, const char *name, TemplateTableRow &parentRow);

	int                     getTemplateTableRowCount() const;
	TemplateTableRow *      getTemplateTableRow(int const row) const;
	TemplateTableRow const &getTemplateTableRow() const;

protected:

	virtual void keyReleaseEvent(QKeyEvent *keyEvent);

private:

	TemplateTableRow const &m_templateTableRow;

private:

	// Disabled

	StaticListDialog();
	StaticListDialog(StaticListDialog const &);
	StaticListDialog &operator=(StaticListDialog const &);
};

// ============================================================================

#endif // INCLUDED_StaticListDialog_H
