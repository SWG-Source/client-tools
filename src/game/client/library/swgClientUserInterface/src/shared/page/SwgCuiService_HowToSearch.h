// ======================================================================
//
// SwgCuiService_HowToSearch.h
// Copyright Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SwgCuiService_HowToSearch_H
#define INCLUDED_SwgCuiService_HowToSearch_H

#include "clientUserInterface/CuiMediator.h"
#include "swgClientUserInterface/SwgCuiService.h"
#include "UIEventCallback.h"

//-----------------------------------------------------------------

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------
class SwgCuiService::HowToSearch : public CuiMediator
                                   , public UIEventCallback
{
public:

	HowToSearch(UIPage &page, SwgCuiService &communityMediator);

	virtual void performActivate();
	virtual void performDeactivate();
	virtual void update(float deltaTimeSecs);

private:
	

	MessageDispatch::Callback  *m_callBack;
	SwgCuiService              &m_serviceMediator;

	// Disabled

	~HowToSearch();
	HowToSearch(HowToSearch const &rhs);
	HowToSearch &operator =(HowToSearch const &rhs);
};

// ======================================================================

#endif // INCLUDED_SwgCuiService_HowToSearch_H
