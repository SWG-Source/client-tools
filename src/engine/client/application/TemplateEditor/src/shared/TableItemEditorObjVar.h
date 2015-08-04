// ============================================================================
//
// TableItemEditorObjVar.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemEditorObjVar_H
#define INCLUDED_TableItemEditorObjVar_H

#include "BaseDialogEdit.h"

class DynamicVariableParamData;
class EditTableItem;
class ObjvarDialog;
class TemplateTableRow;

//-----------------------------------------------------------------------------
class TableItemEditorObjVar : public BaseDialogEdit
{
public:

	TableItemEditorObjVar(QWidget *parentWidget, const char *name, TableItemBase &parentTableItemBase, TemplateTableRow &parentRow);

	void setValue(const DynamicVariableParamData &dynamicVariableParamData);

protected:

	virtual QString getTextFromDialog() const;

private:

	ObjvarDialog *getObjvarDialog() const;

private:

	// Disabled

	TableItemEditorObjVar &operator =(TableItemEditorObjVar const &);
};

// ============================================================================

#endif // INCLUDED_TableItemEditorObjVar_H
