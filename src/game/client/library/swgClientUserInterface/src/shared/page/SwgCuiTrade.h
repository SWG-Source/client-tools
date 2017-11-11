//======================================================================
//
// SwgCuiTrade.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiTrade_H
#define INCLUDED_SwgCuiTrade_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIPage;
class UIButton;
class UIText;
class NetworkId;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiTrade :
public CuiMediator,
public UIEventCallback
{
public:

	class Panel;

	explicit                SwgCuiTrade                (UIPage & page);

	void                    performActivate            ();
	void                    performDeactivate          ();

	void                    OnButtonPressed            (UIWidget * context);
	bool                    close                      ();

	static SwgCuiTrade *    createInto                 (UIPage & parent);

	void                    update                     (float deltaTimeSecs);

private:
	                       ~SwgCuiTrade                ();
	                        SwgCuiTrade                (const SwgCuiTrade &);
	SwgCuiTrade &           operator=                  (const SwgCuiTrade &);
	
	void                    reset                      ();
	
	void                    onBeginTrade               (const bool &      payload);
	void                    onGiveMoney                (const int &       payload);
	void                    onAccept                   (const bool &      payload);
	void                    onRequestVerify            (const bool &      payload);
	void                    onComplete                 (const bool &      payload);
	void                    onAbort                    (const bool &      payload);
	
	Panel *                 m_panelSelf;
	Panel *                 m_panelOther;
	
	UIButton *              m_buttonCancel;
	UIButton *              m_buttonOk;

	MessageDispatch::Callback *  m_callback;

	float                   m_timeSinceLastRangeCheck;

	UIText *                m_textWaiting;

};

//======================================================================

#endif
