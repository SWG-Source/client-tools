// ============================================================================
//
//	TableItemAtBase.h
//
//	This TableItem is used for dealing with the special '@base' template value,
//	which takes in a .obt (Object Template) and then fills out the Template
//	Table with the newly loaded values.
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemAtBase_H
#define INCLUDED_TableItemAtBase_H

#include "TableItemBase.h"

class FilenameDialogEdit;
class TemplateTableRow;
class StringParam;

//-----------------------------------------------------------------------------
class TableItemAtBase : public TableItemBase
{
public:

	TableItemAtBase(TemplateTableRow &parentRow);

	virtual void    init(TpfTemplate *tpfTemplate, QString const &parameterName, int const parameterIndex, int const weightedListIndex, TemplateTableRow::ParameterProperty const parameterProperty);
	virtual QString getTypeString() const;
	virtual int     getSupportedRowTypes() const;
	void            setPath(QString const &path);
	virtual void    setContentFromEditor(QWidget *widget);
	virtual int     getSupportedParameterProperties() const;

private:

	FilenameDialogEdit *getFilenameDialogEdit() const;

private:

	// Disabled

	TableItemAtBase();
	TableItemAtBase(TableItemAtBase const &);
	TableItemAtBase &operator=(TableItemAtBase const &);
};

// ============================================================================

#endif // INCLUDED_TableItemAtBase_H
