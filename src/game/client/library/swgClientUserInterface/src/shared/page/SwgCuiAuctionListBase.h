//======================================================================
//
// SwgCuiAuctionListBase.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiAuctionListBase_H
#define INCLUDED_SwgCuiAuctionListBase_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "../../../../../../engine/shared/library/sharedFoundation/include/public/sharedFoundation/NetworkId.h"
#include "UIEventCallback.h"

class UITabbedPane;
class AuctionManagerClientData;
class StringId;
class SwgCuiAuctionListPane;
class UIText;
class CuiMoney;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiAuctionListBase :
public CuiMediator,
public UIEventCallback
{
public:

	struct TabInfo
	{
		const char *       name;
		int                type;
		const char *       page;
		const StringId *   label;
		bool disableBuyInDetails;

		TabInfo (const char * const _name, const int _type, const char * const _page, const StringId & _label, bool const _disableBuyInDetails = false) :
		name  (_name),
		type  (_type),
		page  (_page),
		label (&_label),
		disableBuyInDetails(_disableBuyInDetails)
		{
		}
		TabInfo (const TabInfo & rhs) :
		name  (rhs.name),
		type  (rhs.type),
		page  (rhs.page),
		label (rhs.label),
		disableBuyInDetails (rhs.disableBuyInDetails)
		{
		}

		TabInfo & operator= (const TabInfo & rhs)
		{
		name  = rhs.name;
		type  = rhs.type;
		page  = rhs.page;
		label = rhs.label;
		disableBuyInDetails = rhs.disableBuyInDetails;
		return *this;
		}

		TabInfo () :
		name  (0),
		type  (0),
		page  (0),
		label (0),
		disableBuyInDetails(0)
		{
		}

	private:
	};

	//----------------------------------------------------------------------

	typedef AuctionManagerClientData   Data;
	typedef stdvector<int>::fwd        IntVector;
	typedef stdvector<TabInfo>::fwd    TabInfoVector;

	typedef stdmap<int, SwgCuiAuctionListPane *>::fwd PaneMap;

	class TableModel;

	SwgCuiAuctionListBase              (const char * const name, UIPage & page);

	void                   OnTabbedPaneChanged       (UIWidget * context);
	void                   OnButtonPressed           (UIWidget * context);

	void                   update                    (float deltaTimeSecs);

	bool                   close                     ();

	typedef std::pair<int, Unicode::String> ResultInfo;

	void                   onRetrieveRequested       (const NetworkId & id);
	void                   onItemRetrieved           (const std::pair<NetworkId, ResultInfo> & payload);
	

	void                   onWithdrawRequested       (const NetworkId & id);
	void                   onWithdrawn               (const std::pair<NetworkId, ResultInfo> & payload);


protected:

	~SwgCuiAuctionListBase ();

	virtual void                         performActivate   ();
	virtual void                         performDeactivate ();

	void                                 setTabs (const TabInfo * tabInfos, int numTabs);

private:

	SwgCuiAuctionListBase ();
	SwgCuiAuctionListBase (const SwgCuiAuctionListBase & rhs);
	SwgCuiAuctionListBase operator= (const SwgCuiAuctionListBase & rhs);

	void                   setupCurrentTab           ();
	void                   setupTabAll               ();
	void                   requestUpdate             (bool optional);
	void                   showDetails               () const;
	void                   setActiveView             (int type, bool forceDeactivate = false);
	void                   updateStatusText          ();

	void                   setupTabs  ();
	void                   clearPanes ();

	MessageDispatch::Callback * m_callback;

	UITabbedPane *              m_tabs;

	UIButton *                  m_buttonExit;
	UIButton *                  m_buttonDetails;
	UIButton *                  m_buttonRefresh;
	UIText *                    m_caption;

	PaneMap *                   m_paneMap;
	int                         m_type;

	IntVector *                 m_tabTypes;

	NetworkId                   m_confirmingWithdraw;
	NetworkId                   m_confirmingAccept;

	TabInfoVector *             m_tabInfoVector;
	CuiMoney *                  m_money;

	float                       m_elapsedTime;

	enum { NumTextStatus = 2};

	UIText *                    m_textStatus [NumTextStatus];
};

//======================================================================

#endif
