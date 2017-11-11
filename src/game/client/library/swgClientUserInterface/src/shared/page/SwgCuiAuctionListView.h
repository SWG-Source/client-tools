//======================================================================
//
// SwgCuiAuctionListView.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAuctionListView_H
#define INCLUDED_SwgCuiAuctionListView_H

#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"

//======================================================================

class UITable;
class SwgCuiAuctionFilter;
class AuctionManagerClientData;
class SwgCuiAuctionPaneTableModel;
class SwgCuiAuctionListPane;

//----------------------------------------------------------------------

#include "UIEventCallback.h"
#include "UITypes.h"

namespace MessageDispatch
{
	class Callback;
	template <typename MessageType, typename IdentifierType> class Transceiver;
};

//----------------------------------------------------------------------

class SwgCuiAuctionListView : 
public UIEventCallback
{
public:

	struct Messages
	{
		struct DoubleClick
		{
			typedef SwgCuiAuctionListView Payload;
		};

		struct SelectionChanged
		{
			typedef SwgCuiAuctionListView Payload;
		};

		struct ContextRequest
		{
			typedef std::pair <const SwgCuiAuctionListView *, std::pair<NetworkId, UIPoint> > Payload;
		};
	};

	struct Transceivers
	{
		typedef MessageDispatch::Transceiver <const Messages::DoubleClick::Payload &,      Messages::DoubleClick>      DoubleClick;
		typedef MessageDispatch::Transceiver <const Messages::SelectionChanged::Payload &, Messages::SelectionChanged> SelectionChanged;
		typedef MessageDispatch::Transceiver <const Messages::ContextRequest::Payload &,   Messages::ContextRequest>   ContextRequest;
	};

	typedef SwgCuiAuctionPaneTableModel     TableModel;
	typedef AuctionManagerClientData        Data;
	typedef stdvector<Data>::fwd            DataVector;
	typedef stdvector<int>::fwd             ObjectTypeVector;

	SwgCuiAuctionListView (SwgCuiAuctionListPane & pane, UITable & table, SwgCuiAuctionFilter * filter, int type);
	~SwgCuiAuctionListView ();
	
	bool                             OnMessage                 (UIWidget * context, const UIMessage & msg);
	void                             OnGenericSelectionChanged (UIWidget * context);

	void                             saveSettings              (int const paneType) const;
	void                             loadSettings              (int const paneType);

	void                             setData                   (const DataVector & dv);
	void                             requestUpdate             (int startingIndex);
	int                              getType                   () const;

	void                             setActive                 (bool b);

	void                             refreshData               ();
	void                             getDataVector             (DataVector & dv);
	void                             updateData                (const DataVector & dv);

	Transceivers::DoubleClick &      getTransceiverDoubleClick      ();
	Transceivers::SelectionChanged & getTransceiverSelectionChanged ();
	Transceivers::ContextRequest &   getTransceiverContextRequest   ();


	typedef stdvector<NetworkId>::fwd NetworkIdVector;
	const NetworkId &       findSelectedAuction       (bool warn) const;
	const NetworkIdVector & findSelectedAuctions      (bool warn) const;

	void                             traverseAuctionToView     (const bool & forward);
	void                             onListChanged             (const uint8 & sequence);
	void                             onFilterChanged           (const SwgCuiAuctionFilter & filter);

	typedef std::pair<int, Unicode::String> ResultInfo;

	void                             onCreateAuction           (const std::pair<NetworkId, ResultInfo> & result);
	void                             onGenericResponse         (const std::pair<NetworkId, ResultInfo> & result);
	void							 onItemRetrieve			   (const std::pair<NetworkId, ResultInfo> & result);
	void                             update                    (float deltaTimeSecs);
	void                             clearAllSelection         ();

private:

	SwgCuiAuctionListView ();
	SwgCuiAuctionListView (const SwgCuiAuctionListView & rhs);
	SwgCuiAuctionListView & operator= (const SwgCuiAuctionListView & rhs);

	UITable *                 m_table;
	SwgCuiAuctionFilter *     m_filter;
	int                       m_type;
	TableModel *              m_model;

	Transceivers::DoubleClick *      m_transceiverDoubleClick;
	Transceivers::SelectionChanged * m_transceiverSelectionChanged;
	Transceivers::ContextRequest *   m_transceiverContextRequest;

	bool                             m_active;

	MessageDispatch::Callback *      m_callback;

	float                            m_elapsedTime;

	SwgCuiAuctionListPane *          m_pane;
};

//----------------------------------------------------------------------

inline SwgCuiAuctionListView::Transceivers::DoubleClick & SwgCuiAuctionListView::getTransceiverDoubleClick ()
{
	return *NON_NULL (m_transceiverDoubleClick);
}

//----------------------------------------------------------------------

inline SwgCuiAuctionListView::Transceivers::SelectionChanged & SwgCuiAuctionListView::getTransceiverSelectionChanged ()
{
	return *NON_NULL (m_transceiverSelectionChanged);
}

//----------------------------------------------------------------------

inline SwgCuiAuctionListView::Transceivers::ContextRequest & SwgCuiAuctionListView::getTransceiverContextRequest ()
{
	return *NON_NULL (m_transceiverContextRequest);
}

//----------------------------------------------------------------------

inline int SwgCuiAuctionListView::getType       () const
{
	return m_type;
}
 
//======================================================================

#endif
