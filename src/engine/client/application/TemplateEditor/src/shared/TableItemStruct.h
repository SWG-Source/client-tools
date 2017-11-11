// ============================================================================
//
//	TableItemStruct.h
//
//	This TableItem is used for editing a template structure
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemStruct_H
#define INCLUDED_TableItemStruct_H

#include "TableItemBase.h"

class StructDialogEdit;
class TemplateTableRow;

template <class ST> class StructParam;
class ObjectTemplate;
typedef StructParam<ObjectTemplate> StructParamOT;

//-----------------------------------------------------------------------------
class TableItemStruct : public TableItemBase
{
public:

	TableItemStruct(TemplateTableRow &parentRow);

	virtual void    init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual QString getTypeString() const;
	virtual void    setContentFromEditor(QWidget *widget);
	virtual int     getSupportedParameterProperties() const;

private:

	StructDialogEdit *getStructDialogEdit() const;
	void              initAsSingle(StructParamOT const *structParamOT);

private:

	// Disabled

	TableItemStruct();
	TableItemStruct(TableItemStruct const &);
	TableItemStruct &operator=(TableItemStruct const &);
};

// ============================================================================

#endif // INCLUDED_EditFilenameTableItem_H
