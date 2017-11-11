// ============================================================================
//
// DialogObjvarList.h
//
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_DialogObjvarList_H
#define INCLUDED_DialogObjvarList_H

#include "DynamicListDialog.h"

class DynamicVariableParamData;

//-----------------------------------------------------------------------------
class DialogObjvarList : public DynamicListDialog
{
public:

	DialogObjvarList(QWidget *parent, char const *name, TemplateTableRow &parentTemplateTableRow);

	//void    init(std::vector<DynamicVariableParamData *> const *dynamicVariableParamData);
	void    init(DynamicVariableParamData const *dynamicVariableParamData);
	QString getText() const;

public slots:

	virtual void addNewRowAboveCurrent();
	virtual void addNewRowBelowCurrent();
	virtual void deleteCurrentRow();

	void createObjVarTemplateTableRows(int const index, DynamicVariableParamData const &dynamicVariableParamData);

private:

	// Disabled

	DialogObjvarList();
	DialogObjvarList(DialogObjvarList const &);
	DialogObjvarList &operator=(DialogObjvarList const &);
};

// ============================================================================

#endif // INCLUDED_DialogObjvarList_H
