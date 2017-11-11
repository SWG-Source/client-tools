// ============================================================================
//
//	TableItemObjvar.h
//
//	This TableItem is used for editing an objvar
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemObjvar_H
#define INCLUDED_TableItemObjvar_H

#include "TableItemBase.h"

class DynamicVariableParamData;
class TableItemEditorObjVar;
class TemplateTableRow;

//-----------------------------------------------------------------------------
class TableItemObjvar : public TableItemBase
{

public:

	TableItemObjvar(TemplateTableRow &parentRow);

	virtual void    initDefault();
	virtual void    init(DynamicVariableParamData const *dynamicVariableParamData);
	virtual void    init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual QString getTypeString() const;
	virtual void    setContentFromEditor(QWidget *widget);
	virtual int     getSupportedParameterProperties() const;

private:

	TableItemEditorObjVar *getTableItemEditorObjVar() const;
	void                   initAsSingle(DynamicVariableParamData const *dynamicVariableParamData);

	DynamicVariableParamData const *m_value;

private:

	// Disabled

	TableItemObjvar();
	TableItemObjvar(TableItemObjvar const &);
	TableItemObjvar &operator=(TableItemObjvar const &);
};

// ============================================================================

#endif // INCLUDED_EditFilenameTableItem_H
