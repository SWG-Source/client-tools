// ============================================================================
//
//	TableItemString.h
//
//	This TableItem is used for entering a single string value.
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemString_H
#define INCLUDED_TableItemString_H

#include "TableItemBase.h"

class TemplateTableRow;
class FloatRangeDialogEdit;
class StringEdit;
class StringParam;

//-----------------------------------------------------------------------------
class TableItemString : public TableItemBase
{

public:

	TableItemString(TemplateTableRow &parentRow);

	virtual void    initDefault();
	virtual void    init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual void    init(StringParam const *stringParam, int const parameterIndex, int const weightedListIndex);
	virtual QString getTypeString() const;
	virtual void    setContentFromEditor(QWidget *widget);
	virtual int     getSupportedParameterProperties() const;

private:

	void        initAsSingle(StringParam const *stringParam);
	void        initAsWeightedList(StringParam const *stringParam, int const parameterIndex);

	StringEdit *getStringEdit() const;

private:

	// Disabled

	TableItemString();
	TableItemString(TableItemString const &);
	TableItemString &operator=(TableItemString const &);
};

// ============================================================================

#endif // INCLUDED_TableItemString_H
