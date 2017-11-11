// ============================================================================
//
// StringIDDialog.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_StringIDDialog_H
#define INCLUDED_StringIDDialog_H

#include "StaticListDialog.h"

class TemplateTableRow;
class StringId;

//-----------------------------------------------------------------------------
class StringIDDialog : public StaticListDialog
{
public:

	StringIDDialog(QWidget *parent, const char *name, TemplateTableRow &parentRow);

	void    setValue(const StringId &stringId);

	QString getTableName() const;
	QString getIndexName() const;

private:

	TemplateTableRow *m_templateTableRowString;
	TemplateTableRow *m_templateTableRowIndex;

private:

	// Disabled

	StringIDDialog();
	StringIDDialog(StringIDDialog const &);
	StringIDDialog &operator=(StringIDDialog const &);
};

// ============================================================================

#endif // INCLUDED_StringIDDialog_H
