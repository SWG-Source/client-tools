// ============================================================================
//
// StructDialog.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_StructDialog_H
#define INCLUDED_StructDialog_H

#include "StaticListDialog.h"

class TemplateTableRow;
class TpfTemplate;

//-----------------------------------------------------------------------------
class StructDialog : public StaticListDialog
{
public:

	StructDialog(QWidget *parent, const char *name, TemplateTableRow &parentRow);

	void setValue(TpfTemplate &structTemplate, std::string const &path);

private:

	// Disabled

	StructDialog();
	StructDialog(StructDialog const &);
	StructDialog &operator=(StructDialog const &);
};

// ============================================================================

#endif // INCLUDED_StructDialog_H
