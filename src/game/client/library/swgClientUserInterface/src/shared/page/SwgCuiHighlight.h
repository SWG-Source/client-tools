//======================================================================
//
// SwgCuiHighlight.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiHighlight_H
#define INCLUDED_SwgCuiHighlight_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"

#include "sharedMessageDispatch/Receiver.h"

#include <map>

class UIText;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------

class SwgCuiHighlight :
public CuiMediator,
public MessageDispatch::Receiver
{
public:

	explicit                  SwgCuiHighlight              (UIPage & page, int sceneType);
	void                      update                   (float deltaTimeSecs);

	struct HighlightRecord
	{
		UIWidget *target;
		UIPage *highlightPage;
		float maxTime;
		float currentTime;
	};

	void                     addHighlight(UIWidget *target, float time);
	void                     removeHighlights(UIWidget *target);
	void                     removeAllHighlights();

	virtual void             receiveMessage               (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

protected:

	virtual void             performActivate   ();
	virtual void             performDeactivate ();

private:

	                          ~SwgCuiHighlight ();
	SwgCuiHighlight ();
	SwgCuiHighlight (const SwgCuiHighlight & rhs);
	SwgCuiHighlight & operator= (const SwgCuiHighlight & rhs);

	UIPage *m_sampleHighlightPage;
   std::map<UIWidget *, HighlightRecord> m_highlightPages;

	int m_sceneType;
};

//======================================================================

#endif
