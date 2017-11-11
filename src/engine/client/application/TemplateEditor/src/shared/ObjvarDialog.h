// ============================================================================
//
// ObjvarDialog.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_ObjvarDialog_H
#define INCLUDED_ObjvarDialog_H

#include "StaticListDialog.h"

class DialogObjvarList;
class DynamicVariableParamData;
class TemplateTableRow;

//-----------------------------------------------------------------------------
class ObjvarDialog : public StaticListDialog
{
public:

	ObjvarDialog(QWidget *parent, char const *name, TemplateTableRow &parentTemplateTableRow);

	void    init(DynamicVariableParamData const &objvarData);
	QString getName() const;
	QString getValue() const;

private:

	TemplateTableRow *m_nameTemplateTableRow;
	TemplateTableRow *m_valueTemplateTableRow;
	DialogObjvarList *m_dialogObjVarList;

private:

	// Disabled

	ObjvarDialog();
	ObjvarDialog(ObjvarDialog const &);
	ObjvarDialog &operator=(ObjvarDialog const &);
};

// ============================================================================

#endif // INCLUDED_ObjvarDialog_H
