// ============================================================================
//
//	TableItemObjvarType.h
//
//	This TableItem is used for dealing with the special 'Active Objvar value'.
//	Objvars can have multiple types, so this is the special row that lets you
//	select which type is the one you want it to be.
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TableItemObjvarType_H
#define INCLUDED_TableItemObjvarType_H

#include "TableItemBase.h"

class IntegerParam;
class ObjvarTypeEdit;
class TemplateTableRow;

//-----------------------------------------------------------------------------
class TableItemObjvarType : public TableItemBase
{

public:

	TableItemObjvarType(TemplateTableRow &parentRow);
	virtual ~TableItemObjvarType();

	virtual void    init();
	virtual QString getTypeString() const;
	virtual int     getSupportedRowTypes() const;

	//const ObjvarTypeEdit *getObjvarTypeEditor() const; // expose this for Objvar dialog

private:

	//void initAsSingle(const IntegerParam *parameterData);

private:

	// Disabled

	TableItemObjvarType();
	TableItemObjvarType(TableItemObjvarType const &);
	TableItemObjvarType &operator=(TableItemObjvarType const &);
};

// ============================================================================

#endif // INCLUDED_TableItemObjvarType_H
