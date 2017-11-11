//======================================================================
//
// SwgCuiInventoryContainerDetailsTableModel.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiInventoryContainerDetailsTableModel_H
#define INCLUDED_SwgCuiInventoryContainerDetailsTableModel_H

//======================================================================

#include "UITableModel.h"

class SwgCuiInventoryContainer;
class UITable;

//----------------------------------------------------------------------

class SwgCuiInventoryContainerDetailsTableModel :
public UITableModel
{
public:

	SwgCuiInventoryContainerDetailsTableModel ();
	~SwgCuiInventoryContainerDetailsTableModel ();

	void                   setContainerMediator       (SwgCuiInventoryContainer * containerMediator);

	bool                   GetValueAtText             (int row, int col, UIString     & value)  const;
	bool                   GetValueAtBool             (int row, int col, bool & value)          const;
	bool                   GetValueAtInteger          (int row, int col, int & value)           const;
	bool                   GetValueAtWidget           (int row, int col, UIWidget *& value)     const;

	UIBaseObject          *Clone() const { return 0; }

	bool                   GetLocalizedColumnName     (int col, UIString & str) const;
	bool                   GetLocalizedColumnTooltip  (int col, UIString & str) const;

	UITableTypes::CellType FindColumnCellType         (int col) const;
	int                    FindColumnCount            () const;
	int                    FindRowCount               () const;
	UIWidget *             GetDragWidgetAt            (int row, int col);

	void                   setType                    (int type);
	int                    getType                    () const;

	void                   updateTableColumnSizes (UITable & table) const;

	int getDefaultSortColumn() const;

private:
	SwgCuiInventoryContainerDetailsTableModel (const SwgCuiInventoryContainerDetailsTableModel &);
	SwgCuiInventoryContainerDetailsTableModel & operator= (const SwgCuiInventoryContainerDetailsTableModel &);


	int                       m_type;
	SwgCuiInventoryContainer * m_containerMediator;
};

//----------------------------------------------------------------------

inline int SwgCuiInventoryContainerDetailsTableModel::getType () const
{
	return m_type;
}

//======================================================================

#endif
