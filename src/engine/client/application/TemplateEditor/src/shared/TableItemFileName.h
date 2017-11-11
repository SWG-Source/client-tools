// ============================================================================
//
//	TableItemFileName.h
//
//	This TableItem is used for entering a simple float value.
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemFilename_H
#define INCLUDED_TableItemFilename_H

#include "TableItemBase.h"

class FilenameDialogEdit;
class TemplateTableRow;
class StringParam;

//-----------------------------------------------------------------------------
class TableItemFileName : public TableItemBase
{
public:

	TableItemFileName(TemplateTableRow &parentRow);

	virtual void    init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual QString getTypeString() const;
	void            setContentFromEditor(QWidget *widget);
	virtual int     getSupportedParameterProperties() const;

private:

	void                init(StringParam const *stringParam, int const parameterIndex, int const weightedListIndex);
	void                initAsSingle(StringParam const *stringParam);
	void                initAsWeightedList(StringParam const *stringParam, int const parameterIndex);

	FilenameDialogEdit *getFilenameDialogEdit() const;

private:

	// Disabled

	TableItemFileName();
	TableItemFileName(TableItemFileName const &);
	TableItemFileName &operator=(TableItemFileName const &);
};

// ============================================================================

#endif // INCLUDED_TableItemFilename_H
