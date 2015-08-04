// ============================================================================
//
//	TableItemFloat.h
//
//	This TableItem is used for entering a simple float value.
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemFloat_H
#define INCLUDED_TableItemFloat_H

#include "TableItemBase.h"

class FloatEdit;
class FloatRangeDialogEdit;
class FloatParam;
class TemplateTableRow;

//-----------------------------------------------------------------------------
class TableItemFloat : public TableItemBase
{
public:

	TableItemFloat(TemplateTableRow &parentRow);

	virtual void    initDefault();
	virtual void    init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual void    init(FloatParam const *floatParam, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual QString getTypeString() const;
	virtual void    setContentFromEditor(QWidget *widget);
	virtual int     getSupportedParameterProperties() const;

private:

	FloatEdit *           getFloatEdit() const;
	FloatRangeDialogEdit *getFloatRangeDialogEdit() const;
	void                  initAsRandomRange(FloatParam const *floatParam);
	void                  initAsSingle(FloatParam const *floatParam);
	void                  initAsWeightedList(FloatParam const *floatParam, int const parameterIndex);

private:

	// Disabled

	TableItemFloat();
	TableItemFloat(TableItemFloat const &);
	TableItemFloat &operator=(TableItemFloat const &);
};

// ============================================================================

#endif // INCLUDED_TableItemFloat_H
