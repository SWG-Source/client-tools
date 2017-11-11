//======================================================================
//
// SwgCuiResourceExtractionHopperTableModel.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiResourceExtractionHopperTableModel_H
#define INCLUDED_SwgCuiResourceExtractionHopperTableModel_H

//======================================================================

#include "swgClientUserInterface/SwgCuiResourceExtraction_Hopper.h"

#include "UITableModel.h"

class SwgCuiInventoryContainer;
class SwgCuiSurvey;
class UITable;

//----------------------------------------------------------------------

class SwgCuiResourceExtraction_Hopper::TableModel : public UITableModel
{
public:

	enum Type
	{
		T_hopperSelf
	};

	TableModel (SwgCuiResourceExtraction_Hopper & containerMediator);
	~TableModel ();

	bool                   GetValueAtText   (int row, int col, UIString     & value)  const;
	bool                   GetValueAtBool   (int row, int col, bool & value)          const;
	bool                   GetValueAtInteger(int row, int col, int & value)           const;
	bool                   GetValueAtWidget (int row, int col, UIWidget *& value)     const;
	bool                   GetSortKeyAtInteger(int visualRow, int col, UITableTypes::CellType type, int & result) const;


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
	SwgCuiResourceExtraction_Hopper &             m_ownerMediator;
};

//----------------------------------------------------------------------

inline SwgCuiResourceExtraction_Hopper::TableModel::Type SwgCuiResourceExtraction_Hopper::TableModel::getType () const
{
	return m_type;
}

#endif

//======================================================================

