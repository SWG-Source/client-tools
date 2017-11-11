//======================================================================
//
// SwgCuiMissionBrowser_TableModel.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiMissionBrowser_TableModel_H
#define INCLUDED_SwgCuiMissionBrowser_TableModel_H

//======================================================================

#include "swgClientUserInterface/SwgCuiMissionBrowser.h"
#include "UITableModel.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/Watcher.h"

class MessageQueueMissionListResponseData;
class NetworkId;

//----------------------------------------------------------------------

class SwgCuiMissionBrowser::TableModel :
public UITableModel
{
public:

	TableModel ();

	enum Type
	{
		T_Destroy,
		T_Deliver,
		T_Bounty,
		T_Dancer,
		T_Musician,
		T_Crafting,
		T_Survey,
		T_Hunting,
		T_Assassin,
		T_Recon,
		T_Count
	};

	typedef ClientMissionObject DataElement;
	typedef stdvector<ConstWatcher <DataElement> >::fwd DataVector;

	virtual bool                   GetValueAtText   (int row, int col, UIString     & value)  const;
	virtual bool                   GetValueAtBool   (int row, int col, bool & value)          const;
	virtual bool                   GetValueAtInteger(int row, int col, int & value)           const;

	virtual UIBaseObject          *Clone() const { return 0; }

	virtual bool                   GetLocalizedColumnName (int col, UIString & str) const;
	virtual UITableTypes::CellType FindColumnCellType (int col) const;
	virtual int                    FindColumnCount () const;
	virtual int                    FindRowCount () const;

	void                           setType (Type type);
	void                           setData (Type type, const DataVector & dv);

	Type                           getType () const;

	void                           updateTableColumnSizes (UITable & table) const;

	void                           getMissionDataId (int row, NetworkId & id) const;

	const DataElement *            findDataElement (int visualRow) const;

	void                           clearAll ();

private:
	~TableModel ();
	TableModel (const TableModel &);
	TableModel & operator= (const TableModel &);

	Type                                        m_type;

	DataVector *                                m_data;
};

//----------------------------------------------------------------------

inline SwgCuiMissionBrowser::TableModel::Type SwgCuiMissionBrowser::TableModel::getType () const
{
	return m_type;
}

//======================================================================

#endif
