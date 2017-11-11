// ============================================================================
//
//	TableItemBoolean.h
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemBoolean_H
#define INCLUDED_TableItemBoolean_H

#include "TableItemBase.h"

class BoolEdit;
class BoolParam;
class TemplateTableRow;

//-----------------------------------------------------------------------------
class TableItemBoolean : public TableItemBase
{
public:

	TableItemBoolean(TemplateTableRow &parentRow);

	virtual void    init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual QString getTypeString() const;
	virtual void    setContentFromEditor(QWidget *widget);
	virtual int     getSupportedParameterProperties() const;

private:

	void      init(BoolParam const *boolParam, int const parameterIndex, int const weightedListIndex);
	void      initAsSingle(BoolParam const *boolParam);
	void      initAsWeightedList(BoolParam const *boolParam, int const parameterIndex);

	BoolEdit *getBoolEdit() const;

private:

	// Disabled

	TableItemBoolean();
	TableItemBoolean(TableItemBoolean const &);
	TableItemBoolean &operator=(TableItemBoolean const &);
};

// ============================================================================

#endif // INCLUDED_TableItemBoolean_H
