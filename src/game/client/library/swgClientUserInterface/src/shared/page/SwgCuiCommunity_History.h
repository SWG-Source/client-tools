// ======================================================================
//
// SwgCuiCommunity_History.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiCommunity_History_H
#define INCLUDED_SwgCuiCommunity_History_H

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"
#include "swgClientUserInterface/SwgCuiCommunity.h"

class MatchMakingResult;
class UITable;

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiCommunity::History : public CuiMediator
                               , public UIEventCallback
{
public:

	History(UIPage &page, SwgCuiCommunity const &communityMediator);

	virtual void performActivate();
	virtual void performDeactivate();

	void onMatchFound(MatchMakingResult const &matchResult);

private:
	
	enum Column
	{
		C_name = 0,
		C_matchToThem = 1,
		C_matchToYou = 2,
		C_time = 3,
		C_count = 4
	};

	SwgCuiCommunity const &    m_communityMediator;
	MessageDispatch::Callback *m_callBack;
	UITable *                  m_historyTable;

	// Disabled

	~History();
	History(History const &rhs);
	History &operator =(History const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiCommunity_History_H
