//======================================================================
//
// SwgCuiResourceExtraction_SetResource_TableModel.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiResourceExtractionSetResourceTableModel_H
#define INCLUDED_SwgCuiResourceExtractionSetResourceTableModel_H

//======================================================================

#include "swgClientUserInterface/SwgCuiResourceExtraction_SetResource.h"

#include "UITableModel.h"

class SwgCuiInventoryContainer;
class SwgCuiSurvey;
class UITable;

//----------------------------------------------------------------------

class SwgCuiResourceExtraction_SetResource::TableModel : public UITableModel
{
public:

	enum Type
	{
		T_SetResourceSelf
	};

	TableModel (SwgCuiResourceExtraction_SetResource & containerMediator);
	~TableModel ();

	bool                   GetValueAtText   (int row, int col, UIString     & value)  const;
	bool                   GetValueAtBool   (int row, int col, bool & value)          const;
	bool                   GetValueAtInteger(int row, int col, int & value)           const;
	bool                   GetValueAtWidget (int row, int col, UIWidget *& value)     const;
	bool                   GetSortKeyAtInteger(int row, int col, UITableTypes::CellType type, int & result)  const;

	UIBaseObject          *Clone() const { return 0; }

	bool                   GetLocalizedColumnName    (int col, UIString & str) const;
	bool                   GetLocalizedColumnTooltip (int col, UIString & str) const;

	UITableTypes::CellType FindColumnCellType     (int col) const;
	int                    FindColumnCount        () const;
	int                    FindRowCount           () const;
	UIWidget *             GetDragWidgetAt        (int row, int col);

	void                           setType (Type type);
	Type                           getType () const;

	void                   updateTableColumnSizes (UITable & table) const;


private:
	TableModel (const TableModel &);
	TableModel & operator= (const TableModel &);


	Type                       m_type;
	SwgCuiResourceExtraction_SetResource &             m_ownerMediator;
};

//----------------------------------------------------------------------

inline SwgCuiResourceExtraction_SetResource::TableModel::Type SwgCuiResourceExtraction_SetResource::TableModel::getType () const
{
	return m_type;
}

#endif

//======================================================================