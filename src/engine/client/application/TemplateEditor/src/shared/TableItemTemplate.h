// ============================================================================
//
//	TableItemTemplate.h
//
//	This TableItem is used for dealing with the special '@base' template value,
//	which takes in a .obt (Object Template) and then fills out the Template
//	Table with the newly loaded values.
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemTemplate_H
#define INCLUDED_TableItemTemplate_H

#include "TableItemBase.h"

class FilenameDialogEdit;
class StringParam;
class TemplateTableRow;

//-----------------------------------------------------------------------------
class TableItemTemplate : public TableItemBase
{

public:

	TableItemTemplate(TemplateTableRow &parentRow);

	virtual void    init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual QString getTypeString() const;
	virtual void    setContentFromEditor(QWidget *widget);
	virtual int     getSupportedParameterProperties() const;

private:

	void                init(StringParam const *stringParam, int const parameterIndex, int const weightedListIndex);
	void                initAsSingle(StringParam const *stringParam);
	void                initAsWeightedList(StringParam const *stringParam, int const parameterIndex);

	FilenameDialogEdit *getFileNameDialogEdit();

private:

	// Disabled

	TableItemTemplate();
	TableItemTemplate(TableItemTemplate const &);
	TableItemTemplate &operator=(TableItemTemplate const &);
};

// ============================================================================

#endif // INCLUDED_TableItemTemplate_H
