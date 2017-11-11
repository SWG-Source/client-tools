//======================================================================
//
// SwgCuiCraftOption.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SwgCuiCraftOption_H
#define INCLUDED_SwgCuiCraftOption_H

//======================================================================

#include "clientUserInterface/CuiMediator.h"
#include "UIEventCallback.h"

class UIPage;
class UIButton;

namespace MessageDispatch
{
	class Callback;
};

//----------------------------------------------------------------------

class SwgCuiCraftOption :
public CuiMediator,
public UIEventCallback
{
public:

	explicit                    SwgCuiCraftOption (UIPage & page);

	virtual void                performActivate   ();
	virtual void                performDeactivate ();

	virtual void                OnButtonPressed   (UIWidget *context );

	virtual void                update            (float deltaTimeSecs);
	bool                        close             ();

private:
	                           ~SwgCuiCraftOption ();
	                            SwgCuiCraftOption (const SwgCuiCraftOption &);
	SwgCuiCraftOption &         operator= (const SwgCuiCraftOption &);

	void                        onCustomize               (const int & );
	void                        onSessionEnded            (const bool &);

	UIButton *                  m_buttonSchematic;
	UIButton *                  m_buttonPrototype;
	UIButton *                  m_buttonExperiment;
	bool                        m_transition;

	MessageDispatch::Callback *   m_callback;

	bool                        m_sessionEnded;
};

//======================================================================

#endif
