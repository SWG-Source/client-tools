// ============================================================================
//
//	TableItemStringId.h
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemStringId_H
#define INCLUDED_TableItemStringId_H

#include "TableItemBase.h"
#include "StringId.h"

class TemplateTableRow;
class StringIDDialogEdit;
class StringIdParam;

//-----------------------------------------------------------------------------
class TableItemStringId : public TableItemBase
{
public:

	TableItemStringId(TemplateTableRow &parentRow);

	virtual QString getTypeString() const;
	virtual void    init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual void    setContentFromEditor(QWidget *widget);
	virtual int     getSupportedParameterProperties() const;

private:

	void init(StringIdParam const *stringIdParam, int const parameterIndex, int const weightedListIndex);
	void initAsSingle(StringIdParam const *stringIdParam);
	void initAsWeightedList(StringIdParam const *stringIdParam, int const parameterIndex);

	StringIDDialogEdit *getStringIDDialogEdit() const;

private:

	// Disabled

	TableItemStringId();
	TableItemStringId(TableItemStringId const &);
	TableItemStringId &operator=(TableItemStringId const &);
};

// ============================================================================

#endif // INCLUDED_EditFilenameTableItem_H
