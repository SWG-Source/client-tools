// ============================================================================
//
//	TableItemEnum.h
//
//	This TableItem is used for selecting a single enumerated value.
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemEnum_H
#define INCLUDED_TableItemEnum_H

#include "TableItemBase.h"

class EnumEdit;
class IntegerParam;
class TemplateTableRow;

//-----------------------------------------------------------------------------
class TableItemEnum : public TableItemBase
{

public:

	TableItemEnum(TemplateTableRow &parentRow);

	virtual void    init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual void    init(IntegerParam const *integerParam, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual QString getTypeString() const;
	virtual void    setContentFromEditor(QWidget *widget);
	virtual int     getSupportedParameterProperties() const;

private:

	void initAsSingle(IntegerParam const *integerParam);
	void initAsWeightedList(IntegerParam const *integerParam, int const parameterIndex);

	EnumEdit *getEnumEdit() const;

private:

	// Disabled

	TableItemEnum();
	TableItemEnum(TableItemEnum const &);
	TableItemEnum &operator=(TableItemEnum const &);
};

// ============================================================================

#endif // INCLUDED_TableItemEnum_H
