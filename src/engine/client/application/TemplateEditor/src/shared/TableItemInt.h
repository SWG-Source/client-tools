// ============================================================================
//
//	TableItemInt.h
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemInt_H
#define INCLUDED_TableItemInt_H

#include "TableItemBase.h"
#include "TemplateTableRow.h"

class DiceRollDialogEdit;
class IntegerEdit;
class IntegerRangeDialogEdit;
class IntegerParam;
class TemplateTableRow;

//-----------------------------------------------------------------------------
class TableItemInt : public TableItemBase
{
public:

	TableItemInt(TemplateTableRow &parentRow);

	virtual void    initDefault();
	virtual void    init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual void    init(IntegerParam const *integerParam, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual QString getTypeString() const;
	virtual int     getSupportedParameterProperties() const;
	virtual void    setContentFromEditor(QWidget *widget);
	
private:

	void                    initAsDieRoll(IntegerParam const *integerParam);
	void                    initAsRandomRange(IntegerParam const *integerParam);
	void                    initAsSingle(IntegerParam const *integerParam);
	void                    initAsWeightedList(IntegerParam const *integerParam, int const parameterIndex);

	DiceRollDialogEdit *    getDiceRollDialogEdit() const;
	IntegerEdit *           getIntegerEdit() const;
	IntegerRangeDialogEdit *getIntegerRangeDialogEdit() const;

private:

	// Disabled

	TableItemInt();
	TableItemInt(TableItemInt const &);
	TableItemInt &operator=(TableItemInt const &);
};

// ============================================================================

#endif // INCLUDED_TableItemInt_H
