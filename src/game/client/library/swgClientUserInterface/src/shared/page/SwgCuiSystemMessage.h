//======================================================================
//
// SwgCuiSystemMessage.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiSystemMessage_H
#define INCLUDED_SwgCuiSystemMessage_H

//======================================================================

#include "UINotification.h"
#include "clientUserInterface/CuiMediator.h"

class CuiInstantMessageManagerElement;
class CuiSystemMessageManagerData;
class UIPage;
class UIText;
class UIWidget;
class UIComposite;

namespace MessageDispatch
{
	class Callback;
}

// ======================================================================

/**
* SwgCuiSystemMessage
*/

class SwgCuiSystemMessage :
public CuiMediator
{
public:

	enum Type
	{
		T_normal,
		T_noob
	};

	explicit                      SwgCuiSystemMessage         (UIPage & page, Type type = T_normal);

	void                          performActivate      ();
	void                          performDeactivate    ();

	void                          checkStatus          ();

	void                          appendText           (const Unicode::String & str);

	void                          onInstantMessageReceived (const CuiInstantMessageManagerElement & elem);
	void                          onSystemMessageReceived  (const CuiSystemMessageManagerData & msg);

	void                          update                   (float deltaTimeSecs);

private:
	                             ~SwgCuiSystemMessage ();
	                              SwgCuiSystemMessage ();
	                              SwgCuiSystemMessage (const SwgCuiSystemMessage & rhs);
	SwgCuiSystemMessage &         operator=      (const SwgCuiSystemMessage & rhs);

private:

	UIComposite *                 m_composite;
	UIText *                      m_sampleText;
	float                         m_timeout;

	MessageDispatch::Callback *   m_callback;

	bool                          m_moveToTop;

	bool						  m_updatePack;

	Type                          m_type;
};

//======================================================================

#endif
