//======================================================================
//
// SwgCuiAuctionPaneSwgCuiAuctionPaneTableModel.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAuctionPaneSwgCuiAuctionPaneTableModel_H
#define INCLUDED_SwgCuiAuctionPaneSwgCuiAuctionPaneTableModel_H

#include "UITableModel.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include "swgClientUserInterface/SwgCuiAuctionPaneTypes.h"

#include <map>

class AuctionManagerClientData;
class UITable;

//======================================================================

class SwgCuiAuctionPaneTableModel :
public UITableModel
{
public:

	typedef SwgCuiAuctionPaneTypes::Type Type;

	struct DisplayCache
	{
		Unicode::String typeName;
		Unicode::String ownerName;
		Unicode::String itemName;
		Unicode::String highBidder;
		Unicode::String timeRemaining;
		Unicode::String location;
		int             last_timer;
		NetworkId       id;
		Vector          position_w;
		int             entranceFee;
		bool            isOnSamePlanet;
	};

	typedef stdvector<DisplayCache>::fwd DisplayCacheVector;

	typedef AuctionManagerClientData   Data;
	typedef stdvector<Data>::fwd       DataVector;

	explicit                       SwgCuiAuctionPaneTableModel (Type type);

	bool                           GetValueAtText          (int row, int col, UIString & value)  const;
	bool                           GetValueAtInteger       (int row, int col, int & value)       const;
	virtual bool                   GetSortKeyAtString      (int row, int col, UITableTypes::CellType type, UIString & strValue)  const;
	virtual bool                   GetValueAtImage         (int row, int col, UIImageStyle *& value)  const;

	UIBaseObject *                 Clone                   () const { return 0; }

	bool                           GetLocalizedColumnName  (int col, UIString & str) const;
	UITableTypes::CellType         FindColumnCellType      (int col) const;
	int                            FindColumnCount         () const;
	int                            FindRowCount            () const;

//	void                           setType                 (Type type);
	void                           setData                 (const DataVector & dv);
	void                           updateData              (const DataVector & dv);

	Type                           getType                 () const;

	const NetworkId &              getAuctionDataItemId    (int row) const;

	const Data *                   findDataElement         (int visualRow, const DisplayCache *& cache) const;
	int                            findVisualRowForAuction (const NetworkId & auctionId) const;

	void                           clearAll                ();
	void                           updateTableColumnSizes  (UITable & table);
	void                           persistTableColumnSizes (UITable const & table) const;

	bool GetTooltipAt(int row, int col, UIString & tooltip) const;

private:
	SwgCuiAuctionPaneTableModel  ();
	~SwgCuiAuctionPaneTableModel ();
	SwgCuiAuctionPaneTableModel (const SwgCuiAuctionPaneTableModel &);
	SwgCuiAuctionPaneTableModel & operator= (const SwgCuiAuctionPaneTableModel &);

	Data *                         findDataElement(const NetworkId & auctionId) const;
	DisplayCache  *                 findDisplayCache(const NetworkId & auctionId) const;

	void                                        refreshDisplayCache ();

	Type                                        m_type;

	DataVector *                                m_data;
	DisplayCacheVector *                        m_displayCache;

	mutable std::map<NetworkId, UIString> m_toolTips;
};


//----------------------------------------------------------------------

inline SwgCuiAuctionPaneTableModel::Type SwgCuiAuctionPaneTableModel::getType                 () const
{
	return m_type;
}

//======================================================================

#endif
